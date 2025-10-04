// #####################################################################################
//
//       __ ________  _____  ____  ___   ___  ___
//      / //_/ __/\ \/ / _ )/ __ \/ _ | / _ \/ _ \
//     / ,< / _/   \  / _  / /_/ / __ |/ , _/ // /
//    /_/|_/___/_  /_/____/\____/_/_|_/_/|_/____/
//      / _ \/ _ | / _ \/_  __/ |/ / __/ _ \
//     / ___/ __ |/ , _/ / / /    / _// , _/
//    /_/  /_/ |_/_/|_| /_/ /_/|_/___/_/|_|
//
// Banner logos from: https://patorjk.com/software/taag/#p=display&c=c%2B%2B&f=Banner3
//
// Addittion by cm 7/25:
// TouchProvider class for handling touch events as a common provider for all widgets
// This allows the widgets to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly

// Function to get touch coordinates from the TFT display
// It returns pressed = true if a touch is detected, false otherwise
// The coordinates are stored in the tx and ty variables
// The procedure touchProvider.checkTouch();
// must be used in the main handler or any modal dialog wait loops
// to check for touch events
// #####################################################################################

#ifndef PANEL_GUI_H
#define PANEL_GUI_H

#include <Arduino.h>
#include <TFT_eSPI.h>

#ifdef WIFI_ENABLED
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WiFiAP.h>
  #include <esp_wps.h>
  #include "server.h"
#endif // WIFI_ENABLED

#include "global_vars.h"
#include "buttons.h"
#include "switches.h"
#include "checkBox.h"
#include "Free_Fonts.h"
#include "indicators.h"
#include "radioButtonGroup.h"
#include "numericDisplay.h"
#include "TouchProvider.h"
#include "dialogBox.h" // Include dialog box for modal dialogs
#include "modalMenuList.h" // Include modal menu list for selection dialogs
#include "tabControls.h"
#include "checkBoxGroup.h" // Include checkbox group widget for multiple selections
#include "keypad.h"  // Include keypad for numeric input
#include "sliders.h"  // Include slider widget for continuous input

// Complex objects, not inherited from GUIobject
#include "analogMeter.h"
#include "scrollingScope.h"
#include "bargraphs.h"
#include "meterScaleDefaults.h"
#include "encoderEntry.h" // Include encoder entry widget for numeric input with encoder
#include "clock.h"


#define BUTTON_W 70
#define BUTTON_W2 35  // halbe Button-Breite
#define BUTTON_H 34
#define BUTTON_H2 17  // halbe Button-Höhe
#define WIDEBUTTON_W 96
#define WIDEBUTTON_W2 48  // halbe Button-Breite
#define WIDEBUTTON_H 34
#define WIDEBUTTON_H2 17  // halbe Button-Höhe

#define SMALLBUTTON_W 50
#define SMALLBUTTON_H 18

#define NUM_POS_X (DISPLAY_W - NUMERICS_WIDTH + 2)  // Numeric display X position
#define NUM_POS_Y (DISPLAY_H - NUMERICS_HEIGHT + 2) // Numeric display Y position

// Objects used in the GUI. Most belong to base class GUIobject and share
// common properties and methods for handling touch events and drawing.

SlideSwitch switchRange = SlideSwitch(&tft, &touchProvider);      // Range switch widget

PushButton offsetBtn  = PushButton(&tft, &touchProvider);       // modal dialog Button
PushButton setupBtn  = PushButton(&tft, &touchProvider);        // modal dialog Button
PushButton saveBtn  = PushButton(&tft, &touchProvider);         // Quit Button
PushButton exitBtn  = PushButton(&tft, &touchProvider);       // Cancel Button
PushButton dirBtn  = PushButton(&tft, &touchProvider);          // Directory Button
PushButton touchCalBtn  = PushButton(&tft, &touchProvider);     // Touch Calibration Button
PushButton scanWifiBtn  = PushButton(&tft, &touchProvider);     // Scan WiFi Button
PushButton startWPSBtn  = PushButton(&tft, &touchProvider);     // Start WPS Button
#ifndef ENCODER_ENABLED
  // invisible buttons
  PushButton leftWipeBtn  = PushButton(&tft, &touchProvider);     // Left Wipe Button
  PushButton rightWipeBtn  = PushButton(&tft, &touchProvider);    // Right Wipe Button
#endif
LedIndicator statusLED = LedIndicator(&tft, &touchProvider);    // LED toggle widget
LedIndicator ovldLED = LedIndicator(&tft, &touchProvider);      // LED toggle widget

Checkbox enaBeepCheckbox = Checkbox(&tft, &touchProvider); // Checkbox widget
Checkbox wifiEnaCheckbox = Checkbox(&tft, &touchProvider); // Checkbox widget
Checkbox wifiAPCheckbox = Checkbox(&tft, &touchProvider);  // Checkbox widget

NumericDisplay numericDisplay = NumericDisplay(&tft, &touchProvider); // Initialize numeric display with TFT_eSPI object

RadioButtons radioButtons = RadioButtons(&tft, &touchProvider); // A Radio button group widget
labelArray_t  radioLabels = {"Beep 1", "Test 2", "Radio 3", "Radio 4", "Radio 5"}; // up to 10 radio buttons

CheckboxGroup optionCheckboxGroup = CheckboxGroup(&tft, &touchProvider);
labelArray_t  checkLabels = {"Option 1", "FCK TRMP", "Another Option", "Always bother me", "This is no option"}; // up to 10 checkbox buttons

BottomTabs setupTabs = BottomTabs(&tft, &touchProvider);
labelArray_t  tabLabels = {"Setup", "WiFi", "Test", "Dummy"}; // Some tab labels for the setup page tabs, up to 10 tabs

SliderHor slider1 = SliderHor(&tft, &touchProvider);    // Initialize horizontal slider widget
SliderVert slider2 = SliderVert(&tft, &touchProvider);  // Initialize vertical slider widget

// Dialog box, modal menu and numeric keypad do not use the base class GUIobject
// and must be handled separately.
// Initialize numeric keypad with TFT_eSPI object and touch provider
NumericKeypad numericKeypad = NumericKeypad(&tft, &touchProvider);
DialogBox dialogBox = DialogBox(&tft, &touchProvider);
ModalMenu modalMenu = ModalMenu(&tft, &touchProvider);

EncoderEntryField encoderEntry = EncoderEntryField(&tft, &touchProvider);

AnalogClock analogClock = AnalogClock(&tft, &touchProvider);


// Create a list of GUI objects for loop touch handling and easy enabling/activating
// This allows the main loop to check for touch events on all GUI objects
// Objects are handled in this order in the GUI "checkPressed()" main loop
GUIObject *guiObjects[] = {
  &setupBtn, &statusLED, &ovldLED, &switchRange, &numericDisplay,
  #ifndef ENCODER_ENABLED
    &leftWipeBtn, &rightWipeBtn,
  #endif
  // setupTabs will erase area on redraw, so all buttons on tabs must follow after it
  &setupTabs, &offsetBtn, &saveBtn, &exitBtn,
  &enaBeepCheckbox, &touchCalBtn, &dirBtn, &radioButtons,
  &encoderEntry,
  &scanWifiBtn, &startWPSBtn, &wifiEnaCheckbox, &wifiAPCheckbox, &analogClock,
  &slider1, &slider2, &optionCheckboxGroup
};
const int guiObjectsCount = sizeof(guiObjects) / sizeof(guiObjects[0]);

// Group mask bits, may be combined (OR-ed) with others to have a control appear on multiple pages
#define PAGE_MAIN 1   // bit 0
#define PAGE_SETUP 2  // bit 1
#define PAGE_WIFI 4   // bit 2
#define PAGE_OPTIONS 8 // bit 3
#define PAGE_ALL (PAGE_MAIN | PAGE_SETUP | PAGE_WIFI | PAGE_OPTIONS) // appears on all pages
#define PAGE_ALLTABS (PAGE_SETUP | PAGE_WIFI | PAGE_OPTIONS)  // appears on all tab pages

// Object masks for each GUI object as in *guiObjects[], determine if object is handled in group
// Objects are handled in this order in activate/enable function's loop
const uint32_t guiObjectGroups[guiObjectsCount] = {
  PAGE_MAIN, PAGE_MAIN, PAGE_MAIN, PAGE_MAIN, PAGE_MAIN,
  #ifndef ENCODER_ENABLED
    PAGE_MAIN, PAGE_MAIN,
  #endif
  // setupTabs will erase its area on redraw, so all buttons contained must follow after it
  PAGE_ALLTABS, PAGE_SETUP, PAGE_ALLTABS, PAGE_ALLTABS,
  PAGE_SETUP, PAGE_SETUP, PAGE_SETUP, PAGE_SETUP,
  PAGE_SETUP,
  PAGE_WIFI, PAGE_WIFI, PAGE_WIFI, PAGE_WIFI, PAGE_WIFI,
  PAGE_OPTIONS, PAGE_OPTIONS, PAGE_OPTIONS
};

// Create a list of GUI objects for loop updating
// These do not have a "pressed" check and are handled separately
GUIObject *guiUpdateObjects[] = {
  &statusLED, &ovldLED, &numericDisplay // all on PAGE_MAIN
};
const int guiUpdateObjectsCount = sizeof(guiUpdateObjects) / sizeof(guiUpdateObjects[0]);

// Complex objects, not inherited from GUIobject
AnalogMeter analogMeter = AnalogMeter(&tft); // Initialize analog meter with TFT_eSPI object
ScrollingScope scrollingScope = ScrollingScope(&tft); // Initialize scrolling scope with TFT_eSPI object

// use touchProvider, to disable touch events remove "&touchProvider" overload on bargraphs
HorizontalBargraph barGraphAmps = HorizontalBargraph(&tft, &touchProvider); // Initialize horizontal bar graph with TFT_eSPI object
HorizontalBargraph barGraphVolts = HorizontalBargraph(&tft, &touchProvider); // Initialize horizontal bar graph with TFT_eSPI object
VerticalBargraph barGraphVert = VerticalBargraph(&tft, &touchProvider); // Initialize vertical bar graph with TFT_eSPI object


int setupTabIndex = 0;
int oldRangeIdx = -1;

// ##############################################################################
//
//  ########  ##     ## ########
//  ##     ## ###   ### ##     ##
//  ##     ## #### #### ##     ##
//  ########  ## ### ## ########
//  ##     ## ##     ## ##
//  ##     ## ##     ## ##
//  ########  ##     ## ##
//
// ##############################################################################

// Show Bitmap File

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

// Draw a BMP file from the SPIFFS file system to the TFT display
void drawBmp(const char *filename, int16_t x, int16_t y) {
  if ((x >= DISPLAY_W) || (y >= DISPLAY_H)) return;
  fs::File bmpFS;
  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");
  if (!bmpFS) {
    DEBUG_PRINTLN("File not found");
    return;
  }
  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t  r, g, b;
  uint32_t startTime = millis();
  if (read16(bmpFS) == 0x4D42) {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)) {
      y += h - 1;
      bool oldSwapBytes = tft.getSwapBytes();
      tft.setSwapBytes(true);
      bmpFS.seek(seekOffset);
      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];
      for (row = 0; row < h; row++) {
        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t*  bptr = lineBuffer;
        uint16_t* tptr = (uint16_t*)lineBuffer;
        // Convert 24 to 16-bit colours
        for (uint16_t col = 0; col < w; col++) {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
        }
        // Push the pixel row to screen, pushImage will crop the line if needed
        // y is decremented as the BMP image is drawn bottom up
        tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
      }
      tft.setSwapBytes(oldSwapBytes);
      DEBUG_PRINT("Loaded in "); DEBUG_PRINT(millis() - startTime);
      DEBUG_PRINTLN(" ms");
    }
    else DEBUG_PRINTLN("BMP format not recognized.");
  }
  bmpFS.close();
}

// ##############################################################################
//
// ##      ## #### ######## ####
// ##  ##  ##  ##  ##        ##
// ##  ##  ##  ##  ##        ##
// ##  ##  ##  ##  ######    ##
// ##  ##  ##  ##  ##        ##
// ##  ##  ##  ##  ##        ##
//  ###  ###  #### ##       ####
//
// ##############################################################################

// "wifiConnect.h" uses some buttons and dialog boxes.
// It is not a real header file, just a collection of functions.
#ifdef WIFI_ENABLED
  #include "wifiConnect.h"
#endif


// ##############################################################################
//
//  ######## ##    ##    ###    ########  ##       ########
//  ##       ###   ##   ## ##   ##     ## ##       ##
//  ##       ####  ##  ##   ##  ##     ## ##       ##
//  ######   ## ## ## ##     ## ########  ##       ######
//  ##       ##  #### ######### ##     ## ##       ##
//  ##       ##   ### ##     ## ##     ## ##       ##
//  ######## ##    ## ##     ## ########  ######## ########
//
// ##############################################################################

// Each GUIObject has a mask that determines which group it belongs to.
// The mask is a 32-bit integer where each bit represents a different group,
// so up to 32 groups can be defined.
// When the GUIObject's group bit is set, the control belongs to that group.
// This makes it easy to manage groups of controls together like in tabs or pages:
// Controls can be shown or hidden together based on the active tab or page.

// Group bits may be combined when an object belongs to multiple groups,
// useful when an object appears in different contexts or pages.
// The array guiObjectGroups[] holds the mask values for each GUIObject.

// set each control mask from the GUI mask array, to be called once during initialization
void initControlMasks() {
  DEBUG_PRINTLN("Set all Control Masks");
  for (int idx = 0; idx < guiObjectsCount; idx++) {
    guiObjects[idx]->setMask(guiObjectGroups[idx]); // use the generic 32 bit mask flag
  }
}

// Set the controls of a specific group enabled or disabled
void enableControlGroup(int32_t group_mask, bool enabled) {
  DEBUG_PRINT("Set Controls Enabled = ");
  DEBUG_PRINT(enabled);
  DEBUG_PRINT(", mask = ");
  DEBUG_PRINTLN(group_mask);
  for (int idx = 0; idx < guiObjectsCount; idx++) {
    if (guiObjects[idx]->getMask() & group_mask) // Check if the control belongs to the specified group
      guiObjects[idx]->setEnabled(enabled);
  }
}

// Set the controls of a specific group active or inactive (greyed out if inactive)
void activateControlGroup(int32_t group_mask, bool active) {
  DEBUG_PRINT("Set Controls Active = ");
  DEBUG_PRINT(active);
  DEBUG_PRINT(", mask = ");
  DEBUG_PRINTLN(group_mask);
  for (int idx = 0; idx < guiObjectsCount; idx++) {
    if (guiObjects[idx]->getMask() & group_mask) // Check if the control belongs to the specified group
      guiObjects[idx]->setActive(active);
  }
}

// Disable all controls, regardless of group
void disableAllControls() {
  DEBUG_PRINTLN("Disable all Controls");
  for (int idx = 0; idx < guiObjectsCount; idx++) {
    guiObjects[idx]->setEnabled(false);
  }
}

// Draws all controls when they are set to "enabled".
// Greys out visible/enabled controls when active = false.
// Objects should be greyed out when a menu or modal dialog is in foreground
void drawEnabledControls(bool active) {
  DEBUG_PRINT("Draw active controls, active = ");
  DEBUG_PRINTLN(active);
  for (int idx = 0; idx < guiObjectsCount; idx++) {
    guiObjects[idx]->redraw(active);
  }
}

// Draws all controls belonging to a specified group.
// Greys out these controls when active = false.
// Objects should be greyed out when a menu or modal dialog is in foreground
void drawControlGroup(int32_t group_mask, bool active) {
  DEBUG_PRINT("Draw Controls Active = ");
  DEBUG_PRINT(active);
  DEBUG_PRINT(", mask = ");
  DEBUG_PRINTLN(group_mask);
  disableAllControls();
  enableControlGroup(group_mask, true);
  drawEnabledControls(active);
}

// ##############################################################################
//
//  ########     ###     ######   ########  ######
//  ##     ##   ## ##   ##    ##  ##       ##    ##
//  ##     ##  ##   ##  ##        ##       ##
//  ########  ##     ## ##   #### ######    ######
//  ##        ######### ##    ##  ##             ##
//  ##        ##     ## ##    ##  ##       ##    ##
//  ##        ##     ##  ######   ########  ######
//
// ##############################################################################

void initControls(); // forward declaration

// Standard measurement pages
// will enable and draw main page controls
void enableStdControls(instrStates_e newState) {
  if (newState == state_invalid) return; // first valid state
  drawControlGroup(PAGE_MAIN, true); // draw group controls in active state
  // Set the range index to opposite measurement
  if (activeMeasurement == amps)
   numericDisplay.setRangeIdxColor(settings.voltRangeIdx, TFT_BLUE);
  else
    numericDisplay.setRangeIdxColor(settings.ampRangeIdx, TFT_DARKGREEN);
  numericDisplay.setLevel(-0.1, true); // Force update display and redraw

  barGraphVert.setEnabled(false);
  barGraphAmps.setEnabled(false);
  barGraphVolts.setEnabled(false);
  switch (newState) {
    case state_meterInit:
      // Initialize meter controls
      instrState = state_meter; // next state
      analogMeter.init(0, 0, MAINWINDOW_W, MAINWINDOW_H);
      if (activeMeasurement == amps) {
        analogMeter.setRangeIdxColor(settings.ampRangeIdx);
      } else {
        analogMeter.setRangeIdxColor(settings.voltRangeIdx); // gets values from meterScaleDefaults
      }
      break;
    case state_bgInit:
      // Initialize bargraph touch controls
      instrState = state_bg; // next state
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextFont(4);
      tft.drawCentreString("TFT Panel Meter", DISPLAY_W/2, 5, 4);
      barGraphAmps.init(0, 30, MAINWINDOW_W, 55);
      barGraphAmps.setRangeIdxColor(settings.ampRangeIdx, TFT_GREEN, true);
      barGraphAmps.update(0, markerAmps, true);     // Draw scale and bar
      barGraphVolts.init(0, 100, MAINWINDOW_W, 55);
      barGraphVolts.setRangeIdxColor(settings.voltRangeIdx, TFT_BLUE, true);
      barGraphVolts.update(0, markerVolts, true);     // Draw scale and bar
      break;
    case state_scopeInit:
      // Initialize scope controls
      instrState = state_scope; // next state
      scrollingScope.init(5, 0, 240, MAINWINDOW_H);
      scrollingScope.newTrace(TFT_GREEN, settings.ampRangeIdx, 0, activeMeasurement == amps); // Init trace 0, Amps
      scrollingScope.newTrace(TFT_CYAN, settings.voltRangeIdx, 1, activeMeasurement == volts); // Init trace 1, Volts
      barGraphVert.init(250, 0, 70, MAINWINDOW_H);
      barGraphVert.setRangeIdxColor(settings.ampRangeIdx, TFT_GREEN, true);
      barGraphVert.update(0, markerAmps, true); // Redraw vertical bar graph for Amps
      break;
    default:
      break;
  }
  tft.setTextSize(1);
  tft.setTextFont(2);
  tft.setTextDatum(BC_DATUM);  // Bottom center text datum
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Secondary Display", DISPLAY_W - NUMERICS_WIDTH / 2, DISPLAY_H - NUMERICS_HEIGHT - 2);
  tft.setTextDatum(TL_DATUM);  // Top left text datum
}

void enableTabControls(int16_t tab_idx) {
  disableAllControls(); // Disable all controls first
  switch (tab_idx) {
    case 0:
      drawControlGroup(PAGE_SETUP, true);
      break;
    case 1:
      drawControlGroup(PAGE_WIFI, true);
      analogClock.update(timeinfo, true);
      break;
    case 2:
      drawControlGroup(PAGE_OPTIONS, true);
      break;
    default:
      break;
  }
}

// Clear Screen, initialize the main page with all controls needed
// and set new instrState
void enablePageControls(instrStates_e newState) {
  DEBUG_PRINTLN("Init MAIN page");
  switch (newState) {
    case state_invalid:
      initControls(); // Initialize controls only on startup
      newState = state_meterInit; // first valid state
      // controls are active, visible and enabled, but not drawn yet.
      // continue with state_meterInit, no break!
    case state_meterInit:
    case state_bgInit:
    case state_scopeInit:
      // Initialize meter controls
      touchProvider.resetEncDelta();
      tft.fillScreen(TFT_BLACK); // also clear screen on startup as instrState changes to state_meterInit
      enableStdControls(newState); // will enable and draw main page controls
      break;
    case state_setupInit:
      // Initialize setup controls to last open tab
      tft.fillScreen(TFT_BLACK);
      setupTabIndex = 0;
      instrState = state_setup; // next state
      enableTabControls(setupTabIndex);
      break;
    default:
      break;
  }
}


// ##############################################################################
//
//     ###     ######  ######## ####  #######  ##    ##
//    ## ##   ##    ##    ##     ##  ##     ## ###   ##
//   ##   ##  ##          ##     ##  ##     ## ####  ##
//  ##     ## ##          ##     ##  ##     ## ## ## ##
//  ######### ##          ##     ##  ##     ## ##  ####
//  ##     ## ##    ##    ##     ##  ##     ## ##   ###
//  ##     ##  ######     ##    ####  #######  ##    ##
//
// ##############################################################################

// Handle actions assigned in GUIobject's setPressAction() method


// Done button pressed action
// This function is called when the Done button is pressed
void saveBtnPressed(void) {
  spkrClick();
  saveCredentials();
  drawEnabledControls(false);  // grey out enabled controls
  dialogBox.message(F("Settings saved"), F("to EEPROM memory"), 1000);
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}

void exitBtnPressed(void) {
  spkrClick();
  tft.fillScreen(TFT_BLACK);
  enablePageControls(state_meterInit);
}

void touchCalBtnPressed(void) {
  spkrClick();
  drawEnabledControls(false);  // grey out enabled controls
  settings.touchCalDataOK = 0; // Reset calibration data
  saveCredentials(); // store data
  dialogBox.message(F("Calibration data reset"), F("Reboot to apply"), 1000);
  DEBUG_PRINTLN("Calibration data reset");
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}

// Directory button pressed action
// It retrieves the directory entries, and allows the user to select a file to load
void dirBtnPressed(void) {
  spkrClick();
  drawEnabledControls(false);
  // here we must wait for button release as modalListSelect disables it
  touchProvider.waitReleased(); // Wait for button release
  int dir_entries = getDirectory(dirArr,  "");
  int file_num = modalMenu.select(dirArr, dir_entries, "Select file to load");
  if (file_num >= 0) {
    tft.setTextDatum(TL_DATUM);  // Top left text datum
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(dirArr[file_num], 170, 110, 2);
  }
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}

// Radio button press action
void radioBtnPressed(void) {
  int idx = radioButtons.getSelectedItem() + 1;
  DEBUG_PRINT("Radio button pressed: ");
  DEBUG_PRINTLN(idx);
  for (int i = 0; i < idx; i++) {
    spkrBeep(66); // Beep for 66 ms
    delay(66); // Wait for 66 ms
  }
}

void wifiEnaCheckboxPressed(void) {
  spkrClick();
  drawEnabledControls(false);
  if (wifiEnaCheckbox.isChecked()) {
    #ifdef WIFI_ENABLED
      wl_status_t status = WL_DISCONNECTED; // Initialize status to disconnected
      settings.wifiEnabled = true;
      status = WiFi.status(); // Get the current WiFi status
      if (status != WL_CONNECTED) {
        settings.wifiAPenabled = wifiAPCheckbox.isChecked();
        if (settings.wifiAPenabled) {
          status = wifi_connect_ap();
        } else {
          status = wifi_connect_sta();
          if (status != WL_CONNECTED) {
            wifiEnaCheckbox.setState(false, true);
          }
        }
        #ifdef DEBUG
          Serial.println("AsyncWebServer and ElegantOTA started");
          Serial.print("on Server IP Address: ");
          if (settings.wifiAPenabled) {
            Serial.println(WiFi.softAPIP());
          } else {
            Serial.println(WiFi.localIP());
          }
        #endif
      }
    #endif
  } else {
    #ifdef WIFI_ENABLED
      settings.wifiEnabled = false;
      stop_server(); // Stop the server if connected
      WiFi.disconnect();
    #endif
  }
  #ifndef WIFI_ENABLED
    wifiEnaCheckbox.setState(false, true);
    dialogBox.message(F("WIFI disabled in FW"), "", 2000, 2); // Show message if WIFI is not enabled
  #endif
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}

void wifiAPCheckboxPressed(void) {
  spkrClick();
  drawEnabledControls(false);
  #ifdef WIFI_ENABLED
    if (settings.wifiAPenabled && !wifiAPCheckbox.isChecked()) {
      if (setupTabs.isEnabled())
        setupTabs.setActive(false, true);
      // If AP mode is enabled and checkbox is unchecked, disable AP mode
      WiFi.softAPdisconnect(true);
      stop_server(); // Stop the server if connected
      wifiEnaCheckbox.setState(false, true);
      settings.wifiEnabled = false; // Disable AP mode
    }
  #else // not WIFI_ENABLED
    dialogBox.message(F("WIFI disabled in FW"), "", 2000, 2); // Show message if WIFI is not enabled
  #endif
  settings.wifiAPenabled = wifiAPCheckbox.isChecked();
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}

menuArr_t meterMenuEntries= {
    "Scaling 30mA", // #0 30mA
    "Scaling 100mA", // #1 100mA
    "Scaling 300mA", // #2 300mA
    "Scaling 1A", // #3 1A
    "Scaling 3A", // #4 3A
    "Scaling 1V",  // #5 1V
    "Scaling 3V",  // #6 3V
    "Scaling 10V", // #7 10V
    "Scaling 30V", // #8 30V
    "Scaling 100V", // #9 100V
    "Offset  Amps", // #10 Offset Amps
    "Offset  Volts" // #11 Offset Volts
} ;

void offsetBtnPressed(void) {
  menuArr_t* menu;
  int menu_len = 12; // Setup state has 12 items
  menu = &meterMenuEntries;  // May be changed to different menu string array
  spkrClick();
  drawEnabledControls(false);
  // Open the menu
  int menu_item = modalMenu.select(*menu, menu_len, "Select menu item");
  if (menu_item < 0) {
    drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
    return;
  }
  float value;
  numericKeypad.init(30, 10, 260, 220, TFT_WINDOWGREY); // Initialize numeric keypad position and size
  if (menu_item < 10) {
    numericKeypad.setEntryValue(settings.adcScalings[menu_item]);
    value = numericKeypad.entry(meterMenuEntries[menu_item], 3, true); // 3 decimals
    if (numericKeypad.isEntryValid()) {
      settings.adcScalings[menu_item] = value;
    }
  } else {
    switch (menu_item) {
      case 10:
        // Numeric keypad for entering Amps Offset
        numericKeypad.setEntryValue(settings.adcRawOffsetAmps);
        value = rint(numericKeypad.entry(meterMenuEntries[menu_item], 0, true));
        if (numericKeypad.isEntryValid()) {
          settings.adcRawOffsetAmps = rint(value);
        }
        break;
      case 11:
        // Numeric keypad for entering Volts Offset
        numericKeypad.setEntryValue(settings.adcRawOffsetVolts);
        value = numericKeypad.entry(meterMenuEntries[menu_item], 0, true);
        if (numericKeypad.isEntryValid()) {
          settings.adcRawOffsetVolts = rint(value);
        }
        break;
      default: // CANCEL
        break;
    }
  }
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}


void scanWifiBtnPressed(void) {
  spkrClick();
  drawEnabledControls(false);
  #ifdef WIFI_ENABLED
    menuArr_t wifiArr;
    int n = wifi_scanNetworks(wifiArr);
    int menu_item = modalMenu.select(wifiArr, n, "Select WIFI network");
  #else // not WIFI_ENABLED
    dialogBox.message(F("WIFI disabled in FW"), "", 2000, DB_ERROR); // Show message if WIFI is not enabled
  #endif
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}

void startWPSBtnPressed(void) {
  spkrClick();
  drawEnabledControls(false);
  //drawEnabledControls(false);
  #ifdef WIFI_ENABLED
    wps_connect(); // Start WPS connection
  #else // not WIFI_ENABLED
    dialogBox.message(F("WIFI disabled in FW"), "", 2000, DB_ERROR); // Show message if WIFI is not enabled
  #endif
  drawEnabledControls(true);   // redraw inactive controls when menu is closed to restore window
}

void enaBeepPressed(void) {
  spkrClick();
  settings.spkrBeep = enaBeepCheckbox.isChecked();
}

void optionsPressed(void) {
  spkrClick();
  int16_t idx = optionCheckboxGroup.getSelectedItem(); // last pressed item
  settings.config_bool[idx] = optionCheckboxGroup.getItemState(idx);
}

void slider1Pressed(void) {
  settings.config_float[0] = slider1.getLevel();
}

void slider2Pressed(void) {
  settings.config_float[1] = slider2.getLevel();
}

void setupTabPressed(void) {
  spkrClick();
  // Open the selected setup tab
  setupTabIndex = setupTabs.getSelectedItem();
  enableTabControls(setupTabIndex);
  DEBUG_PRINT("Tab changed to ");
  DEBUG_PRINTLN(setupTabIndex);
}

void encoderEntryPressed(void) {
  spkrClick();
  // Handle encoder entry press
  #ifndef ENCODER_ENABLED
    drawEnabledControls(false);
    numericKeypad.init(30, 10, 260, 220, TFT_WINDOWGREY); // Initialize numeric keypad position and size
    numericKeypad.setEntryValue((float)settings.config_int[0]);
    settings.config_int[0] = rint(numericKeypad.entry(F("Enter value"), 0, true));
    encoderEntry.setValue(settings.config_int[0]); // Update the encoder entry field with the new value
    drawEnabledControls(true);   // redraw inactive controls when keypad is closed to restore window
  #endif
}

// #############################################################################


void numericDisplayPressed(void) {
  spkrClick();
  // If the numeric display is touched, toggle the measurement type
  activeMeasurement = (active_measurement_e)((activeMeasurement + 1) % 2);
  enableStdControls((instrStates_e)(instrState & ~1)); // Back to init state
}

void leftWipeBtnPressed(void) {
  spkrClick();
  if (instrState <= state_meter) {
    enablePageControls(state_scopeInit);
  } else {
    enablePageControls((instrStates_e)(instrState - 3)); // Cycle through measurement states
  }
  DEBUG_PRINT("Left wipe, cycle states to ");
  DEBUG_PRINTLN(instrState - 3);
}

void rightWipeBtnPressed(void) {
  spkrClick();
  enablePageControls((instrStates_e)((instrState + 1) % 6)); // Cycle through measurement states
  DEBUG_PRINT("Right wipe, cycle states to ");
  DEBUG_PRINTLN((instrState + 1) % 6);
}

// Press action for the High Range ON/OFF switch
void switchRangeToggled(void) {
  spkrClick();
  settings.ampHiRangeOn = switchRange.getState();
  rangeChanged = true;
}

void setupBtnPressed(void) {
  spkrClick();
  enablePageControls(state_setupInit); // Switch to setup state
}

// ##############################################################################
//
//  #### ##    ## #### ########
//   ##  ###   ##  ##     ##
//   ##  ####  ##  ##     ##
//   ##  ## ## ##  ##     ##
//   ##  ##  ####  ##     ##
//   ##  ##   ###  ##     ##
//  #### ##    ## ####    ##
//
// ##############################################################################


// Initialize all GUI elements like buttons, sliders, checkboxes etc.
// All objects are initialized on their default position on the screen, but not drawn yet.
// By default, a GUIObject is enabled, visible and active, but not drawn on initialization.
// To draw the object, call the draw(), setActive() or setState() method after setting its properties.
// Controls are set "enabled" later depending on open page or tab.
void initControls() {
  DEBUG_PRINTLN("Init Controls");

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // SETUP PAGE
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  setupTabs.init(0, 200, 320, 40, 3, TFT_WHITE, TFT_WINDOWGREY, TFT_YELLOW, 4);
  setupTabs.setPressAction(setupTabPressed); // just beep, optional
  setupTabs.setTabWidth(85); // override default tab width, optional
  setupTabs.setLabelArray(&tabLabels);

  saveBtn.init(220, 15, 90, 28, TFT_WHITE, TFT_BTNGREY, TFT_RED, 2, FF21);
  saveBtn.setLabel("SAVE");
  saveBtn.setPressAction(saveBtnPressed);

  exitBtn.init(220, 50, 90, 28, TFT_WHITE, TFT_BTNGREY, TFT_MAGENTA, 2, FF21);
  exitBtn.setLabel("EXIT");
  exitBtn.setPressAction(exitBtnPressed);

  dirBtn.init(240, 85, 70, 28, TFT_WHITE, TFT_BTNGREY, TFT_CYAN, 2, FF21);
  dirBtn.setLabel("DIR");
  dirBtn.setPressAction(dirBtnPressed);

  touchCalBtn.init(240, 120, 70, 28, TFT_WHITE, TFT_BTNGREY, TFT_GREEN, 2, FF21);
  touchCalBtn.setLabel("TCAL");
  touchCalBtn.setPressAction(touchCalBtnPressed); // Set touch calibration action

  offsetBtn.init(240, 155, 70, 28, TFT_WHITE, TFT_BTNGREY, TFT_RED, 2, FF21);
  offsetBtn.setLabel("OFFS");
  offsetBtn.setPressAction(offsetBtnPressed); // Set offset menu button action

  enaBeepCheckbox.init(10, 20, 30, TFT_WHITE, TFT_WINDOWGREY, TFT_YELLOW, 2, 4);
  enaBeepCheckbox.setState(settings.spkrBeep, true);
  enaBeepCheckbox.setPressAction(enaBeepPressed);
  enaBeepCheckbox.setLabel("Enable Beep");

  radioButtons.init(13, 70, 24, 3, TFT_WHITE, TFT_WINDOWGREY, TFT_CYAN, 2, 2);
  radioButtons.setPressAction(radioBtnPressed);
  radioButtons.setLabelArray(&radioLabels);
  radioButtons.setSelectedItem(settings.config_int[2]);

  encoderEntry.init(140, 120, 60, 28, TFT_WHITE, TFT_BLACK, TFT_YELLOW, 2, FF21);
  encoderEntry.setPressAction(encoderEntryPressed);
  encoderEntry.setLabel("Encoder Entry");
  encoderEntry.setValue(settings.config_int[0]); // Init the encoder entry field with settings value

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  wifiEnaCheckbox.init(10, 20, 20, TFT_WHITE, TFT_WINDOWGREY, TFT_WHITE, 2, 2);
  wifiEnaCheckbox.setPressAction(wifiEnaCheckboxPressed);
  wifiEnaCheckbox.setLabel("WiFi Enable");

  wifiAPCheckbox.init(10, 50, 20, TFT_WHITE, TFT_WINDOWGREY, TFT_WHITE, 2, 2);
  wifiAPCheckbox.setPressAction(wifiAPCheckboxPressed);
  wifiAPCheckbox.setLabel("WiFi AP");

  scanWifiBtn.init(210, 100, 100, 28, TFT_WHITE, TFT_BTNGREY, TFT_MAGENTA, 2, FF21);
  scanWifiBtn.setLabel("Scan WiFi");
  scanWifiBtn.setPressAction(scanWifiBtnPressed); // Set scan WiFi action

  startWPSBtn.init(210, 135, 100, 28, TFT_WHITE, TFT_BTNGREY, TFT_PURPLE, 2, FF21);
  startWPSBtn.setLabel("WPS");
  startWPSBtn.setPressAction(startWPSBtnPressed); // Set start WPS action
  #ifdef WIFI_ENABLED
    wifiEnaCheckbox.setState(wifi_connected(), false);
    wifiEnaCheckbox.setActive(true);
    wifiAPCheckbox.setActive(true);
    wifiAPCheckbox.setState(settings.wifiAPenabled, false);
  #else
    wifiEnaCheckbox.setActive(false);
    wifiEnaCheckbox.setState(false, false);
    wifiAPCheckbox.setActive(false);
    wifiAPCheckbox.setState(false, false);
  #endif
  startWPSBtn.setEnabled(true);
  scanWifiBtn.setEnabled(true);

  analogClock.init(65, 70, 120, TFT_WHITE, TFT_ORANGE, TFT_BLACK, TFT_BLUE, TFT_DIALOGGREY);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  optionCheckboxGroup.init(10, 20, 20, 5, TFT_WHITE, TFT_WINDOWGREY, TFT_WHITE, 2, 2);
  optionCheckboxGroup.setPressAction(optionsPressed); // Default action for checkbox group
  optionCheckboxGroup.setLabelArray(&checkLabels);
  for (int idx = 0; idx < 5; idx++) {
    optionCheckboxGroup.setItemState(idx, settings.config_bool[idx]);
  }

  slider1.init(10, 160, 300, 30, TFT_WHITE, TFT_CYAN, TFT_RED, 2, 2);
  slider1.setLevel(settings.config_float[0]);
  slider1.setPressAction(slider1Pressed);
  // slider1.setLabel("Slider 1");
  slider2.init(170, 10, 30, 140, TFT_WHITE, TFT_YELLOW, TFT_RED, 2, 2);
  slider2.setLevel(settings.config_float[1]);
  slider2.setPressAction(slider2Pressed);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // MAIN PAGE
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  setupBtn.initCenter(160, 218, 30, BUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, 2, FF21);
  setupBtn.setPressAction(setupBtnPressed); // Set setup button action
  setupBtn.setLabel("S");

  #ifndef ENCODER_ENABLED
    leftWipeBtn.init(0, 0, 50, MAINWINDOW_H); // always invisible
    leftWipeBtn.setPressAction(leftWipeBtnPressed);
    rightWipeBtn.init(MAINWINDOW_W - 50, 0, 50, MAINWINDOW_H); // always invisible
    rightWipeBtn.setPressAction(rightWipeBtnPressed);
  #endif

  switchRange.initCenter(BUTTON_W/2 + 3, 218, BUTTON_W, BUTTON_H, TFT_WHITE, TFT_GREEN, TFT_WHITE, 2, 2);
  switchRange.setLabelDatum(0, -5, TC_DATUM);
  switchRange.setLabel("Hi Range\0");
  switchRange.setPressAction(switchRangeToggled);
  switchRange.setState(settings.ampHiRangeOn); // set state and redraw

  statusLED.initCenter(96, 218, 24, TFT_GREEN, LED_ROUND, 2); // Position x=96, y=218, size=24, color=TFT_GREEN, shape=LED_ROUND
  statusLED.setLabel("WIFI\0");

  ovldLED.initCenter(126, 218, 24, TFT_RED, LED_ROUND, 2); // Position x=126, y=218, size=24, color=TFT_RED, shape=LED_SQUARE
  ovldLED.setLabel("OVL");

  #ifdef WIFI_ENABLED
    statusLED.setState(wifi_connected(), true, true); // Set status LED to On, blinking
  #else
    statusLED.setState(false, false, true); // Set status LED to Off, not blinking
  #endif
  ovldLED.setState(false, true, true); // Set overload LED to Off, not blinking

  numericDisplay.init(NUM_POS_X, NUM_POS_Y, TFT_DARKGREEN, true);
  numericDisplay.setRangeIdxColor(settings.voltRangeIdx, TFT_BLUE); // Set the range index to initial value
  numericDisplay.setPressAction(numericDisplayPressed); // Set numeric display action

  initControlMasks(); // Ensure all controls have their masks set
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


// Handle the GUI, check for button presses and call the appropriate actions
// This function must be called regularly in main loop to update the GUI
void handleGUI() {
  int idx;
  // Check for touch input, gets the touch coordinates and sets pressed to true if a valid touch is detected
  if (touchProvider.checkTouch()) {
    for (idx = 0; idx < guiObjectsCount; idx++) {
      guiObjects[idx]->checkPressed(true); // Check if any object was pressed
    }
  }
  // Update objects that need frequent update like blinking LEDs
  for (idx = 0; idx < guiUpdateObjectsCount; idx++) {
    guiUpdateObjects[idx]->update(); // Update all objects that need to be updated
  }

  if (barGraphAmps.checkPressed()) {
    markerAmps = barGraphAmps.getLevelMarker();
  }
  if (barGraphVolts.checkPressed()) {
    markerVolts = barGraphVolts.getLevelMarker();
  }
  if (barGraphVert.checkPressed()) {
    markerAmps = barGraphVert.getLevelMarker();
  }
  #ifdef ENCODER_ENABLED
    if (instrState != state_setup) {
      int enc_delta = touchProvider.getEncDelta();
      if (enc_delta) {
        // Handle encoder input
        spkrClick();
        if (enc_delta < 0) {
          if (instrState <= state_meter) {
            enablePageControls(state_scopeInit);
          } else {
            enablePageControls((instrStates_e)(instrState - 3)); // Cycle through measurement states
          }
        } else {
          enablePageControls((instrStates_e)((instrState + 1) % 6)); // Cycle through measurement states
        }
      }
    }
  #endif
}

#endif