/***************************************************************************************
//
// The library uses functions present in TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI

// Addittion by cm 7/25:
// TouchProvider class for handling touch events as a common provider for all widgets
// This allows the CheckboxWidget to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly.
// Usage: Instantiate TouchProvider once in main.cpp and pass it to widget constructors
// Example:
// TouchProvider touchProvider = TouchProvider(&tft);
// Then, pass it to widget constructor:
// CheckboxWidget testCheckbox1 = CheckboxWidget(&tft, &touchProvider);
// ButtonWidget   testButton1   = ButtonWidget(&tft, &touchProvider);
//
// Added visibility and enabled control:
// A switch can be made invisible or "dead" (not responding to touches, not redrawn)
// as needed.
//
***************************************************************************************/

#ifndef _SliderHorWidgetH_
#define _SliderHorWidgetH_


#include "guiObject.h" // Common touch provider for all widgets

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

class SliderHorWidget : public GUIObject {

  public:
  //SwitchWidget(TFT_eSPI *tft, TouchProvider *touchProvider) {}
	SliderHorWidget(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  // "Classic" init() uses centre & size
  void initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, uint8_t textfont = 2);
  // Overloaded function to use a FreeFont instead of a text font
  void initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);

  // New/alt init() uses upper-left corner & size
  void init(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, uint8_t textfont = 2);
  // Overloaded function to use a FreeFont instead of a text font
  void init(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont);

  void draw(float level, bool draw_border = true); // Draw the slider with the specified level, draw_border is optional

  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(_level); // draw object
  }

  // Active state, object responds to user input when active; may be visible or not visible, though
  // set switch active (or inactive) and redraw (or not), overload with redraw
  void setActive(bool active, bool redraw = false) {
    _active = active;
    if (redraw)
      draw(_level);
  }

	// Enabled state overrides Active and Visible states.
	// Object will be ignored and not drawn if not enabled
  // Overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      draw(_level);
  }

  // Set switch active (or inactive) and redraw (or not), overload with redraw
  void setLevel(float level, bool redraw = false) {
    _level = level;
    _level_integrator = level;
    if (redraw)
      draw(_level);
  }

  float getLevel() const { return _level; } // returns current level as float

  // Check if the slider is pressed
  // If the slider is pressed, it will enter a modal state,
  // updating the slider level, calling PressAction and redraw thumb control
  bool checkPressed(bool wait_released) override {
    if (!_enabled) return false; // Do not check if not enabled
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      // Calculate the relative position of the touch within the slider
      bool first_touch = true;
      while (_touchProvider->checkTouch()) {
        _level = (float)(_touchProvider->tx - _thumb_start) / (float)(_thumb_travel);
        if (_level < 0) _level = 0; // Ensure level is not negative
        if (_level > 1.0) _level = 1.0; // Ensure level is not greater than maximum
        if (first_touch) {
          _level_integrator = _level;
          first_touch = false;
        }
        _level_integrator += (_level - _level_integrator) * 0.25; // Simple low-pass filter
        draw(_level_integrator, false);
        _pressAction(); // Call the pressed action callback continuously
        delay(10);
        if (!wait_released) break; // Exit loop if no wait for release is requested
      }
      // If the switch is pressed, wait until the touch is released
      if (wait_released) {
        _touchProvider->waitReleased();
      }
      _level = _level_integrator;
      // _pressAction(); // Call the pressed action callback AFTER release
      return true;
    }
    return false; // Return the current level of the slider
  }

  private:
	float _level, _level_integrator;
  int16_t _thumb_travel, _thumb_delta, _thumb_start, _thumb_end, _thumb_height, _thumb_width, _thumb_y;

  void resetCommon() {
    _currstate = false;
    _laststate = false;
    _isOn      = false;
    _checked   = false;
    _textdatum = TC_DATUM;  // Center datum for text
    _label[0]  = '\0';      // Initialize label to empty string
    _enabled     = true;      // enabled by default
    _active    = true;      // active by default
    _visible   = true;      // visible by default
    _fillcolor = _tft->alphaBlend(128, _bordercolor, TFT_BLACK);
    // slider/switch specific
	  _level    = 0.5;  // Default level
	  _level_integrator = 0.5;  // Default level
    _thumb_y      = _y + _borderwidth + 2;
    _thumb_width  = _h - (_borderwidth * 2); // Calculate thumb height
    _thumb_height = _thumb_width - 4; // Calculate thumb width
    _thumb_travel = _w - _thumb_width - (_borderwidth * 2) -4; // Calculate travel distance for the slider thumb
    _thumb_delta  = 0;  // Initialize thumb position
    _thumb_start  = _x + _thumb_width/2 + _borderwidth + 2; // Initialize thumb start position (center of thumb)
    _thumb_end    = _x + _thumb_travel + _thumb_width/2; // Initialize thumb end position
  }

};

#endif
