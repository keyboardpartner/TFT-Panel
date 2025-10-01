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

// Switch Class for TFT_eSPI display,
// providing a simple interface for creating and managing switches on the display.

/***************************************************************************************
// The following Checkbox class has been ported over from the Adafruit_tft library.
// A slightly different implementation in this Checkbox library allows:
//
// 1. The Checkbox labels to be in any font
// 2. Allow longer labels
// 3. Allow text datum to be set
// 4. Allow invert state to be read via getState(), true = inverted
// 5. Define pressed and released callbacks per Checkbox instances
//
// The library uses functions present in TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI

// Addittion by cm 7/25:
// TouchProvider class for handling touch events as a common provider for all s
// This allows the Checkbox to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly.
// Usage: Instantiate TouchProvider once in main.cpp and pass it to  constructors
// Example:
// TouchProvider touchProvider = TouchProvider(&tft);
// Then, pass it to  constructor:
// Checkbox testCheckbox1 = Checkbox(&tft, &touchProvider);
// Button   testButton1   = Button(&tft, &touchProvider);
****************************************************************************************/

#ifndef _CheckboxH_
#define _CheckboxH_

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all s
#include "guiObject.h" // Common GUI object for all s
#include "Free_Fonts.h" // Include large fonts


// Simple checkbox that toggles state on touch, inherits from GUIObject
class Checkbox : public GUIObject {

  public:
  Checkbox(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject() {_tft = tft; _touchProvider = touchProvider; }

    // "Classic" init() uses centre & size
  void     initCenter(int16_t x, int16_t y, uint16_t size, uint16_t bordercolor, uint16_t bgcolor, uint16_t textcolor, uint16_t borderwidth, uint8_t textfont = 2);
  // Overloaded function to use a FreeFont instead of a text font
  void     initCenter(int16_t x, int16_t y, uint16_t size, uint16_t bordercolor, uint16_t bgcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);

  // New/alt init() uses upper-left corner & size
  void     init(int16_t x1, int16_t y1, uint16_t size, uint16_t bordercolor, uint16_t bgcolor, uint16_t textcolor, uint16_t borderwidth, uint8_t textfont = 2);
  // Overloaded function to use a FreeFont instead of a text font
  void     init(int16_t x1, int16_t y1, uint16_t size, uint16_t bordercolor, uint16_t bgcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);

  void     draw(bool checked = false);

  // Active state, object responds to user input when active; may be visible or not visible, though
  // set switch active (or inactive) and redraw (or not), overload with redraw
  void setActive(bool active, bool redraw = false) {
    _active = active;
    if (redraw)
      draw(_checked);
  }

  // override with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      draw(_checked);
  }

  // set switch active (or inactive) and redraw (or not), overload with redraw
  void setState(bool state, bool redraw = false) {
    _checked = state;
    if (redraw)
      draw(_checked);
  }

  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(_checked); // draw object
  }

  bool checkPressed(bool wait_released) override {
    if (!_enabled) return false; // Do not check if not alive
		bool pressed = false;
    _laststate = _checked;
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      _currstate = !_checked; // Toggle state
      if (_currstate != _laststate) {
        setState(_currstate, true); // Redraw Checkbox
        _pressAction(); // Call the toggle action callback
				pressed = true;
      }
			delay(20); // delay for debounce
      // If the Checkbox is pressed, wait until the touch is released
      if (wait_released) {
        _touchProvider->waitReleased();
      }
    } else {
      _currstate = _laststate; // Reset current state to last known state
    }
		return pressed;
  }

 private:
  TFT_eSPI *_tft;
	TouchProvider *_touchProvider;
};

#endif
