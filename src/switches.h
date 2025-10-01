/***************************************************************************************
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
// Checkboxs testCheckbox1 = Checkboxs(&tft, &touchProvider);
// Buttons   testButton1   = Buttons(&tft, &touchProvider);
//
// Added visibility and "enabled" control:
// A switch can be made invisible or "dead" (not responding to touches, not redrawn)
// as needed.
//
***************************************************************************************/

#ifndef _SwitchsH_
#define _SwitchsH_

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts


// Simple slide on/off switch that toggles state on touch, inherits from GUIObject
class SlideSwitch : public GUIObject {

  public:

  // Constructor, creates a horizontal slider switch
	SlideSwitch(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) {}

  // "Classic" init() uses centre & size
  void     initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, uint8_t textfont = 2);
  // Overloaded function to use a FreeFont instead of a text font
  void     initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);

  // New/alt init() uses upper-left corner & size
  void     init(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, uint8_t textfont = 2);
  // Overloaded function to use a FreeFont instead of a text font
  void     init(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);

  void     draw(bool switch_on, bool draw_border = true); // Draw the switch with the specified state and border option

  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(_isOn); // draw object
  }

  // Active state, object responds to user input when active; may be visible or not visible, though
  // set switch active (or inactive) and redraw (or not), overload with redraw
  void setActive(bool active, bool redraw = false) {
    _active = active;
    if (redraw)
      draw(_isOn);
  }

	// Enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
	// overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      draw(_isOn);
  }

  // set switch on or off and redraw (or not), overload with redraw
  void setState(bool state, bool redraw = false) {
    _isOn = state;
    _checked = state;  // Update also checked state
   if (redraw)
      draw(state);
  }

  // Check if the switch is pressed, if so, toggle its state
  bool checkPressed(bool wait_released) override {
    if (!_enabled) return false; // Do not check if not enabled
    _laststate = _isOn;
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      _currstate = !_isOn; // Toggle state
      if (_currstate != _laststate) {
        setState(_currstate, true); // Redraw switch
        _pressAction(); // Call the pressed action callback
      }
      // If the switch is pressed, wait until the touch is released
			delay(20); // delay for debounce
      if (wait_released) {
        _touchProvider->waitReleased();
      }
    } else {
      _currstate = _laststate; // Reset current state to last known state
    }
    _isOn = _currstate; // Update the switch state
    _checked = _currstate;
    return _currstate;  // Return the current state of the switch
  }

  private:
  int16_t _thumb_travel, _thumb_delta, _thumb_start, _thumb_end, _thumb_height, _thumb_width, _thumb_y;
  void resetCommon() {
    _currstate = false;
    _laststate = false;
    _isOn      = false;
    _checked   = false;
    _textdatum = TC_DATUM;  // Center datum for text
    _label[0]  = '\0';      // Initialize label to empty string
    _enabled     = true;    // enabled by default
    _active    = true;      // active by default
    _visible   = true;      // visible by default
    _fillcolor = _tft->alphaBlend(128, _bordercolor, TFT_BLACK);
    // slider/switch specific
    _thumb_y      = _y + _borderwidth + 2;
    _thumb_width  = _h - (_borderwidth * 2); // Calculate thumb height
    _thumb_height = _thumb_width - 4; // Calculate thumb width
    _thumb_travel = _w - _thumb_width - (_borderwidth * 2) -4; // Calculate travel distance for the slider thumb
    _thumb_delta  = 0;  // Initialize thumb position
    _thumb_start  = _x + _thumb_width/2 + _borderwidth + 2; // Initialize thumb start position (center of thumb)
    _thumb_end    = _x + _thumb_travel; // Initialize thumb end position
  }

};

#endif
