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

#ifndef _ENCODERENTRY_H_
#define _ENCODERENTRY_H_

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts


// Integer entry field with encoder inc/dec, inherits from GUIObject
// Uses a rotary encoder connected to pins defined in "hwdefs.h"

class EncoderEntryField : public GUIObject {

  public:

  EncoderEntryField(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  void init(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t bgcolor, uint16_t textcolor, uint16_t borderwidth = 1, uint8_t textfont = 2) {
		_x           = x;
		_y           = y;
		_w           = w;
		_h           = h;
		_xd          = 0;
		_yd          = 0;
		_borderwidth  = borderwidth;
		_bordercolor 	= bordercolor;
		_bgcolor      = bgcolor;
		_textcolor    = textcolor;
		_textfont     = textfont; // use TextFont
		_textfreefont = NULL;
		_value = 0;
		_textdatum = TC_DATUM; // Center datum for text
    _label[0] = '\0';
		_enabled = true;    // enabled by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

  // Overloaded function to use a FreeFont instead of a text font
  void init(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t bgcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont) {
		_x           = x;
		_y           = y;
		_w           = w;
		_h           = h;
		_xd          = 0;
		_yd          = 0;
		_borderwidth  = borderwidth;
		_bordercolor 	= bordercolor;
		_bgcolor      = bgcolor;
		_checkedcolor = textcolor;
		_textcolor    = textcolor;
		_textfont     = 255;// Default to 255, meaning use the FreeFont
		_textfreefont = textfreefont;
		_value = 0;
    _label[0] = '\0';
		_textdatum = TC_DATUM; // Center datum for text
		_enabled = true;    // enabled by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

  void drawFrame(uint16_t bordercolor) {
    if (_borderwidth > 0) {
      for (int i = 0; i < _borderwidth; i++) {
        _tft->drawRect(_x + i, _y + i, _w - (i * 2), _h - (i * 2), bordercolor);
      }
    }
  }

  void draw(int value, bool inverse) {
	  if (!_visible || !_enabled) return; // Do not draw if not visible or not enabled
		uint16_t bw2 = _borderwidth * 2;
		uint16_t my_textcolor = _textcolor;
		uint16_t my_bgcolor = _bgcolor;
		uint16_t my_bordercolor = _bordercolor;
		if (!_active) {
			my_textcolor = TFT_DARKGREY; // Use dark grey for inactive RadioButtons
      my_bgcolor = TFT_BLACK;
			my_bordercolor = TFT_GREY; // Use dark grey for inactive RadioButtons
		}
    if (inverse) {
      my_textcolor = _bgcolor;
      my_bgcolor = _textcolor;
    }
    uint8_t tempdatum = _tft->getTextDatum();
    uint16_t tempPadding = _tft->getTextPadding();
    drawFrame(my_bordercolor);
    _tft->fillRect(_x + _borderwidth, _y + _borderwidth, _w - _borderwidth * 2, _h - _borderwidth * 2, my_bgcolor); // Fill background
    // Draw the slider label
    if (_label[0] != '\0') { // Do not draw label if empty
      int16_t center_x = _x + (_w / 2);
      int16_t center_y = _y + (_h / 2);
      _tft->setTextColor(my_bordercolor);
      switch (_textdatum) {
      case TC_DATUM:
        _tft->setTextDatum(BC_DATUM); // Set text datum for label above switch
        _tft->drawString(_label, center_x + _xd, _y + _yd - 2, 2);
        break;
      case BC_DATUM:
        _tft->setTextDatum(TC_DATUM); // Set text datum for label below switch
        _tft->drawString(_label, center_x + _xd, _y + _yd + _h + 2, 2);
        break;
      case CL_DATUM:
      case BL_DATUM:
      case TL_DATUM:
        _tft->setTextDatum(_textdatum); // Set text datum for label
        _tft->drawString(_label, _x + _xd - 2, center_y + _yd, 2);
        break;
      case CR_DATUM:
      case BR_DATUM:
      case TR_DATUM:
        _tft->setTextDatum(_textdatum); // Set text datum for label
        _tft->drawString(_label, _x + _xd + _w + 2, center_y + _yd, 2);
        break;
      default:
        _tft->drawString(_label, _x + _xd, _y + _yd, 2);
      }
    }
    if (_textfont == 255) {
      _tft->setFreeFont(_textfreefont); // Use FreeFont if set
    } else {
      _tft->setTextFont(_textfont);
    }
    _tft->setTextColor(my_textcolor, my_bgcolor);
		_tft->setTextDatum(CL_DATUM); // Center datum for text
    _tft->drawString(String(_value), _x + _borderwidth + 4, _y + _h / 2 - 1);
		_tft->setTextDatum(tempdatum);
		_tft->setTextPadding(tempPadding);
	}

	// Enabled state overrides Active and Visible states.
  // Object will be ignored and not drawn if not enabled
  // Overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      draw(_value, false);
  }

  // set entry field active (or inactive) and redraw (or not)
	// Overload with redraw
	void setActive(bool active, bool redraw = false) {
		_active = active;
		if (redraw) draw(_value, false);
	}

  // Set value and redraw (or not)
  void setValue(uint16_t value, bool redraw = false) {
		_value = value;
		if (redraw) draw(_value, false);
	}

  void setLimits(int16_t min, int16_t max) {
    _min = min;
    _max = max;
  }

  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(_value, false); // draw object
  }

  // Rotary encoder is enabled by default, may be disabled if needed
  void setEncoderEnabled(bool enabled) {
    _encoder_enabled = enabled;
  }

  // A touch within the field enters a modal state where the value can be changed with the encoder
  bool checkPressed(bool wait_released) override {
    if (!_enabled) return false; // Do not check if not enabled
		bool pressed = false;
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      pressed = true; // Touch is pressed within the object boundaries
      #ifdef ENCODER_ENABLED
        if (!_encoder_enabled) {
          _pressAction(); // Call the press action callback if defined
          if (wait_released)
            _touchProvider->waitReleased();
          return pressed; // Return pressed state without further action
        }
        draw(_value, true);
        spkrClick();
        delay(50);
        _touchProvider->waitReleased();
        _blink_toggle = false;
        uint32_t blink_time = millis();
        while (true) {
          int enc_delta = _touchProvider->getEncDelta();
          if (enc_delta) {
            spkrTick(); // Play a click sound when an item is selected
            // Handle encoder input
            _value += enc_delta;
            if (_value < _min) {
              _value = _min;
            } else if (_value > _max) {
              _value = _max;
            }
            draw(_value, true);
          }
          if (millis() - blink_time > 250) {
            blink_time = millis();
            // Toggle blink state
            drawFrame( _blink_toggle ? _bordercolor : (_bordercolor ^ 0xFFFF)); // Toggle border color
            _blink_toggle = !_blink_toggle;
          }
          if (!digitalRead(ENCBTN_PIN)) {
            spkrOKbeep();
            break;
          }
          if (_touchProvider->checkTouch()) {
            spkrOKbeep();
            break;
          }
          delay(10);
        }
        draw(_value, false);
        while (!digitalRead(ENCBTN_PIN)) {
          delay(10); // wait until released
        }
        _touchProvider->waitReleased();
      #else
        // Handle non-encoder case. A numeric keypad may be used in main program by checking "pressed" result
        // or using ActionCallback set by setPressAction(function), see below.
        // As the keypad erases the screen area, it needs to be redrawn in main program.
        _pressAction(); // Call the press action callback if defined
        if (wait_released)
          _touchProvider->waitReleased();
      #endif
		}
		return pressed;
  }

  private:
	int16_t _value;
  int16_t _max = 999;
  int16_t _min = 0;
  bool _encoder_enabled = true; // Enable or disable encoder input handling
  bool _blink_toggle;
};

#endif // _ENCODERENTRY_H_
