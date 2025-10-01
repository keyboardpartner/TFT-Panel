#ifndef _HWDEFS_H_
#define _HWDEFS_H_

// ############################################################################
//       __ ________  _____  ____  ___   ___  ___
//      / //_/ __/\ \/ / _ )/ __ \/ _ | / _ \/ _ \
//     / ,< / _/   \  / _  / /_/ / __ |/ , _/ // /
//    /_/|_/___/_  /_/____/\____/_/_|_/_/|_/____/
//      / _ \/ _ | / _ \/_  __/ |/ / __/ _ \
//     / ___/ __ |/ , _/ / / /    / _// , _/
//    /_/  /_/ |_/_/|_| /_/ /_/|_/___/_/|_|
//
// ############################################################################
// Hardware pins and definitions for ESP32 board
#define DISPLAY_W 320     // Anzeigebereich Breite
#define DISPLAY_H 240     // Anzeigebereich Höhe

#define LED_ON 1
#define LED_OFF 0
#define SPKR_ON 1
#define SPKR_OFF 0

#define WIFI_ENABLED    // Enable WiFi support
// #define ENCODER_ENABLED    // Enable encoder support

#ifdef BOARD_OA
  #define LED_PIN 2  // 21 bei erster Platinenversion, für Sampling-Kontrolle
  #define TFT_LED_PIN 15
  #define SPKR_PIN 13
  #define DC_PIN_VOLTS 36 // Range ADC
  #define DC_PIN_AMPS 39

  #define ENCA_PIN 34
  #define ENCB_PIN 35
  #define ENCBTN_PIN 32  // 22 bei erster Platinenversion ohne DC-ADC

  /*
  // TFT-Pins in platformio.ini zugewiesen:
    -D TFT_CS=5
    -D TFT_DC=12
    -D TFT_RST=-1
    -D TFT_MOSI=23
    -D TFT_MISO=-1
    -D TFT_SCLK=18
    -D TOUCH_CS=27
  */
#endif

#ifdef BOARD_CYD
  #define LED_PIN 4 // RGB-LED, pins 17, 4, 16
  #define TFT_LED_PIN 21
  #define SPKR_PIN 26
  #define DC_PIN_VOLTS LDR_PIN
  #define DC_PIN_AMPS 35
  #undef ENCODER_ENABLED    // No encoder support
#endif


#ifdef DEBUG
	#define DEBUG_PRINT(x) Serial.print(x)
	#define DEBUG_PRINTLN(x) Serial.println(x)
	#define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
	#define DEBUG_PRINTHEX(x) Serial.printf("0x%04X", x)
#else
	#define DEBUG_PRINT(x)
	#define DEBUG_PRINTLN(x)
	#define DEBUG_PRINTF(fmt, ...)
	#define DEBUG_PRINTHEX(x)
#endif


#endif