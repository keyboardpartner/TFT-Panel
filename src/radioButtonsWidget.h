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

// Radio button group Class for TFT_eSPI display,
// providing a simple interface for creating and managing radio button groups on the display.

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
****************************************************************************************/

#ifndef _RADIOButtonsH_
#define _RADIOButtonsH_


// Set of multiple (up to 10) checkboxes with same properties, inherits from GUIObject
// Gets labels from the label array provided with setLabelArray()
class RadioButtonsWidget : public GUIObject {

  public:
	// Due to some unknown reasons, initialization of GUIObject may fail when tft and touchprovider are supplied with constructor
  RadioButtonsWidget(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  void init(int16_t x, int16_t y, uint16_t size, uint16_t count, uint16_t bordercolor, uint16_t bgcolor, uint16_t checkedcolor, uint16_t borderwidth, uint8_t textfont = 2) {
		_x           = x;
		_y           = y;
		_xd      = 0;
		_yd      = 0;
		_size         = size;
		_borderwidth  = borderwidth;
		_bordercolor 	= bordercolor;
		_bgcolor      = bgcolor;
		_checkedcolor = checkedcolor;
		_textcolor    = bordercolor;
		_textfont     = textfont; // use TextFont
		_textfreefont = NULL;
		_selectedItem = 0;
		_last_index  = 0;
		_label_arr = NULL;
		_count = count;
		_w = size * 4; // Default touch width is size * 4
		_textdatum = CL_DATUM; // Center datum for text
		_enabled = true;    // enabled by default
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
		_checkedcolor = checkedcolor;
		_textcolor    = bordercolor;
		_textfont     = 255;// Default to 255, meaning use the FreeFont
		_textfreefont = textfreefont;
		_selectedItem = 0;
		_last_index  = 0;
		_label_arr = NULL;
		_count = count;
		_w = size * 4; // Default touch width is size * 4
		_textdatum = CL_DATUM; // Center datum for text
		_enabled = true;    // enabled by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

  void draw(uint16_t item) {
	  if (!_visible || !_enabled) return; // Do not draw if not visible or not enabled
		uint16_t bw2 = _borderwidth * 2;
		uint16_t my_textcolor = _textcolor;
		uint16_t my_checkcolor = _checkedcolor;
		uint16_t my_uncheckcolor = _tft->alphaBlend(128, _checkedcolor, TFT_BLACK);
		uint16_t my_bordercolor = _bordercolor;
		if (!_active) {
			my_textcolor = TFT_DARKGREY; // Use dark grey for inactive RadioButtons
			my_checkcolor = TFT_DARKGREY; // Use dark grey for inactive RadioButtons
      my_uncheckcolor = TFT_DARKGREY; // Use dark grey for inactive RadioButtons
			my_bordercolor = TFT_GREY; // Use dark grey for inactive RadioButtons
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
		uint16_t y_inc  = _size + _size/4 + 2; // Increment y delta for the next button
		uint16_t radius = _size / 2; // Radius for the circle

		_tft->setTextDatum(_textdatum);
		_tft->setTextColor(my_textcolor, _bgcolor);

		for (uint16_t j = 0; j < _count; j++) {
			uint16_t y_new = _y + radius + y_inc * j;
			// Draw the checkmark if the button is checked
			_tft->fillCircle(_x + radius, y_new, radius, my_bordercolor);
			_tft->fillCircle(_x + radius, y_new, radius - _borderwidth, _bgcolor);
			uint16_t circle_color = (j == item) ? my_checkcolor : my_uncheckcolor;
			_tft->fillCircle(_x + radius, y_new, radius - bw2 - 1, circle_color);

			if (_textfont == 255) {
				_tft->setFreeFont(_textfreefont); // Use FreeFont if set
			} else {
				_tft->setTextFont(_textfont);
			}
			// Adjust text x position based on the text datum
			_tft->setTextPadding(0);
			if (_label_arr != NULL && _label_arr->labelstr[j][0] != '\0') {
				_tft->drawString(_label_arr->labelstr[j], text_x, y_new + _yd);
			}
		}
		_tft->setTextDatum(temp_datum);
		_tft->setTextPadding(temp_padding);
	}

	// Alive state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
  // overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      draw(_selectedItem);
  }

  // set RadioButtons active (or inactive) and redraw (or not)
	// Overload with redraw
	void setActive(bool active, bool redraw = false) {
		_active = active;
		if (redraw) draw(_selectedItem);
	}

  // set RadioButtons state (on/off) and redraw (or not)
	// Overload with redraw
  void setSelectedItem(uint16_t selected_item, bool redraw = false) {
		_selectedItem = selected_item;
		if (redraw) draw(_selectedItem);
	}


  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(_selectedItem); // draw object
  }

  // Adjust text datum and x, y deltas
  void setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = CL_DATUM) {
		_xd        = x_delta;
		_yd        = y_delta;
		_textdatum = datum;
	}

	uint16_t getCount(void) {return _count;}            // Get the number of radio buttons in the group

	// Set tab labels from array of labelArray_t
  void setLabelArray(labelArray_t *labels) {
		_label_arr = labels;
	}

	// Set a single tab label
	void setLabel(uint16_t index, const char *label) {
		if (index < 10) {
			strncpy(_label_arr->labelstr[index], label, sizeof(_label_arr->labelstr[index]) - 1);
			_label_arr->labelstr[index][sizeof(_label_arr->labelstr[index]) - 1] = '\0'; // Ensure null-termination
		}
	}


  bool contains(int16_t x, int16_t y, uint16_t btn_number) {
		// Extends touch area to approx. text width - TODO!
		uint16_t y_inc  = _size + _size/4 + 2; // Increment y delta for the next button
		uint16_t y_start = _y + y_inc * btn_number; // Calculate the y position based on button number
		uint16_t y_end = y_start + y_inc; // Calculate the end y position

		if (btn_number >= _count) {
			return false; // Invalid button number
		}
    return ((x >= _x) && (x < (_x + _w)) &&
            (y >= y_start) && (y <= y_end));
  }

  bool checkPressed(bool wait_released) override {
    if (!_enabled) return false; // Do not check if not enabled
		bool pressed = false;
    if (_active && _touchProvider->pressed) {
			for (uint16_t j = 0; j < _count; j++) {
				if (contains(_touchProvider->tx, _touchProvider->ty, j)) {
					_selectedItem = j;
					pressed = true;
					if (_selectedItem != _last_index) {
						draw(_selectedItem); // Redraw the RadioButtons with the new checked state
					}
					_last_index = _selectedItem;
					_pressAction();
			    delay(20); // delay for debounce
					// If the RadioButton is pressed, wait until the touch is released
					if (wait_released) {
						_touchProvider->waitReleased();
					}
					break;
				}
			}
		}
		return pressed;
  }

  private:
	int16_t _last_index;
  labelArray_t *_label_arr; // Button text is 19 chars maximum per line
	uint16_t _count; // Number of Button checked, Number of radio buttons in the group
};

#endif // _RADIOButtonsH_
