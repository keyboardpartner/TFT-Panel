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

// Radio checkbox group Class for TFT_eSPI display,
// providing a simple interface for creating and managing radio checkbox groups on the display.

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
// checkboxWidget   testcheckbox1   = checkboxWidget(&tft, &touchProvider);
****************************************************************************************/

#ifndef _CHECKBOXGROUPWIDGET_H_
#define _CHECKBOXGROUPWIDGET_H_

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts


// Set of multiple (up to 10) checkboxes with same properties, inherits from GUIObject
// Gets labels from the label array provided with setLabelArray()
class CheckboxGroup : public GUIObject {

  public:
	// Due to some unknown reasons, initialization of GUIObject may fail when tft and touchprovider are supplied with constructor
  CheckboxGroup(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  void init(int16_t x, int16_t y, uint16_t size, uint16_t count, uint16_t bordercolor, uint16_t bgcolor, uint16_t checkedcolor, uint16_t borderwidth, uint8_t textfont = 2) {
		_x           = x;
		_y           = y;
		_xd      = 0;
		_yd      = 0;
		_size         = size;
		_borderwidth  = borderwidth;
		_bordercolor 	= bordercolor;
		_bgcolor      = bgcolor;
  	_checkedcolor = _tft->alphaBlend(200, checkedcolor, TFT_BLACK);
		_textcolor    = bordercolor;
		_textfont     = textfont; // use TextFont
		_textfreefont = NULL;
		_selectedItem = 0;
		_label_arr = NULL;
		_count = count;
		_w = size * 4; // Default touch width is size * 4
		_textdatum = ML_DATUM; // Text adjust left, to the right of checkbox
		_enabled = true;    // alive by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

  // Overloaded function to use a FreeFont instead of a text font
  void init(int16_t x, int16_t y, uint16_t size, uint16_t count, uint16_t bordercolor, uint16_t bgcolor, uint16_t checkedcolor, uint16_t borderwidth, const GFXfont *textfreefont) {
		_x           = x;
		_y           = y;
		_xd      = 0;
		_yd      = 0;
		_size         = size;
		_borderwidth  = borderwidth;
		_bordercolor 	= bordercolor;
		_bgcolor      = bgcolor;
  	_checkedcolor = _tft->alphaBlend(200, checkedcolor, TFT_BLACK);
		_textcolor    = bordercolor;
		_textfont     = 255;// Default to 255, meaning use the FreeFont
		_textfreefont = textfreefont;
		_selectedItem = 0;
		_label_arr = NULL;
		_count = count;
		_w = size * 4; // Default touch width is size * 4
		_textdatum = CL_DATUM; // Center datum for text
		_enabled = true;    // alive by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

  // Adjust text datum and x, y deltas
  void setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = CL_DATUM) {
		_xd        = x_delta;
		_yd        = y_delta;
		_textdatum = datum;
	}

  void redrawAll() {
    for (uint16_t i = 0; i < _count; i++) {
      draw(i);
    }
  }

   // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    redrawAll(); // draw objects
  }

  // set Radiocheckboxs active (or inactive) and redraw (or not)
	// Overload with redraw
	void setActive(bool active, bool redraw = false) {
		_active = active;
		if (redraw) redrawAll();
	}


	// Alive state overrides Active and Visible states. Object will be ignored and not drawn if not alive
  // overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw) redrawAll();
  }

	void toggleItem(uint16_t index) {  // set radio checkbox or tab number
		if (index >= 10) return; // Prevent out-of-bounds access
		_selectedItem = index;
		_checkedArray[index] = !_checkedArray[index];
	}

 bool getItemState(uint16_t index) {
		if (index >= 10) return false; // Prevent out-of-bounds access
		return _checkedArray[index];
	}

 // set checkbox # state (on/off)
  void setItemState(uint16_t index, bool isChecked) {
		if (index >= 10) return; // Prevent out-of-bounds access
		_selectedItem = index;
		_checkedArray[index] = isChecked;
	}

  // set Radiocheckboxs state (on/off) and redraw (or not)
	// Overload with redraw
  void setItemState(uint16_t selected_item, bool isChecked, bool redraw) {
		if (selected_item >= 10) return; // Prevent out-of-bounds access
		_selectedItem = selected_item;
		_checkedArray[selected_item] = isChecked;
		if (redraw) redrawAll();
	}

	uint16_t getCount(void) {return _count;}            // Get the number of radio checkboxs in the group

	// Set tab labels from array of labelArray_t
  void setLabelArray(labelArray_t *labels) {
		_label_arr = labels;
	}

	// Set a single tab label
	void setLabel(uint16_t selected_item, const char *label) {
		if (selected_item >= 10) return; // Prevent out-of-bounds access
		strncpy(_label_arr->labelstr[selected_item], label, sizeof(_label_arr->labelstr[selected_item]) - 1);
		_label_arr->labelstr[selected_item][sizeof(_label_arr->labelstr[selected_item]) - 1] = '\0'; // Ensure null-termination
	}

// ############################################################################

  void draw(uint16_t item) {
	  if (!_visible || !_enabled) return; // Do not draw if not visible or not alive
		uint16_t bw2 = _borderwidth * 2;
		uint16_t my_textcolor = _textcolor;
		uint16_t my_checkcolor = _checkedcolor;
		uint16_t my_bordercolor = _bordercolor;
		uint16_t my_bgcolor = _bgcolor;
		if (!_active) {
      my_textcolor = TFT_GREY; // Use dark grey for inactive RadioButtons
      my_checkcolor = TFT_GREY; // Use dark grey for inactive RadioButtons
      my_bordercolor = TFT_DARKGREY; // Use dark grey for inactive RadioButtons
      my_bgcolor = TFT_BLACK;
		}
		uint16_t text_x;
		switch (_textdatum) {
			case ML_DATUM:
				text_x = _x + _xd + _size + _size/4 + 2; // Default x delta is size referenced to x0
				break;
			case MR_DATUM:
				text_x = _x + _xd - _size/4 - 2; // Default x delta is -size/4 + 4 referenced to x0
				break;
			default:
			  text_x = _x + _xd; // Default x delta is 0 referenced to x0
				break; // No adjustment needed for other text datum types
		}
		uint8_t temp_datum = _tft->getTextDatum();
		uint16_t temp_padding = _tft->getTextPadding();
		uint16_t y_inc  = _size + _size/4 + 2; // Increment y delta for the next checkbox
		uint16_t size_2 = _size / 2; // size_2 for the circle

		_tft->setTextDatum(_textdatum);
		_tft->setTextColor(my_textcolor, my_bgcolor);

		for (uint16_t j = 0; j < _count; j++) {
			uint16_t y_new = _y + y_inc * j;
			// Draw the checkmark if the checkbox is checked

			_tft->fillRect(_x, y_new, _size, _size, my_bgcolor);
			for (int i = 0; i < _borderwidth; i++) {
				_tft->drawRect(_x + i, y_new + i, _size - i*2, _size - i*2, my_bordercolor);
			}

			// Draw the checkmark if the Checkbox is checked
			if (getItemState(j)) {
				_tft->fillRect(_x + _borderwidth + 2, y_new + _borderwidth + 2, _size - bw2 - 4, _size - bw2 - 4, my_checkcolor);
				_tft->drawWideLine(_x + _borderwidth + 2, y_new + _borderwidth + 2, _x + _size - bw2 - 1, y_new + _size - bw2 - 1, _borderwidth, my_bordercolor, my_bgcolor);
				_tft->drawWideLine(_x + _borderwidth + 2, y_new + _size - bw2 - 1, _x + _size - bw2 - 1, y_new + _borderwidth + 2, _borderwidth, my_bordercolor, my_bgcolor);
			} else {
				// Draw the box if not checked
				_tft->fillRect(_x + _borderwidth + 2, y_new + _borderwidth + 2, _size - bw2 - 4, _size - bw2 - 4, _tft->alphaBlend(150, my_checkcolor, TFT_BLACK));
			}

			if (_textfont == 255) {
				_tft->setFreeFont(_textfreefont); // Use FreeFont if set
			} else {
				_tft->setTextFont(_textfont);
			}
			// Adjust text x position based on the text datum
			_tft->setTextPadding(0);
			if (_label_arr != NULL && _label_arr->labelstr[j][0] != '\0') {
				_tft->drawString(_label_arr->labelstr[j], text_x, y_new + _yd + size_2);
			}

		}
		_tft->setTextDatum(temp_datum);
		_tft->setTextPadding(temp_padding);
	}


  bool contains(int16_t x, int16_t y, uint16_t btn_number) {
		// Extends touch area to approx. text width - TODO!
		uint16_t y_inc  = _size + _size/4 + 2; // Increment y delta for the next checkbox
		uint16_t y_start = _y + y_inc * btn_number; // Calculate the y position based on checkbox number
		uint16_t y_end = y_start + y_inc; // Calculate the end y position

		if (btn_number >= _count) {
			return false; // Invalid checkbox number
		}
    return ((x >= _x) && (x < (_x + _w)) &&
            (y >= y_start) && (y <= y_end));
  }

// ############################################################################

  bool checkPressed(bool wait_released) override {
    if (!_enabled) return false; // Do not check if not alive
		bool pressed = false;
    if (_active && _touchProvider->pressed) {
			for (uint16_t j = 0; j < _count; j++) {
				if (contains(_touchProvider->tx, _touchProvider->ty, j)) {
      		_currstate = !getItemState(j); // Toggle state
					_selectedItem = j;
					pressed = true;
					setItemState(_selectedItem, _currstate);
					draw(_selectedItem); // Redraw the Radiocheckboxs with the new checked state
					_pressAction();
					// If the Radiocheckbox is pressed, wait until the touch is released
					if (wait_released) {
						_touchProvider->waitReleased();
					}
					break;
				}
			}
		}
		return pressed;
  }

// ############################################################################

  private:
  bool _checkedArray[10];
  labelArray_t *_label_arr; // checkbox text is 19 chars maximum per line
	uint16_t _count; // Number of checkbox checked, Number of radio checkboxs in the group
};

#endif // _CHECKBOXGROUPWIDGET_H_
