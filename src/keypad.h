#ifndef KEYPAD_H
#define KEYPAD_H

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
//          by cm@keyboardpartner.de
//
// ############################################################################
*/

// Modal List Select Box for menus, directories, WIFI networks etc.
// If ENCODER_ENABLED is set, it uses the rotary encoder (param encdelta) for navigation
// encdelta should be NULL when ENCODER_ENABLED is not set

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "global_vars.h" // provides the speaker beep function
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts
#include "buttons.h"


#define KEYPAD_PADDING 10 // 10 Pixel line padding on each side
#define ENTRY_TOP 28 // Top position of the entry field relative to Y

#define SMALLBUTTON_W 50
#define SMALLBUTTON_H 18

#define ENTRY_TEXT_COLOR TFT_YELLOW
#define ENTRY_TEXT_FONT 2

// ##############################################################################

// Modal menu list, may be used for file selection or menu purposes
// Does not use GUIObject for itself
class NumericKeypad {
public:

// Constructor, initializes the numeric entry.
// "tft" is the TFT display object, "touchProvider" is the touch event provider
NumericKeypad(TFT_eSPI *tft, TouchProvider *touchProvider) : _tft(tft), _touchProvider(touchProvider), _btnCancel(tft, touchProvider) {}

	void init(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bgcolor) {
		_x = x;
		_y = y;
		_w = w;
		_h = h;
    _bgcolor = bgcolor;
		_pad_x = x + KEYPAD_PADDING;
		_pad_y = y + KEYPAD_PADDING + h/4;
		_pad_h = h - KEYPAD_PADDING - h/4;
		_pad_w = w - KEYPAD_PADDING * 2;
		_key_h = _pad_h / 4 - KEYPAD_PADDING; // 4 rows
		_key_w = _pad_w / 4 - KEYPAD_PADDING; // 4 columns
		_key_bordercolor = _tft->alphaBlend(128, TFT_WHITE, bgcolor);

		_entry_height = h/8; // Set entry field height
    _entry_left = x + KEYPAD_PADDING;
    _entry_top = y + ENTRY_TOP;
    _entry_width = w - KEYPAD_PADDING * 2;
    _entry_font = (_entry_height >= 28) ? 4 : 2;
    _entry_center_y = _entry_top + _entry_height / 2;

		_first_press = true; // Flag to track first press
	}

	bool isEntryValid() { return _entry_valid; } // Return true when entry was valid

	// Set the default entry value
	void setEntryValue(float value) { _entry_value = value; }



// ##############################################################################

  bool _contains(int16_t x, int16_t y, int row, int col) {
		// Extends touch area to approx. text width - TODO!
		int x_start = _pad_x + col * (_key_w + KEYPAD_PADDING);
		int y_start = _pad_y + row * (_key_h + KEYPAD_PADDING);
		int x_end = x_start + _key_w;
		int y_end = y_start + _key_h;
    return ((x >= x_start) && (x < x_end) &&
            (y >= y_start) && (y <= y_end));
  }

// ##############################################################################

	// Show a modal dialog with keypad, OK and cancel key
	// The user can enter a value (default set by setEntryValue(float)) using the keypad
	// returns the entered number
	float entry(String message1, uint16_t decimal_digits, bool use_plusminus) {
    // _tft->readRect(0,0, DISPLAY_W, DISPLAY_H, screenBuffer);
    _touchProvider->waitReleased();
		_use_decimal = decimal_digits > 0;
		_use_plusminus = use_plusminus;
		_tft->setTextFont(2);
		_tft->setTextColor(TFT_WHITE, _bgcolor);
		_tft->setTextDatum(TL_DATUM); // top left text datum

		_tft->drawRect(_x, _y, _w, _h, TFT_WHITE);
		_tft->fillRect(_x + 1, _y + 1, _w - 2, _h - 2, _bgcolor);
		_tft->drawString(message1, _pad_x + 2, _y + 6, 2);

		// draw entry field
		_tft->drawRect(_entry_left, _entry_top, _entry_width, _entry_height, TFT_WHITE);
		// Make Cancel Button
		_entry_valid = false;
		bool cancelled = false;
		bool enter_ok = false;
		bool decimal_entered = false;
		_btnCancel.init(_x + _w - SMALLBUTTON_W - KEYPAD_PADDING, _y + 5, SMALLBUTTON_W, SMALLBUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, 1, 1);
		_btnCancel.setLabel("CANCEL");
		_btnCancel.setActive(true, true);

    if (_use_decimal) {
      _entryStr = String(_entry_value, decimal_digits);
    } else {
      _entryStr = String((int)rint(_entry_value));
    }
		_drawEntryString();
		char const *label;

		// Draw 4 x 4 keypad
		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
				label = _keypad[row][col];
				_drawKeypadButton(label, false, col, row);
			}
		}
    unsigned long blink_time = millis();
    while (!enter_ok) {
      #ifdef ENCODER_ENABLED
        int enc_delta = _touchProvider->getEncDelta();
        if (enc_delta) {
          spkrTick(); // Play a click sound when an item is selected
          // Handle encoder input
          if (enc_delta > 0) {
            _entryStr = String(_entryStr.toInt() + 1);  // Encoder turned right
          } else {
            _entryStr = String(_entryStr.toInt() - 1);  // Encoder turned left
          }
          _drawEntryString();
        }
        if (!digitalRead(ENCBTN_PIN)) {
					spkrOKbeep();
          enter_ok = true;
          break;
        }
      #endif
			if (_touchProvider->checkWithin(_x, _y, _x + _w, _y + _h)) {
				if (_btnCancel.checkPressed(true)) { // Check if CANCEL button is pressed
					cancelled = true;
		  		spkrCancelBeep();
					break;
				}
				// check 4 x 4 keypad
				for (int row = 0; row < 4; row++) {
					for (int col = 0; col < 4; col++) {
						if (_contains(_touchProvider->tx, _touchProvider->ty, row, col)) {
              if (_first_press) {
                _first_press = false;
                if (_entryStr == "0") {
                  _entryStr = "";
                }
              }
							label = _keypad[row][col];
							if (label[0] == '\0') continue; // Skip empty labels
              decimal_entered = _entryStr.indexOf('.') >= 0 || _entryStr.indexOf(',') >= 0;
							if ((!_use_decimal || decimal_entered) && (label[0] == '.' || label[0] == ',')) continue; // Skip decimal point button
							_drawKeypadButton(label, true, col, row);
							if (row == 3 && col == 3) { // OK button
								enter_ok = true;
								spkrOKbeep();
								break;
							} else if (row == 0 && col == 3) { // Backspace button
								_entryStr.remove(_entryStr.length() - 1);
                if (_entryStr.length() == 0) {
                  _entryStr = "0";
                  _first_press = true; // start over again
                }
              } else if (row == 2 && col == 3) { // +/- button
                if (_use_decimal) {
								  _entryStr = String(-_entryStr.toFloat(), decimal_digits);
                } else {
                  _entryStr = String(-_entryStr.toInt());
                }
							} else {
								_entryStr += label;
							}
							_drawEntryString();
							spkrClick(); // Play a click sound when an item is selected
							delay(100);
							_touchProvider->waitReleased();
							_drawKeypadButton(label, false, col, row);
              _blink_toggle = true; // Reset blink toggle after a key press
						}
					}
				}
			}
      // Blink the cursor in the entry field
      if (millis() - blink_time > 250) {
        blink_time = millis();
        _blink_toggle = !_blink_toggle;  // Toggle blink state
        int text_width = _tft->textWidth(_entryStr, _entry_font);
        int text_height = _tft->fontHeight(_entry_font) - 2;
		    _tft->fillRect(_entry_left + text_width + 6, _entry_center_y - text_height / 2, 7, text_height, _blink_toggle ? ENTRY_TEXT_COLOR : TFT_BLACK);
      }
		}
		delay(100);
		_touchProvider->waitReleased();
		_tft->fillRect(_x, _y, _w, _h, TFT_BLACK);
		if (!cancelled && _entryStr.length()) {
			_entry_valid = true;
			_entry_value = _entryStr.toFloat(); // Convert the entry string to float
		}
		_tft->setTextDatum(TL_DATUM); // top left text datum
    //_tft->pushRect(0,0, DISPLAY_W, DISPLAY_H, screenBuffer);
		return _entry_value; // Return previous value if no valid entry was made
	}

// ############################################################################


private:
  TFT_eSPI *_tft;
  TouchProvider *_touchProvider;  // Pointer to the touch provider for touch handling
  int16_t _x, _y;              // Coordinates of top-left corner of dialog box
	int16_t _pad_x, _pad_y, _pad_h, _pad_w;      // Position and size of inner keypad
	int16_t _h, _w;              // Dimensions of the dialog box
	int16_t _key_h, _key_w;      // Dimensions of the keys
	int16_t _entry_top, _entry_left, _entry_height, _entry_width, _entry_center_y; // Height and padding of the entry field
  PushButton _btnCancel;
	uint16_t _bgcolor, _key_bordercolor;
  uint8_t _entry_font;

	// Keypad layout, disabled buttons are represented by empty strings
	char const *_keypad[4][4] = {
		{"1", "2", "3", "<<"},
		{"4", "5", "6", "\0"},
		{"7", "8", "9", "+/-"},
		{"\0", "0", ".", "OK"}
	};
	String _entryStr = "0"; // Entry string, initialized to "0"
	float _entry_value = 0.0;  // Default entry value
	bool _entry_valid = false; // Set to true if a valid entry is made
  bool _use_decimal = false;
  bool _use_plusminus = false;
  bool _first_press = true;
  bool _blink_toggle; // Toggle for blinking cursor

  	// Draw a single keypad button
	void _drawKeypadButton(char const *label, bool is_active, int col, int row) {
		if (label == nullptr || label[0] == '\0') {
			return; // Skip empty labels
		}
		if (!_use_decimal && (label[0] == '.' || label[0] == ',')) {
			return; // do not draw decimal point button
		}
    if (!_use_plusminus && (label[0] == '+' || label[0] == '-')) {
			return; // do not draw +/- button
		}
		uint16_t button_color = is_active ? TFT_WHITE : TFT_BTNGREY;
		int radius = 3;
		int x = _pad_x + col * (_key_w + KEYPAD_PADDING);
		int y = _pad_y + row * (_key_h + KEYPAD_PADDING);
		int32_t my_key_w = _key_w; // Adjust width for padding
		if (col == 3) { // OK or DEL button
			my_key_w += KEYPAD_PADDING; // Add padding to OK button
		}
		_tft->fillRoundRect(x, y, my_key_w, _key_h, radius, button_color);
		_tft->setTextColor(is_active ? TFT_BLACK : TFT_WHITE, button_color);
		_tft->setTextDatum(MC_DATUM); // mid center text datum
		_tft->drawString(label, x + my_key_w/2 + 1, y + _key_h/2, 2);
		_tft->drawRoundRect(x, y, my_key_w, _key_h, radius, TFT_WHITE);
		_tft->drawRoundRect(x - 1, y - 1, my_key_w + 2, _key_h + 2, radius, _key_bordercolor);
	}

	void _drawEntryString() {
		_tft->fillRect(_entry_left + 1, _entry_top + 1, _entry_width - 2, _entry_height - 2, TFT_BLACK);
		_tft->setTextDatum(CL_DATUM); // top left text datum
		_tft->setTextColor(ENTRY_TEXT_COLOR, TFT_BLACK);
		_tft->drawString(_entryStr, _entry_left + 5, _entry_center_y, _entry_font);
	}

};

#endif
