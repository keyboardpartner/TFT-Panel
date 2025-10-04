# TFT-Panel

### TFT GUI Elements for ESP32 and Other, PlatformIO VSCODE IDE

GUI elements for dual channel DC power supply as example for [TFT_eSPI library](https://github.com/Bodmer/TFT_eSPI) and
touch screen handling with XPT2046 or built-in touch screen of ILI9341 TFT panel. 
Also suitable for other TFT displays with touch screen like CYD ("Cheap Yellow Display", [ESP32-2432S028R](https://randomnerdtutorials.com/cheap-yellow-display-esp32-2432s028r/)) 

All elements are scalable and may be placed anywhere. Demo with display 320x240 px. All GUI elements may be disabled/greyed out on command, useful for inactive controls (for example outside of dialog box or menu list). Analog Meter and clock widgets modified for execution speed and scalability.

The example is #define'd for CYD , it provides a built-in web server (AP mode on 192.168.4.1). Board type **BOARD_CYD** (cheap yellow display) or **BOARD_OA** (homemade) are defined in *platformio.ini*, switch environment in Platformio accordingly. Hardware defines are located in *hwdefs.h*, other defines in *global_vars.h*. Environment will set pin defines for each board.

The CYD uses a touch controller pinout different to TFT_eSPI default, so a separate driver for XPT2046 touch controller is loaded here (see *platformio.ini*).

### Tips for Cheap Yellow Display

The CYD needs the **ILI9341_2_DRIVER** and some *#define*s set to work properly. See *platformio.ini* for details. The display used for evaluation needed **SPI_READ_FREQUENCY** reduced from 20000000 to 10000000 to show correct colors. Also, the Gamma values provided in original *ILI9341_Init.h* from TFT_eSPI package show weak and blurried colors. Disabling the Gamma setup will yield much better color rendition. Replace the *ILI9341_Init.h* file in *.pio/libdeps/ESP32_CYD/TFT_eSPI/TFT_Drivers* with the file provided.

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



