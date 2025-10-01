/***************************************************************************************
// The following single tab class has been ported over from the Adafruit_tft library.
// A slightly different implementation in this single tabHor library allows:
//
// 1. The single tab labels to be in any font
// 2. Allow longer labels
// 3. Allow text datum to be set
// 4. Allow invert state to be read via getState(), true = inverted
// 5. Define pressed and released callbacks per single tab instances
//
// The library uses functions present in TFT_eSPI
// https://github.com/Bodmer/TFT_eSPI

// Addittion by cm 7/25:
// TouchProvider class for handling touch events as a common provider for all Hors
// This allows the single tabHor to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly.
// Usage: Instantiate TouchProvider once in main.cpp and pass it to Hor constructors
// Example:
// TouchProvider touchProvider = TouchProvider(&tft);
// Then, pass it to constructor:
// single tabHor testsingle tab1 = single tabHor(&tft, &touchProvider);
// Button   testButton1   = ButtonHor(&tft, &touchProvider);
//
// Added visibility and enabled control:
// A tab group can be made invisible or "dead" (not responding to touches, not redrawn)
// as needed.
//
****************************************************************************************/

#ifndef _PANELTABS_H_
#define _PANELTABS_H_

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "global_vars.h" // provides the speaker beep function
#include "touchProvider.h" // Common touch provider for all Hors
#include "guiObject.h" // Common GUI object for all Hors
#include "Free_Fonts.h" // Include large fonts

#define FADE_BLEND 128

// Set of tabs on bottom of panel area with a common area for tab content
class BottomTabs : public GUIObject {

  public:

  BottomTabs(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  // New/alt init() uses upper-left corner & height
  void init(int16_t x, int16_t y_baseline, int16_t width, int16_t height, uint16_t count,
							uint16_t bordercolor, uint16_t fillcolor, uint16_t textcolor,
							uint8_t textfont = 2) {
		_x           = x;
		_y           = y_baseline;
		_h           = height;
		_tabwidth     = width / count; // default tab width
		_totalwidth   = width;
		_bordercolor 	= bordercolor;
		_fillcolor    = fillcolor;
		_checkedcolor = textcolor;
		_textcolor    = textcolor;
		_textfont     = textfont;
  	_textfreefont = NULL;
		_selectedItem      = 0;
		_last_selectedItem  = 0;
		_label_arr = NULL; // Initialize all label strings to empty
		_count = count;
		_enabled = true;    // enabled by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

  // Overloaded function to use a FreeFont instead of a text font
  void init(int16_t x, int16_t y_baseline, uint16_t width, uint16_t height, uint16_t count,
							uint16_t bordercolor, uint16_t fillcolor, uint16_t textcolor,
							const GFXfont *textfreefont) {
		_x           = x;
		_y           = y_baseline;
		_h           = height;
		_tabwidth     = width / count; // default tab width
		_totalwidth   = width;
		_bordercolor 	= bordercolor;
		_fillcolor    = fillcolor;
		_checkedcolor = textcolor;
		_textcolor    = textcolor;
		_textfont     = 255;// Default to 255, meaning use the FreeFont
		_textfreefont = textfreefont;
		_selectedItem      = 0;
		_last_selectedItem  = 0;
		_label_arr = NULL; // Initialize all label strings to empty
		_count = count;
		_enabled = true;    // enabled by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

  // X Offset for first tab
  void setOffsetX(int16_t x_delta) {
		_xd        = x_delta;
	}

  // Y Offset for window area (top)
  void setOffsetY(int16_t y_top) {
		_yd        = y_top;
	}

  // Override default tab width (total width of all tabs divided by count)
  void setTabWidth(int16_t tab_width) {
		_tabwidth = tab_width;
	}

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

  // set Tabs active (or inactive) and redraw (or not)
	void setActive(bool active, bool redraw = false) {
		_active = active;
		if (redraw)
			draw(_selectedItem);
	}

	// Enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
  // overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      draw(_selectedItem);
  }

  // Set selected (highlighted) tab according to index
	// Overload with redraw
	void setSelectedItem(uint16_t selected_item, bool redraw = false) {
		_selectedItem = selected_item;
		if (redraw)
			draw(_selectedItem);
	}

	uint16_t getCount(void) {return _count;}            // Get the number of tabs in the group

	// --------------------------------------------------------------------------------

	void drawBottomTab(int16_t x, int16_t y, int16_t w, int16_t h, char *label, uint16_t textcolor, uint16_t bordercolor, uint16_t fillcolor) {
		int16_t r = h / 4;
		int16_t r2 = r * 2;
		int16_t hr = h - r; // Reduce height for rounded corners
		int16_t xw = x + w;
		_tft->fillRect(x, y, w, hr, fillcolor);
		// Draw outer half rounded rect, corner = 1 = bottom, corner = 2 = top
		_tft->fillCircleHelper(x + r, y + hr - 1, r, 1, w - r2 - 1, bordercolor);
		if (bordercolor != fillcolor) {
			_tft->drawFastVLine(x, y, hr, bordercolor); // left border
			_tft->drawFastVLine(x + 1, y, hr, bordercolor); // inner left border
			_tft->drawFastVLine(xw - 1, y, hr, bordercolor); // right border
			_tft->drawFastVLine(xw - 2, y, hr, bordercolor); // inner right border
			// Draw inner half rounded rect, corner = 1 = bottom, corner = 2 = top
			_tft->fillCircleHelper(x + r + 1, y + hr - 1, r - 2, 1, w - r2 - 3, fillcolor);
		}
		_tft->setTextDatum(MC_DATUM);
		if (_textfont == 255) {
			_tft->setFreeFont(_textfreefont); // Set font for the label
		} else {
			_tft->setTextFont(_textfont); // Set default font
		}
		_tft->setTextColor(textcolor, fillcolor);
		_tft->drawString(label, x + w / 2, y + 2 + h / 2);
	}

	// --------------------------------------------------------------------------------

  void draw(uint16_t item) {
    if (!_visible || !_enabled) return; // Do not draw if not visible or not alive
		uint16_t my_textcolor;
		uint16_t my_fadecolor = _bordercolor ^ 0xFFFF; // inverse border color
		uint16_t my_bordercolor;
		uint16_t my_fillcolor;
  	uint8_t temp_datum = _tft->getTextDatum();
		uint16_t temp_padding = _tft->getTextPadding();

		// draw open box around screen area
		if (!_active) {
			my_bordercolor = TFT_GREY; // Use dark grey for inactive tabButtons
			my_fillcolor = TFT_BLACK; // Use black for inactive tabButtons
		} else {
			my_bordercolor = _bordercolor;
			my_fillcolor = _fillcolor;
		}
		if (_bordercolor != _fillcolor) {
			_tft->drawFastHLine(_x, _yd, _totalwidth, my_bordercolor); // Draw bottom border line
			_tft->drawFastHLine(_x, _yd + 1, _totalwidth, my_bordercolor);
			_tft->drawFastVLine(_x, _yd, _y, my_bordercolor); // Draw left border line
			_tft->drawFastVLine(_x + 1, _yd, _y, my_bordercolor); // Draw left border line
			_tft->drawFastVLine(_totalwidth - 1, _yd, _y, my_bordercolor); // Draw right border line
			_tft->drawFastVLine(_totalwidth - 2, _yd, _y, my_bordercolor); // Draw right border line
			_tft->fillRect(_x + 2, _yd + 2, _totalwidth - 4, _y - _yd - 1, my_fillcolor);
		} else {
			_tft->fillRect(_x, _yd, _totalwidth, _y - _yd + 1, my_fillcolor); // Fill the whole area with fill color
		}

		for (uint16_t j = 0; j < _count; j++) {
			if (!_active) {
				my_bordercolor = TFT_GREY; // Use dark grey for inactive tabButtons
				my_textcolor = TFT_DARKGREY; // Use dark grey for inactive tabButtons
				my_fillcolor = TFT_BLACK; // Use black for inactive tabButtons
			} else {
				if (j == item) {
					my_bordercolor = _bordercolor;
					my_textcolor = _textcolor;
					my_fillcolor = _fillcolor;
				} else {
					// Use faded darker colors for inactive tabs
					my_bordercolor = _tft->alphaBlend(FADE_BLEND, _bordercolor, my_fadecolor);
					my_textcolor = _tft->alphaBlend(FADE_BLEND, _textcolor, my_fadecolor);
					my_fillcolor = _tft->alphaBlend(FADE_BLEND, _fillcolor, my_fadecolor);
				}
			}
			_tft->setTextColor(my_textcolor, my_fillcolor);
			if (_label_arr != NULL && _label_arr->labelstr[j][0] != '\0') {
			  drawBottomTab(_x + _xd + j * _tabwidth, _y, _tabwidth, _h, _label_arr->labelstr[j], my_textcolor, my_bordercolor, my_fillcolor);
      } else {
        drawBottomTab(_x + _xd + j * _tabwidth, _y, _tabwidth, _h, "", my_textcolor, my_bordercolor, my_fillcolor);
      }
		}
		// line to first checked tab
		if (!_active) {
			my_bordercolor = TFT_GREY; // Use dark grey for inactive tabButtons
			my_textcolor = TFT_DARKGREY; // Use dark grey for inactive tabButtons
			my_fillcolor = TFT_BLACK; // Use black for inactive tabButtons
		} else {
			my_bordercolor = _bordercolor;
			my_textcolor = _textcolor;
			my_fillcolor = _fillcolor;
		}
		int16_t w = _xd + item * _tabwidth;
		_tft->drawFastHLine(_x, _y, w, my_bordercolor); // Draw bottom border line
		_tft->drawFastHLine(_x, _y + 1, w, my_bordercolor);

		int16_t x = _x + w + _tabwidth; // Calculate the x position after checked tab
		w = _totalwidth - x; // Calculate the width of the remaining tabs
		_tft->drawFastHLine(x, _y, w, my_bordercolor); // Draw bottom border line
		_tft->drawFastHLine(x, _y + 1, w, my_bordercolor);

		_tft->setTextDatum(temp_datum);
		_tft->setTextPadding(temp_padding);
	}


  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(_selectedItem); // draw object
  }

	// --------------------------------------------------------------------------------

  bool contains(int16_t x, int16_t y, uint16_t tab_number) {
		// Extends touch area to approx. text height - TODO!
		int16_t x_start = _x + _xd + _tabwidth * tab_number; // Calculate the x position based on tab number
		int16_t x_end = x_start + _tabwidth; // Calculate the end x position

		if (tab_number >= _count) {
			return false; // Invalid tab number
		}
    return ((x >= x_start) && (x < x_end) &&
            (y >= _y) && (y <= (_y + _h)));
  }

	// --------------------------------------------------------------------------------

  bool checkPressed(bool wait_released) override {
    if (!_enabled) return false; // Do not check if not enabled
		bool pressed = false;
    if (_active && _touchProvider->pressed) {
			for (uint16_t j = 0; j < _count; j++) {
				if (contains(_touchProvider->tx, _touchProvider->ty, j)) {
					_selectedItem = j;
					pressed = true;
					if (_selectedItem != _last_selectedItem) {
						draw(_selectedItem); // Redraw the tabButtons with the new checked state
					}
					_last_selectedItem = j;
					_pressAction();
          delay(20); // Debounce delay to avoid multiple presses
					// If the tabButton is pressed, wait until the touch is released
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
  labelArray_t *_label_arr; // Button text is 19 chars maximum per line
  int16_t _tabwidth, _totalwidth;
  uint16_t  _last_selectedItem; // Selected tab index
	uint16_t  _count; // Number of the tab checked, Number of tabs in the group
};

#endif // _tabButtonsH_
