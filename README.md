# LED-hourglass-32x16
 
 ![LED Hourglass](demo.gif)

# How to flash your ESP32
1. [Download the latest ````hourglass.bin```` and ````program flash.bat```` from the Releases tab](https://github.com/IHOTT/LED-hourglass-32x16/releases)
2. [Download this flashing tool](https://github.com/espressif/esptool/releases) and copy ````esptool.exe```` to the same place as ````program flash.bat````
3. [Download the USB<->Serial driver here](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers), install and restart
3. Plug in your ESP32 (make sure your USB cable does data AND charging!)
4. Open ````program flash.bat```` and make sure the right COM port is written (check Device Manager -> Ports). You might need to modify/replicate this command line manually if you're not on Windows!
5. Run ````program flash.bat````!

# What can I help with?
Uhh we still don't have a decent enclosure for this, besides the crappy purse form factor it came in. Wanna 3D design/print one? Or come up with something easier? I'm lazy and at a loss. 😅