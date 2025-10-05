#ifndef GLOBALVARS_H
#define GLOBALVARS_H

/*
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
*/

// Globale Variablen und Defines für TFT-Panel-Projekt

#define MAINWINDOW_W 320  // Grafik-Bereich Main-Display
#define MAINWINDOW_H 170  // Höhe des Hauptdisplays

#define ADC1_OFFS 57     // ADC-Nullpunkt DC-Messung, Integer-Rohwert
#define ADC2_OFFS 9
#define ADC_DIV_LORANGE 1000.0  // für Vollausschlag 1 = ADC-Wert 4000 von 0..4095
#define ADC_DIV_HIRANGE 3333.3  // für Vollausschlag 1 = ADC-Wert 4000 von 0..4095
#define ADC_OVERLOAD_DC 4050    // ADC-Rohpegel Overload-Grenze

// Grundskalierung für externen MCP3421. Dieser liefert bei unsymmetrischem Betrieb nur Werte von 0..2047!
#define ADC_DIV_LORANGE_EXT 550.0   // für Vollausschlag 1 = ADC-Wert 4000 von 0..4095
#define ADC_DIV_HIRANGE_EXT 1700.0  // für Vollausschlag 1 = ADC-Wert 4000 von 0..4095
#define ADC_OVERLOAD_DC_EXT 1720    // ADC-Rohpegel Overload-Grenze (extern), eigentlich +2047

#define ADC_DIV_VOLT    3333.3  // für Vollausschlag 1 = ADC-Wert 4000 von 0..4095

#define SCOPETIMER_MS 70 // abhängig von Oszi-Breite, Punkte in X-Richtung (Time)
#define UPDATETIMER_MS SCOPETIMER_MS/2

// https://rgbcolorpicker.com/565

#define TFT_BTNGREY  0x736e    //0x5AEB 16-bit colour, RGB565 format
#define TFT_MEDGREY  0x7BEF     //0x7BEF 16-bit colour, RGB565 format
#define TFT_DIALOGGREY  0x8410     //0x7BEF 16-bit colour, RGB565 format
#define TFT_WINDOWGREY  0x8C51  //0xD69A 16-bit colour, RGB565 format

// #########################################################################

#include <Arduino.h>
#include <SPIFFS.h>
#include <Strings.h>
#include <EEPROM.h>
#include <Ticker.h>  // ESP8266 Ticker, https://github.com/esp8266/Arduino/blob/master/libraries/Ticker/src/Ticker.h
//#include <TFT_eWidget.h>           // use own Widget library
#include <Wire.h>

#include "hwdefs.h" // Hardware pins and definitions for ESP32 board
#include "meterScaleDefaults.h"

#include "MCP3421.h"
#include "TouchProvider.h"
#include <TFT_eSPI.h>
//#include <WiFi.h>
#include <time.h>

#include <SdFat.h>
/*
#include <SD.h>
#include <FS.h>
#include <FS.h>
*/

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library as global

// TouchProvider class for handling touch events as a common provider for all widgets
// This allows the widgets to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly
TouchProvider touchProvider = TouchProvider(&tft); // Initialize touch provider with TFT_eSPI object

// 3 = FAT32
#define SD_FAT_TYPE 3
SdFat SD; // Create an instance of the SD library
// #define SD_CS_PIN 5 // Chip Select Pin für SD-Karte, in platformio.ini definiert
// SPI-Geschwindigkeit
#define SPI_SPEED SD_SCK_MHZ(4)
SPIClass SPI_SD = SPIClass(VSPI); // SPI instance for SD Card
bool SD_OK = false; // true, wenn SD-Karte initialisiert

#define MY_TIMEZONE "CET-1CEST,M3.5.0/02,M10.5.0/03" // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define MY_NTP_SERVER "de.pool.ntp.org"

time_t now; // this is the epoch
struct tm *timeinfo;
bool timeOK = false;

CMCP3421 adc_MCP3421(0.1692);
// CMCP3421 coMCP3421(0.1692); // MCP3421 ADC, 16 Bit, 4 Kanal, 18 Bit Auflösung, 0.1692 V/LSB
//int adcPresent = 0; // true, wenn ADC vorhanden
int adcPresent = 0; // true, wenn ADC vorhanden

// -----------------------------------------------------------------------------
// ------------------------------------------------------------------------------

// Zustand des Messgeräts (Statemachine)
enum instrStates_e {
  state_meterInit = 0, state_meter,
  state_bgInit, state_bg,
  state_scopeInit, state_scope,
  state_setupInit, state_setup,
  state_invalid
};
instrStates_e instrState = state_meterInit;

enum active_measurement_e {
  amps = 0,
  volts
};
active_measurement_e activeMeasurement = amps;  // 0=amps, 1=volts


bool rangeChanged = true;   // true, wenn Messbereich geändert wurde
bool measurementChanged = true;   // true, wenn Messart (Volt/Amps) geändert wurde

typedef char menuArr_t[16][32];
menuArr_t dirArr;

// uint16_t screenBuffer[DISPLAY_W * DISPLAY_H]; // Buffer for screen drawing

// ##############################################################################
// ############################## CREDENTIALS ###################################
// ##############################################################################

// -----------------------------EEPROM-DEFAULTS----------------------------------

// bei Änderung der Settings-Struktur auch diesen Wert ändern:
#define SETTINGS_VALIDFLAG 0x53

// Voreinstellungen und Skalierungen des Messgeräts, in Credentials gespeichert
struct {
  char ssid[32] = "FCKAFD";      // Default Router SSID
  char password[32] = "z28hev111";  // Default Router PW
  //char ssid[32] = "WILHELM.TEL-Z7XLWCE1";      // Default Router SSID
  //char password[32] = "15456528653098957079";  // Default Router PW
  uint16_t wifiWPSpin = 0;

  uint16_t touchCalData[6];
  uint16_t touchCalDataOK = 0; // 0x55A5, auf 0 setzen, um Neukalibrierung zu erzwingen

  uint16_t spkrTick = 1;
  uint16_t spkrBeep = 1;

  bool ampHiRangeOn = true;   // true = Hi Range, false = Lo Range
  int ampRangeIdx = 2;        // 2 = 300mA, 3 = 1A, 4 = 3A
  int voltRangeIdx = 7;       // 7 = 10V, 8 = 30V, 9 = 100V

  int adcRawOffsetVolts = ADC2_OFFS; // ADC-Nullpunkt DC-Messung Strom (0) und Spannung (1), Integer-Rohwert
  int adcRawOffsetAmps  = ADC1_OFFS; // ADC-Nullpunkt DC-Messung Strom (0) und Spannung (1), Integer-Rohwert

  // 10 Messbereiche, Skalierung der ADC-Werte, Reihenfolge wie in meterScaleDefaults.h
  float adcScalings[10]  = {1.002, 1.003, 1, 1, 1, 1, 1.007, 1, 1, 1}; // Amps/Volts-Skalierung

  bool wifiEnabled = false;
  bool wifiAPenabled = false;
  bool wifiWPSused = false;

  // other init values
  int  config_int[6] = {0, 0, 0, 0, 0, 0};
  float config_float[6] = {0.0, 0.0, 0.0, 0.0, 0.5, 0.775};

  bool config_bool[6] = {false, false, false, false, false, false};
} settings;

float markerVolts = 0.0; // Variable to store the set value for volts
float markerAmps = 0.0; // Variable to store the set value for amps

/* Store WLAN credentials to "EEPROM" */
void saveCredentials() {
  // Speichert die WLAN-Zugangsdate in "EEPROM"
  EEPROM.begin(512); // 512 Bytes für EEPROM reservieren
  EEPROM.put(0, settings);
  EEPROM.write(511, SETTINGS_VALIDFLAG);
  EEPROM.commit();
  EEPROM.end();
  DEBUG_PRINTLN("EEPROM saved");
}

/* Load WLAN credentials from "EEPROM" or use predefined strings */
void loadCredentials() {
  EEPROM.begin(512); // 512 Bytes für EEPROM reservieren
  if (EEPROM.read(511) == SETTINGS_VALIDFLAG) {
    EEPROM.get(0, settings);
    EEPROM.end();
    DEBUG_PRINTLN("EEPROM read");
  } else {
    DEBUG_PRINTLN("EEPROM invalid, using defaults");
    EEPROM.end();
    saveCredentials(); // vorbelegte Werte speichern
  }
  markerAmps = settings.config_float[4];
  markerVolts = settings.config_float[5];
}

// ##############################################################################

// Get the directory of files in SPIFFS
// This function retrieves the names of files in the SPIFFS filesystem
// and stores them in the dest_arr array. It returns the number of files found.
int getDirectory(menuArr_t array, char* filter) {
  DEBUG_PRINTLN("######### SPIFFS DIR #########");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  int idx = 0;
  while (file) {
    DEBUG_PRINT(idx);
    DEBUG_PRINT(" = ");
    DEBUG_PRINTLN(file.name());
    if ((strlen(filter) == 0) || (strstr(file.name(), filter) != NULL)) {
      strcpy(array[idx], file.name());
      idx++;
    }
    if (idx > 15)
      break;
    file = root.openNextFile();
  }
  DEBUG_PRINTLN("##############################");
  return idx;
}


// ##############################################################################
// ########################### G U I  HELPERS ###################################
// ##############################################################################

// erzeugt Buzzer-Ton, cycle_delay (us) bestimmt 1/Frequenz
void spkrBeepFrequ(uint16_t len_ms, uint16_t cycle_delay) {
  if (settings.spkrBeep)
    for (uint16_t i = 0; i <= len_ms; i++) {
      digitalWrite(SPKR_PIN, SPKR_ON);
      delayMicroseconds(cycle_delay / 3);
      digitalWrite(SPKR_PIN, SPKR_OFF);
      delayMicroseconds(cycle_delay);
    }
}

// erzeugt mittleren Ton auf Buzzer mit Länge len_ms (ms)
void spkrBeep(uint16_t len_ms) {
  spkrBeepFrequ(len_ms, 900);
}

// erzeugt OK-Tonfolge auf Buzzer
void spkrOKbeep() {
  spkrBeepFrequ(30, 1200);
  delay(30);
  spkrBeepFrequ(40, 900);
}
// erzeugt Cancel/Error-Tonfolge auf Buzzer

void spkrCancelBeep() {
  spkrBeepFrequ(20, 900);
  delay(30);
  spkrBeepFrequ(20, 1200);
  delay(30);
  spkrBeepFrequ(40, 1500);
}

// erzeugt Buzzer-Klickgeräusch
void spkrClick() {
  spkrBeepFrequ(5, 1800);
}

// erzeugt leises Buzzer-Klickgeräusch
void spkrTick() {
  if (settings.spkrTick) {
    spkrBeepFrequ(2, 1200);
  }
}

// ##############################################################################
// ############################### SONSTIGES ####################################
// ##############################################################################

// Scan I2C bus and print device addresses found
// DefaultAddress is 0x68 for MCP3421
// Returns number of devices found
int scan_i2c() {
  byte error, address;
  int nDevices;
  DEBUG_PRINTLN("Scanning I2C...");
  nDevices = 0;
  for(address = 3; address <= 0x77; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      DEBUG_PRINTLN("I2C device found at address 0x");
      DEBUG_PRINTHEX(address);
      nDevices++;
    } else if (error==4) {
      DEBUG_PRINT("Unknow error at address 0x");
      DEBUG_PRINTHEX(address);
    }
  }
  if (nDevices == 0) {
    DEBUG_PRINTLN("No I2C devices found\n");
  } else {
    DEBUG_PRINTLN(" - done\n");
  }
  return nDevices;
}

bool adcIsReady() {
  // Prüft, ob ADC bereit ist
  return adc_MCP3421.IsReady();
}

void adcTrigger() {
  // ADC Triggern
  adc_MCP3421.Trigger();
  // delay(10); // kurze Pause, damit ADC Zeit hat, den Wert zu aktualisieren
}

float adcReadValue() {
  // ADC-Wert lesen und zurückgeben
  if (adc_MCP3421.IsReady()) {
    return adc_MCP3421.ReadValue();
  } else {
    return -1.0; // ADC nicht bereit, Rückgabe -1.0
  }
}

int32_t adcReadRaw() {
  // ADC-Rohwert lesen und zurückgeben
  if (adc_MCP3421.IsReady()) {
    return adc_MCP3421.ReadRaw();
  } else {
    return -1; // ADC nicht bereit, Rückgabe -1.0
  }
}

// ##############################################################################
// ############################ TICKER OBJECTS ##################################
// ##############################################################################

// Global variables and objects
Ticker UpdateTicker, ScopeTicker, EncoderTicker, ToggleTicker, SecondTicker;
int update_tick = 0;
int scope_tick = 0;
int encoder_tick = 0;
int toggle_tick = 0;
int second_tick = 0;
bool toggle_bool = false; // Toggle state for blinking text

// ##############################################################################

void second_tick_callback() {
  // Callback von SecondTicker
  second_tick += 1;
}

void toggle_tick_callback() {
  // Callback von ToggleTicker
  toggle_bool = !toggle_bool;
  toggle_tick += 1;
}

void update_tick_callback() {
  // Callback von UpdateTicker
  update_tick += 1;
}

void scope_tick_callback() {
  // Callback von ScopeTicker
  scope_tick += 1;
}

void encoder_tick_callback() {
  // Callback von EncoderTicker
  touchProvider.encoderTick();
}

// ##############################################################################
// ########################### TOUCHSCREEN ######################################
// ##############################################################################

// TFT Touchscreen Calibration

// This function calibrates the touchscreen and stores the calibration data in EEPROM
void touch_calibrate() {
#ifdef BOARD_CYD
  if (settings.touchCalDataOK == 0x55A1) {
    // calibration data valid
    touchProvider.setTouchCYD(settings.touchCalData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM); // middle center text datum
    tft.drawString(F("Touch corners as indicated"), DISPLAY_W / 2, DISPLAY_H / 2 - 20, 2 );
    touchProvider.calibrateTouchCYD(settings.touchCalData, TFT_MAGENTA, TFT_BLACK, 15);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(F("Calibration complete!"), DISPLAY_W / 2, DISPLAY_H / 2, 2);
    delay(500);
    settings.touchCalDataOK = 0x55A1;
    touchProvider.setTouchCYD(settings.touchCalData);
    saveCredentials(); // store data
    tft.setTextDatum(TL_DATUM); // middle center text datum
    tft.setTextFont(1);
  }
#else
  if (settings.touchCalDataOK == 0x55A1) {
    // calibration data valid
    tft.setTouch(settings.touchCalData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM); // middle center text datum
    tft.drawString(F("Touch corners as indicated"), DISPLAY_W / 2, DISPLAY_H / 2 - 20, 2 );
    tft.calibrateTouch(settings.touchCalData, TFT_MAGENTA, TFT_BLACK, 15);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(F("Calibration complete!"), DISPLAY_W / 2, DISPLAY_H / 2, 2);
    delay(500);
    settings.touchCalDataOK = 0x55A1;
    tft.setTouch(settings.touchCalData);
    saveCredentials(); // store data
    tft.setTextDatum(TL_DATUM); // middle center text datum
    tft.setTextFont(1);
  }
#endif
}

//##############################################################################
//############################## HARDWARE INIT #################################
//##############################################################################

// This implementation uses the VSPI interface for both the SD card and the Touchscreen
// The Touchscreen SPI is deinitialized when the SD card is used and reinitialized afterwards

// Initialize SPI for SD card and end Touchscreen SPI to free the bus
bool start_SD() {
  DEBUG_PRINTLN("End XPT SPI");
  #ifdef BOARD_CYD
    touchProvider.end(); // Deinitialize touch provider SPI to free it for SD card
  #endif
  // SPI bus must be deinitialized before initializing it, otherwise
  // it might throw a "addApbChangeCallback(): duplicate func" exception
  DEBUG_PRINTLN("Start SD SPI");
  SPI_SD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS_PIN); // SCK, MISO, MOSI, SS
  if (!SD_OK) {
    DEBUG_PRINTLN("Start SD FAT");
    SD_OK = SD.begin(SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(10), &SPI_SD));
  }
  return SD_OK;
}

// End SPI for SD card and reinitialize Touchscreen SPI
void end_SD() {
  DEBUG_PRINTLN("End SD, Start XPT SPI");
  SPI_SD.end(); // Deinitialize SPI for SD card to free it for Touchscreen
  #ifdef BOARD_CYD
    touchProvider.reInit(); // Re-Initialize touch provider SPI after SD card use
  #endif
}

void hardwareInit() {
  Serial.begin(115200);    // For debug

  #ifdef DEBUG_STARTUP
    spkrOKbeep();
    delay(2000); // Zeit für Debug-Ausgaben
    DEBUG_PRINTLN("Setup started");
  #endif

  pinMode(TFT_LED_PIN, OUTPUT);
  digitalWrite(XPT2046_CS, HIGH); // Touch controller chip select (if used)
  digitalWrite(TFT_CS, HIGH); // TFT screen chip select
  digitalWrite(SD_CS_PIN, HIGH); // SD card chips select, must use GPIO 5 (ESP32 SS)
  digitalWrite(TFT_LED_PIN, true);
  #ifdef SPKR_PIN
    pinMode(SPKR_PIN, OUTPUT);
    digitalWrite(SPKR_PIN, SPKR_OFF);
  #endif
  Wire.begin(21, 22, 400000U); // SDA-Pin, SCL-Pin, 400 kHz
  delay(50); // kurze Pause, damit I2C Bus initialisiert wird
  adcPresent = scan_i2c(); // Scan I2C bus and print device addresses found
  if (adcPresent) {
    adc_MCP3421.Init(false, adc_MCP3421.eSR_12Bit, adc_MCP3421.eGain_x1);
    DEBUG_PRINTLN("Init I2C ADC");
    adc_MCP3421.Trigger();  // erste Wandlung anstoßen
  }
  time(&now);
  // Set arbitrary time in case WiFi is not available
  timeinfo = localtime(&now);
  timeinfo->tm_hour = 10;
  timeinfo->tm_min = 10;
  timeinfo->tm_sec = 25;
  struct timeval tv;
  tv.tv_sec = mktime(timeinfo);
  tv.tv_usec = 0;
  settimeofday(&tv, NULL); // set time on system

  tft.init();
  #ifdef BOARD_OA
    tft.setRotation(3);
  #else
    tft.setRotation(1);
  #endif
  tft.fillScreen(TFT_BLUE);
  tft.setCursor(0, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextSize(1);
  tft.println(F("TFT Panel initialised"));

  tft.println(F("Start SD SPI..."));

  if (start_SD()) {
    DEBUG_PRINTLN("SD Init done");
    tft.println(F("SD Init done"));
    // Dateien im Verzeichnis ausgeben
    SD.ls(LS_DATE | LS_SIZE | LS_R);
   } else {
    DEBUG_PRINTLN("SD Init failed!");
    tft.println(F("ERROR: SD Init failed or card not present!"));
  }
  end_SD(); // Reinitialize Touch SPI

  tft.println(F("Install tickers..."));
  SecondTicker.attach_ms(1000, second_tick_callback);
  UpdateTicker.attach_ms(UPDATETIMER_MS, update_tick_callback);
  ScopeTicker.attach_ms(SCOPETIMER_MS, scope_tick_callback);
  ToggleTicker.attach_ms(333, toggle_tick_callback); // Toggle every 333 ms, e.g. for blinking text
  toggle_bool = false;
  #ifdef ENCODER_ENABLED
    pinMode(ENCA_PIN, INPUT_PULLUP);
    pinMode(ENCB_PIN, INPUT_PULLUP);
    pinMode(ENCBTN_PIN, INPUT_PULLUP);
    EncoderTicker.attach_ms(2, encoder_tick_callback);
  #endif

  tft.println(F("Loading credentials..."));
  loadCredentials();
  touch_calibrate();  // falls keine Kalibrierdaten vorhanden, neu anlegen

  // Initialize SPIFFS
  if(SPIFFS.begin(true)){
    DEBUG_PRINTLN("Mounting SPIFFS done.");
    tft.println(F("SPIFFS mounted successfully."));
  } else {
    DEBUG_PRINTLN("Error occurred while mounting SPIFFS");
    tft.println(F("ERROR: SPIFFS not mounted!"));
  }
  if (adcPresent == 0) {
    tft.println(F("No I2C Device found, internal ADC used")); // Warning, no I2C device found
  }
  tft.println(F("Hardware Init done"));

  if (start_SD()) {
    SD.ls(LS_DATE | LS_SIZE | LS_R);  // LS_R recursive
  }
  end_SD(); // Reinitialize Touch SPI


}

#endif // GLOBALVARS_H