/*
 * This code looks for .gif files in the /gifs32x16/ directory on SPIFFS,
 * then loops each GIF in the directory for DISPLAY_TIME_SECONDS.
 * It will advance to the next GIF if the button is pressed.
 */

#define DISABLE_MATRIX_TEST
#define NEOMATRIX
#include "GifAnim_Impl.h"

// the GPIO pin the button is wired to.  the other button wire goes to GND.
#define BUTTON_PIN 35
bool previousButtonState = LOW;

// the number of seconds to display a gif before moving to the next one
#define DISPLAY_TIME_SECONDS 5

// If the matrix is a different size than the GIFs, allow panning through the GIF
// while displaying it, or bouncing it around if it's smaller than the display
int OFFSETX = 0;
int OFFSETY = 0;
int FACTX = 0;
int FACTY = 0;

// which gif file do you want to play first?
#define FIRSTINDEX 0

int num_files;

// Setup method runs once, when the sketch starts
void setup() {
    Serial.println("Starting AnimatedGIFs Sketch");
    sav_setup();

    // Seed the random number generator
    // This breaks SmartMatrix output on ESP32
    //randomSeed(analogRead(14));

    #if defined(ESP8266)
	Serial.println();
	Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
	Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
	Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
	Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
	Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
	Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
	Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
	Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
	Serial.println();
    #endif
    #if ENABLE_SCROLLING == 1
	matrix.addLayer(&scrollingLayer); 
    #endif

    // for ESP32 we need to allocate SmartMatrix DMA buffers after initializing
    // the SD card to avoid using up too much memory
    // Determine how many animated GIF files exist
    num_files = enumerateGIFFiles(GIF_DIRECTORY, true);

    if(num_files < 0) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("No gifs directory", -1);
#endif
        die("No gifs directory");
    }

    if(!num_files) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("Empty gifs directory", -1);
#endif
        die("Empty gifs directory");
    }
    Serial.print("Index of files: 0 to ");
    Serial.println(num_files);
    Serial.flush();
    // At least on teensy, due to some framework bug it seems, early
    // serial output gets looped back into serial input
    // Hence, flush input.
    while(Serial.available() > 0) { char t = Serial.read(); t=t; }

    pinMode(BUTTON_PIN, INPUT);

    #if ENABLE_SCROLLING == 1
        scrollingLayer.start("Hello, world!", -1);
    #endif
}

void adjust_gamma(float change) {
#ifdef NEOMATRIX
    matrix_gamma += change;
    matrix->precal_gamma(matrix_gamma);
    Serial.print("Change gamma to: "); 
    Serial.println(matrix_gamma); 
#else
    Serial.println("Gamma changing not supported in SmartMatrix lib"); 
#endif
}

void loop() {
  Serial.println("loop"); 
    static unsigned long lastTime = millis();
    static int index = FIRSTINDEX;
    static int8_t new_file = 1;
    static uint16_t frame = 0;
    // allow stalling on a picture if requested
    static uint32_t longer = 0;
    char readchar;
    // frame by frame display
    static bool debugframe = false;
    bool gotnf = false;
    // clear display before each frame
    static bool clear = false;

    if (Serial.available()) readchar = Serial.read(); else readchar = 0;

    switch(readchar) {
    case 'n': 
	Serial.println("Serial => next"); 
	new_file = 1;  
	index++;
	break;

    case 'p':
	Serial.println("Serial => previous");
	new_file = 1;
	index--;
	break;

    case 'f':
	Serial.println("Serial => debug frames, press 'g' for next frame");
	debugframe = true;
	longer = 3600; //  if frame debugging, keep current gif for 1h
	break;

    case 'g':
	Serial.println("Serial => next frame");
	gotnf = true;
	break;

    case 'c':
	Serial.print("Toggle clear screen: ");
	clear = !clear;;
	Serial.println(clear);
	break;

    case '+': adjust_gamma(+0.2); break;

    case '-': adjust_gamma(-0.2); break;

    // = allows staying on a single picture for up to 1H instead of a few seconds
    case '=':
	longer = longer?0:3600;
	Serial.print("Image display time: "); 
	Serial.println(longer + DISPLAY_TIME_SECONDS); 
	break;

    default:
	// BUG: this does not work for index '0', just type '1', and 'p'
	if (readchar) {
	    while ((readchar >= '0') && (readchar <= '9')) {
		new_file = 10 * new_file + (readchar - '0');
		readchar = 0;
		if (Serial.available()) readchar = Serial.read();
	    }

	    if (new_file) {
		Serial.print("Got new file via serial ");
		Serial.println(new_file);
		index = new_file;
	    } else {
		Serial.print("Got serial char ");
		Serial.println(readchar);
	    }
	}
    }

    if (debugframe) {
	if (! gotnf) return;
    }

    // if we push the button, move to the next gif file
    bool buttonState = digitalRead(BUTTON_PIN);
//    Serial.println(buttonState);
    if (buttonState == LOW && previousButtonState == HIGH){
      lastTime = millis();
      new_file = 1;
      index++;
    }
    previousButtonState = buttonState;

//    if (millis() - lastTime > ((DISPLAY_TIME_SECONDS + longer) * 1000)) {
//	new_file = 1;
//	index++;
//    }

    if (new_file) { 
	frame = 0;
	new_file = 0;
	lastTime = millis();
	if (index >= num_files) index = 0;
	if (index <= -1) index = num_files - 1;
        Serial.print("Fetching file index #");
        Serial.println(index);

        if (openGifFilenameByIndex(GIF_DIRECTORY, index) >= 0) {
            // Can clear screen for new animation here, but this might cause flicker with short animations
	    // matrix->clear();

            decoder.startDecoding();
        } else {
	    die("FATAL: failed to open file");
	}
    }

    if (clear) screenClearCallback();
    decoder.decodeFrame();
    frame++;
    if (debugframe) {
	Serial.print("Displayed frame #");
	Serial.print(frame);
	Serial.println(". Press g for next frame");
    }
#if DEBUGLINE
    delay(1000000);
#endif
}

// vim:sts=4:sw=4
