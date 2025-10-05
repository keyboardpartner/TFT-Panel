# TFT-Panel

### TFT GUI Elements for ESP32 and Other, PlatformIO VSCODE IDE

GUI elements for dual channel DC power supply as example for [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) and
touch screen handling with XPT2046 or built-in touch screen of TFT panel. 
Also suitable for other TFT displays with touch screen like CYD ("Cheap Yellow Display", [ESP32-2432S028R](https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/)) 

All elements are scalable and may be placed anywhere. Demo with display 320x240 px. All GUI elements may be disabled/greyed out on command, useful for inactive controls (for example outside of dialog box or menu list). Analog Meter and clock widgets modified for execution speed and scalability.

The example is *#define*'d for CYD , it provides a built-in web server (AP mode on 192.168.4.1). Board type **BOARD_CYD** (cheap yellow display, using a ST7789 display driver IC) or **BOARD_OA** (homemade, with ILI9341 display driver) are defined in *platformio.ini*, switch environment in PlatformIO accordingly. Hardware defines are located in *hwdefs.h*, other defines in *global_vars.h*. Environment will set pin defines for each board.

The CYD uses a touch controller pinout different to TFT_eSPI default, so a separate driver for XPT2046 touch controller is loaded here (see *platformio.ini*).

### Tips for Cheap Yellow Display (CYD)

The **ESP32-2432S028R CYD** used for evaluation has 2 USB ports (Micro USB and USB-C) and works well with the **ST7789_DRIVER**. It needed the **SPI_READ_FREQUENCY** defined in *platformio.ini* environment section *[env:esp32_CYD]* reduced from 20000000 to **10000000** to read correct colors from screen area. The SPI write speed **SPI_FREQUENCY** may be increased up to 50000000 for faster screen updates.

Other or older CYDs (without USB-C) may need the **ILI9341_2_DRIVER** instead of **ILI9341_DRIVER** and some *#define*s set to work properly. See *platformio.ini* for details. Also, the Gamma values provided in original *ILI9341_Init.h* from TFT_eSPI package show weak and blurried colors. Disabling the Gamma setup will yield much better color rendition. Replace the *ILI9341_Init.h* file in *.pio/libdeps/ESP32_CYD/TFT_eSPI/TFT_Drivers* with the file provided. Please check if your CYD works with the **ST7789_DRIVER** before replacing the *ILI9341_Init.h* file!

For various CYD versions, also see [ESP32 mit 2,8 Zoll TFT (Cheap Yel­low Display)](https://hartmut-waller.info/arduinoblog/esp32-mit-28-zoll-tft/) from Hartmut Waller and [CYD Touch Programming](http://www.geochecker.gps-cache.de/esp8266-esp32/esp32-2432s028-cheap-yellow-display-touchscreen-programmierung-platformio-library-xpt2046-digitizer.htm) from Oliver Kuhlemann (both pages in german).

### Using the SD card on CYD

In order to use the **SD card** in conjunction with the TFT_eSPI library, a certain amount of care is required. The XTP2046 on CYD uses different SPI pins than those expected by the TFT_eSPI library, so the built-in touch functions of TFT_eSPI cannot be used. For the CYD, the separate [XTP2046 library from Paul Stoffregen](https://github.com/PaulStoffregen/XPT2046_Touchscreen) is loaded. We use the ESP32 VSPI for **both SD card and touch controller**. This works fine when SPI pins are switched between SD card and touch usage. See *hardwareInit()* function in *global_vars.h* for details.

### Classes Provided

Button, Switch, LED indicator, Analog Meter,
Numeric Display, Analog Bargraph horizontal with maximum hold (red line) and optional value set point (triangle) by touch
<p>
<img src=./doc/IMG_1055_resize.JPG/ width="400"/>
<img src=./doc/IMG_1056_resize.JPG/ width="400"/>
</p>

Scrolling Oscilloscope Chart, Analog Bargraph vertical with maximum hold (red line) and optional value set point (triangle) by touch
<p>
<img src=./doc/IMG_1057_resize.JPG/ width="400"/>
</p>

Checkbox, Radio Button Group, Tab Control, Numeric Entry with rotary encoder or numeric keypad, Checkbox Group, Slider/Trackbar, Tab Control
<p>
<img src=./doc/IMG_1058_resize.JPG/ width="400"/>
<img src=./doc/IMG_1060_resize.JPG/ width="400"/>
</p>

Numeric Keypad, Dialog Box, Menu List, Analog Clock
<p>
<img src=./doc/IMG_1062_resize.JPG/ width="400"/>
<img src=./doc/IMG_1054_resize.JPG/ width="400"/>
<img src=./doc/IMG_1061_resize.JPG/ width="400"/>
<img src=./doc/IMG_1059_resize.JPG/ width="400"/>
</p>



