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

// Base Class for graphical user interface objects

/***************************************************************************************
// by cm 7/25:
// This class is a base for graphical user interface objects.
//
// The action callback can do anything and is called when the object is pressed.
// Avoid long-running tasks in the callback to keep the UI responsive.
// Refrain from initialisation of own data structure (or callback itself)!
//
// The inherited object should implement its own draw() and update() methods.
//
// Due to some unknown reasons, initialization of GUIObject may fail
// when *tft and *touchprovider references are passed with constructor.
// This was observed with "radioButtonsWidget.h".
// Therefore, we do not store the references here.
// In this case, also overloads are not passed to the class. Compiler glitch? No idea.
//
****************************************************************************************/

#ifndef _GUIOBJECTH_
#define _GUIOBJECTH_

#define MAX_LABELS 10
#define MAX_LABEL_LEN 20 // Maximum label length for buttons, tabs and radio buttons

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets


// The action callback can do anything and is called when the object is pressed.
// Avoid long-running tasks in the callback to keep the UI responsive.
// Refrain from initialisation of own data structure (or callback itself)!
typedef void (*actionCallback)(void);
static void dummyAction(void) { }; // In case user callback is not defined!

// Array of up to 10 labels, 20 chars long each
struct labelArray_t {
	char labelstr[MAX_LABELS][MAX_LABEL_LEN]; // 10 labels for radio buttons and tabs
};

#define TFT_GREY  0x5AEB //0x5AEB 16-bit colour, RGB565 format

// Base class for graphical user interface objects, provides data structures and common methods
class GUIObject  {

  public:

  GUIObject(TFT_eSPI *tft = NULL, TouchProvider *touchProvider = NULL) {
    _tft = tft;
    _touchProvider = touchProvider;
		_pressAction = dummyAction;
		_x       = 0;
		_y       = 0;
		_xd      = 0;
		_yd      = 0;
		_h       = 10;
		_w       = 10;
		_size		 = 10;
		_borderwidth = 2;
  	_textfreefont = NULL;
		_textfont = 2;
		_textdatum = MC_DATUM; // Mid Center datum for text
		_label[0] = '\0'; // Initialize label to empty string
		_enabled = true;    // enabled by default
    _active = true;	// inactive by default
		_visible = true;  // visible by default
		_checked = false; // unchecked by default
		_isOn = false;    // unchecked by default
    _bordercolor = TFT_WHITE;
    _fillcolor = TFT_DARKGREY;
    _textcolor = TFT_WHITE;
    _checkedcolor = TFT_GREEN;
		_thumbcolor = TFT_BLUE; // Default thumb color for switch
		_bgcolor = TFT_BLACK;   // Default background color
    _tag = 0; // Default tag value
  }

  void setColors(uint16_t border, uint16_t fill, uint16_t text, uint16_t checked, uint16_t thumb, uint16_t bgcolor) {
    _bordercolor = border;
    _fillcolor = fill;
    _textcolor = text;
    _checkedcolor = checked;
    _thumbcolor = thumb;
    _bgcolor = bgcolor;
  }

	// override without thumb color
  void setColors(uint16_t border, uint16_t fill, uint16_t text, uint16_t checked, uint16_t bgcolor) {
    _bordercolor = border;
    _fillcolor = fill;
    _textcolor = text;
    _checkedcolor = checked;
    _bgcolor = bgcolor;
  }

	// override without thumb and background colors
  void setColors(uint16_t border, uint16_t fill, uint16_t text, uint16_t checked) {
    _bordercolor = border;
    _fillcolor = fill;
    _textcolor = text;
    _checkedcolor = checked;
  }

  void setTag(int16_t tag) { _tag = tag; } // Set tag for object
  int16_t getTag() const { return _tag; }  // Get tag for object

  void setMask(uint32_t mask) { _mask = mask; } // Set mask for object
  uint32_t getMask() const { return _mask; }    // Get mask for object

  // set color for LEDs, switches and sliders
	void setColor(uint16_t color) { _color = color; _thumbcolor = color; }
	int16_t getColor() const { return _color; }

	void setPosition(int x, int y) { _x = x; _y = y; }
	void getPosition(int &x, int &y) const { x = _x; y = _y; }
	int getX() const { return _x; }
	int getY() const { return _y; }

	void setDimensions(int16_t w, int16_t h) { _w = w; _h = h; }
	void getDimensions(int16_t &w, int16_t &h) const { w = _w; h = _h; }

	void setSize(int16_t size) { _w = size;	_h = size;	_size = size;	}
	int16_t getSize() const { return _size; }

  // On/Off or Checked state of object
	void setState(bool state) { _checked = state; _isOn = state; }
	bool getState(void) const { return _checked; }
	bool isOn(void) const { return _isOn; }
	void setChecked(bool checked) { _checked = checked; _isOn = checked; }
	bool getChecked(void) const { return _checked; }
  bool isChecked() const { return _checked; }

	// Visible state, object is only drawn if visible = true
	void setVisible(bool visible) { _visible = visible; }
	bool getVisible(void) const  { return _visible; }
	bool isVisible(void) const { return _visible; }

	void setSelectedItem(uint16_t selected_item) { _selectedItem = selected_item; } // set radio button or tab number
	uint16_t getSelectedItem(void) const {return _selectedItem;}        // Get selected item index
  bool isSelectedItem(uint16_t selected_item) const { return _selectedItem == selected_item; } // Check if item is selected

  actionCallback _pressAction = dummyAction; // Pressed action callback
  void setPressAction(actionCallback action) { _pressAction = action; }

  // "Redraw with previous state" action, will be overridden in derived classes
  virtual void redraw(bool active) {}

  // "Draw with new value or blinking" action, will be overridden in derived classes
  virtual void update() {}

  // Check if object is pressed, will be overridden in derived classes
  virtual bool checkPressed(bool wait_released) { return false; }

  // Active state, object is greyed out when not active; may be visible or not visible, though
	void setActive(bool active) { _active = active; }
  virtual void setActive(bool active, bool redraw) {}
	bool getActive(void) const { return _active; }
	bool isActive(void) const { return _active; }

	// enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
	void setEnabled(bool enabled) { _enabled = enabled; }
	bool getEnabled(void) const { return _enabled; }
	bool isEnabled(void) const { return _enabled; }

  void setLabel(const char *label) {
    if (label == NULL || strlen(label) > MAX_LABEL_LEN) {
      _label[0] = '\0';  // Clear label if NULL or too long
    } else {
      strncpy(_label, label, sizeof(_label) - 1);
      _label[sizeof(_label) - 1] = '\0';  // Ensure null termination
    }
  }

  void setTextFont(uint8_t font) {
		_textfont = font;
		_textfreefont = NULL;
	}
  // Overload for free fonts
  void setTextFont(const GFXfont *font) {
		_textfreefont = font;
		_textfont = 255; // Use free font
	}

  void setTextDatum(uint8_t text_datum) { _textdatum = text_datum; }
  // Adjust text datum and x, y deltas
  void setTextDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = CL_DATUM) {
		_xd        = x_delta;
		_yd        = y_delta;
		_textdatum = datum;
	}

  // Adjust text datum and x, y deltas
	// This sets the position of the label relative to the LED
  void setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = TC_DATUM) {
		_xd      = x_delta;
		_yd      = y_delta;
		_textdatum = datum;
	}

  // Checks if coordinates are within the object boundaries
  bool contains(int16_t x, int16_t y) const {
    return ((x >= _x) && (x <= (_x + _w)) &&
            (y >= _y) && (y <= (_y + _h)));
  }

protected:
  // These are visible only in descendants of class
  TFT_eSPI *_tft;
	TouchProvider *_touchProvider;
  const GFXfont *_textfreefont;   // FreeFont used for button text

  uint32_t _mask;   // generic mask, for grouping objects in loop
  int16_t _tag;     // Tag for object, can be used to identify the object in callbacks

  int16_t  _x, _y;              // Coordinates of top-left corner of object
  int16_t _xd, _yd; // Text x and y deltas for label positioning
  int16_t _h, _w, _size; // height and width of object
  int16_t _center_x;
  int16_t _center_y;
  int16_t _radius;

  uint8_t _textfont; // Text font used for object text
  uint8_t _textdatum; // Text datum for label positioning
  uint16_t _color, _bordercolor, _fillcolor, _textcolor, _checkedcolor, _thumbcolor, _bgcolor;
  uint16_t _borderwidth;

  uint16_t _selectedItem; // Selected item index for radio buttons and tabs
  bool _active, _visible, _enabled; // Object states
  bool _isOn, _checked;
  bool _currstate, _laststate; // Switch/LED states
  char _label[MAX_LABEL_LEN];  // Button, LED or checkbox label text

private:

};

#endif // _GUIOBJECTH_
