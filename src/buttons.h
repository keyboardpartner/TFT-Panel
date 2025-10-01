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

// Button Class for TFT_eSPI display,
// providing a simple interface for creating and managing buttons on the display.

/***************************************************************************************
// The following button class has been ported over from the Adafruit_tft library.
// A slightly different implementation in this Buttons library allows:
//
// 1. The button labels to be in any font
// 2. Allow longer labels
// 3. Allow text datum to be set
// 4. Allow invert state to be read via getState(), true = inverted
//
// The library uses functions present in TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI

// Addittion by cm 7/25:
// TouchProvider class for handling touch events as a common provider for all widgets
// This allows the Checkboxs to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly.
// Usage: Instantiate TouchProvider once in main.cpp and pass it to widget constructors
// Example:
// TouchProvider touchProvider = TouchProvider(&tft);
// Then, pass it to widget constructor:
// Checkboxs testCheckbox = Checkboxs(&tft, &touchProvider);
// Buttons   testButton1   = Buttons(&tft, &touchProvider);
// Added visibility and "alive" control:
// A button can be made invisible or "dead" (not responding to touches, not redrawn)
// as needed.
***************************************************************************************/
#ifndef _ButtonsH_
#define _ButtonsH_

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts


// Simple button widget, inherits from GUIObject
// Displays a colored rectangular button with rounded edges
class PushButton : public GUIObject {

 public:
  PushButton(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  // "Classic" init() uses centre & size, with overloaded function to use a FreeFont
  void     initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h); // for invisible buttons
  void     initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t fill, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);
  void     initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t fill, uint16_t textcolor, uint16_t borderwidth, uint8_t textfont = 2);

  // New/alt init() uses upper-left corner & size, with overloaded function to use a FreeFont
  void     init(int16_t x, int16_t y, uint16_t w, uint16_t h); // for invisible buttons
  void     init(int16_t x, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t fill, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);
  void     init(int16_t x, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t fill, uint16_t textcolor, uint16_t borderwidth, u_int8_t textfont = 2);

  void     draw(bool inverted = false);

  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(false); // draw object
  }

	// override with redraw
  void setActive(bool active, bool redraw = false) {
    _active = active;
    if (_visible && redraw)
      draw(_inverted);
  }

	// Enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      draw(false);
  }

  bool checkPressed(bool wait_released) override {
    // Simple proc to check if the button is pressed, waits for release if wait_released is true
    // Result will be set true if there is a valid touch on the screen
    _currstate = false;
    if (!_enabled) return false; // Do not check if not alive
    bool was_just_pressed = false;
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      _currstate = true;
      if (_laststate != _currstate) {
        was_just_pressed = true; // Button was pressed
        draw(true);
        _pressAction(); // Call the press action callback if defined
        // If the switch is pressed, wait until the touch is released
			  delay(20); // delay for debounce
        if (wait_released) {
          _touchProvider->waitReleased();
          draw(false);
          _currstate = false; // Reset current state to false after release
        }
      }
    }
    _laststate = _currstate;
    _checked = was_just_pressed;
    return was_just_pressed; // Return the current state of the button
  }

 private:
  bool  _inverted; // Button states
};

#endif
