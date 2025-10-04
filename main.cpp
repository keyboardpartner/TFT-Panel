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

// GUI elements for dual channel DC power supply as example for TFT_eSPI library and
// touch screen handling with XPT2046 or built-in touch screen of ILI9341
// Also suitable for other TFT displays with touch screen like CYD

// 2.4" TFT: http://www.lcdwiki.com/res/MSP2402/2.4inch_SPI_Module_MSP2402_User_Manual_EN.pdf
// Comment Banner logos from: https://patorjk.com/software/taag/#p=display&c=c%2B%2B&f=Banner3
// uint16_t color picker: https://rgbcolorpicker.com/565

#define DEBUG           // Enable debugging messages
//#define DEBUG_STARTUP // Enable startup debugging messages and wait time

#include <Arduino.h>
#include <SPIFFS.h>
#include "Free_Fonts.h" // Include large fonts
#include <TFT_eSPI.h>   // Hardware-specific TFT library
#include "global_vars.h"
#include "panel_gui.h"

#ifdef WIFI_ENABLED
  #include "server.h" // WiFi server for ESP32
#endif




// ##############################################################################
// ############################## G L O B A L S #################################
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
//
//   ######  ######## ######## ##     ## ########
//  ##    ## ##          ##    ##     ## ##     ##
//  ##       ##          ##    ##     ## ##     ##
//   ######  ######      ##    ##     ## ########
//        ## ##          ##    ##     ## ##
//  ##    ## ##          ##    ##     ## ##
//   ######  ########    ##     #######  ##
//
// ##############################################################################

void setup(void) {
  hardwareInit();
  #ifdef DEBUG_STARTUP
    spkrOKbeep();
    delay(2000); // Zeit für Debug-Ausgaben
    DEBUG_PRINTLN("Setup started");
  #endif

  SecondTicker.attach_ms(1000, second_tick_callback);
  UpdateTicker.attach_ms(UPDATETIMER_MS, update_tick_callback);
  ScopeTicker.attach_ms(SCOPETIMER_MS, scope_tick_callback);
  #ifdef ENCODER_ENABLED
    pinMode(ENCA_PIN, INPUT_PULLUP);
    pinMode(ENCB_PIN, INPUT_PULLUP);
    pinMode(ENCBTN_PIN, INPUT_PULLUP);
    EncoderTicker.attach_ms(2, encoder_tick_callback);
  #endif

  ToggleTicker.attach_ms(333, toggle_tick_callback); // Toggle every 333 ms, e.g. for blinking text
  toggle_bool = false;

  loadCredentials();
  touch_calibrate();  // falls keine Kalibrierdaten vorhanden, neu anlegen

 // initDialogs(&tft, &touchProvider);

  // Initialize SPIFFS
  if(SPIFFS.begin(true)){
    DEBUG_PRINTLN("Mounting SPIFFS done.");
  } else {
    DEBUG_PRINTLN("Error occurred while mounting SPIFFS");
    dialogBox.modalDlg("SPIFFS not mounted!", "", 2); // Warning, SPIFFS not mounted
  }
  tft.fillScreen(TFT_BLACK);


  drawBmp("/splash.bmp", 0, 8);
  delay(500);
  DEBUG_PRINTLN("Init done.");

  if (adcPresent == 0) {
    dialogBox.modalDlg("No I2C Device found,", "Internal ADC used", 2); // Warning, no I2C device found
  }

  #ifdef WIFI_ENABLED
    if (settings.wifiEnabled) {
      if (settings.wifiAPenabled)
        wifi_connect_ap();
      else
        wifi_connect_sta();
      settings.wifiEnabled = wifi_connected(); // Check if connected to WiFi
    } else {
      if (dialogBox.modalDlg("Connect to WIFI?", "", 17)) {
        wifi_connect_sta();
        settings.wifiAPenabled = false;
        settings.wifiEnabled = wifi_connected(); // Check if connected to WiFi
      } else {
        stop_server();
      }
    }
  #else
    dialogBox.modalDlg("Launch missiles?", "This will destroy the world!", 17);
  #endif // WIFI_ENABLED
  delay(500);
  spkrOKbeep();
  measurementChanged = true; // Force redraw of numeric display
  enablePageControls(state_invalid);
}


// ##############################################################################
//
//  ##        #######   #######  ########
//  ##       ##     ## ##     ## ##     ##
//  ##       ##     ## ##     ## ##     ##
//  ##       ##     ## ##     ## ########
//  ##       ##     ## ##     ## ##
//  ##       ##     ## ##     ## ##
//  ########  #######   #######  ##
//
// ##############################################################################

void loop() {

  if (second_tick) {
    getLocalTime(timeinfo, 0);
    analogClock.update(timeinfo, false);
    second_tick = 0;
  }

  if (update_tick) {
    update_tick = 0;
    handleGUI(); // Handle GUI events and button presses

    int16_t adc1_raw, adc2_raw; // ADC-Rohwerte
    bool adc1_ovld, adc2_ovld;   // ADC-Overload-Flag

    // Anzeige aktualisieren
    float level_fs_amps, level_fs_volts; // Level in full scale, 0 bis 1.0
    if (adcPresent) {
      // Pegel von externem ADC MCP3421 lesen
      if (adc_MCP3421.IsReady())  {
        adc1_raw = adc_MCP3421.ReadRaw() + settings.adcRawOffsetAmps; // ADC-Wert A-Messung
        adc_MCP3421.Trigger();
        if (settings.ampHiRangeOn) {
          level_fs_amps = (float)(adc1_raw) * settings.adcScalings[settings.ampRangeIdx] / ADC_DIV_HIRANGE_EXT; // interner ADC, umrechnen auf Fullscale = 1.0
          settings.ampRangeIdx = 3; // Hi Range
        }
        else {
          level_fs_amps = (float)(adc1_raw) * settings.adcScalings[settings.ampRangeIdx] / ADC_DIV_LORANGE_EXT; // interner ADC, umrechnen auf Fullscale = 1.0
          settings.ampRangeIdx = 2; // Default Range
        }
        adc1_ovld = (adc1_raw > ADC_OVERLOAD_DC_EXT); // ADC-Wert Overload-Grenze
      }
    } else {
      // Pegel von internem ADC lesen
      adc1_raw = analogRead(DC_PIN_AMPS) + settings.adcRawOffsetAmps; // ADC-Wert A-Messung
      if (settings.ampHiRangeOn) {
        level_fs_amps = (float)(adc1_raw) * settings.adcScalings[settings.ampRangeIdx] / ADC_DIV_HIRANGE; // interner ADC, umrechnen auf Fullscale = 1.0
        settings.ampRangeIdx = 3; // Hi Range
      }
      else {
        level_fs_amps = (float)(adc1_raw) * settings.adcScalings[settings.ampRangeIdx] / ADC_DIV_LORANGE; // interner ADC, umrechnen auf Fullscale = 1.0
        settings.ampRangeIdx = 2; // Default Range
      }
      adc1_ovld = (adc1_raw > ADC_OVERLOAD_DC); // ADC-Wert Overload-Grenze
    }
    if (adc1_ovld)
      adc1_raw = ADC_OVERLOAD_DC; // ADC-Wert Overload-Grenze

    adc2_raw = analogRead(DC_PIN_VOLTS) + settings.adcRawOffsetVolts; // ADC-Wert V-Messung
    adc2_ovld = (adc2_raw > ADC_OVERLOAD_DC); // ADC-Wert Overload-Grenze
    if (adc2_ovld)
      adc2_raw = ADC_OVERLOAD_DC; // ADC-Wert Overload-Grenze

    level_fs_volts = (float)(adc2_raw) * settings.adcScalings[settings.voltRangeIdx] / ADC_DIV_VOLT; // interner ADC, umrechnen auf Fullscale = 1.0

    int active_secondary_measurement = (active_measurement_e)((activeMeasurement + 1) % 2);


    // Update the main display based on the current state
    switch (instrState) {
    case state_meter:
      if (rangeChanged || measurementChanged)
        enableStdControls(state_meterInit);
      if (activeMeasurement == amps) {
        analogMeter.setLevel(level_fs_amps);
      } else {
        analogMeter.setLevel(level_fs_volts);
      }
      break;
    case state_bg:
      if (rangeChanged || measurementChanged)
        enableStdControls(state_bgInit);
      barGraphAmps.update(level_fs_amps, markerAmps);
      barGraphVolts.update(level_fs_volts, markerVolts);
      break;
    case state_scope:
      if (rangeChanged || measurementChanged)
        enableStdControls(state_scopeInit);
      // Scope-Messwerte updaten, Trace zeichnen, Overload/Saved-LED anzeigen
      if (scope_tick) {
        // neuen Scope-Messwert eintragen
        scope_tick = 0;
        scrollingScope.newSample(level_fs_amps, 0); // Scope trace 0
        scrollingScope.newSample(level_fs_volts, 1); // Scope trace 1
        scrollingScope.grid(); // könnte vom Trace überschrieben worden sein
        scrollingScope.trace(0);
        scrollingScope.trace(1);
      }
      barGraphVert.update(level_fs_amps, markerAmps); // Update vertical bar graph for Amps
      break;
    case state_setup:
      // handled by setup button actions
      break;
    default:
      break;
    }

    // update the numeric display range and color
    if (measurementChanged) {
      if (activeMeasurement == amps) {
        analogMeter.setRangeIdxColor(settings.ampRangeIdx); // default red needle
        numericDisplay.setRangeIdxColor(settings.voltRangeIdx, TFT_BLUE);
      } else {
        analogMeter.setRangeIdxColor(settings.voltRangeIdx);
        numericDisplay.setRangeIdxColor(settings.ampRangeIdx, TFT_DARKGREEN);
      }
    }
    // update display and redraw number if enabled
    if (activeMeasurement == amps)
      numericDisplay.setLevel(level_fs_volts);
    else
      numericDisplay.setLevel(level_fs_amps);

    ovldLED.setState(adc1_ovld, false); // disabled in setup page
    #ifdef WIFI_ENABLED
      statusLED.setState(wifi_connected(), true); // Set status LED to On, blinking
    #else
      statusLED.setState(false, false); // Set status LED to Off, not blinking
    #endif

    rangeChanged = false;
    measurementChanged = false;
  }
}
