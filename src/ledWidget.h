#ifndef _LEDWIDGETH_
#define _LEDWIDGETH_

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

// LED Class for TFT_eSPI display,
// providing a simple way to draw and control LED-like indicators on the screen.

// Added visibility and "alive" control:
// A LED can be made invisible or "dead" (not responding to touches, not redrawn)
// as needed.

enum led_type_t {
	LED_ROUND,
	LED_SQUARE,
	LED_RECT
};

// Displays a round or rectangular LED with blinking capability
class LedWidget : public GUIObject {

public:
	// Constructor to initialize the LED widget with default values
	// Uses TFT_eSPI for drawing on the display
	LedWidget(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

	// Draw a LED at the specified position with the specified size and color
	// If is_on is true, the LED is drawn in the specified color,
	// otherwise it is drawn in a darker shade of the specified color
	// x, y are the center coordinates of the LED, size is the size of the LED
	void init(int x, int y, int size, uint16_t color = TFT_RED, led_type_t type = LED_ROUND, u_int8_t textfont = 2) {
		initCenter(x - size/2, y - size/2, size, color, type, textfont);
	}

	void initCenter(int x, int y, int size, uint16_t color = TFT_RED, led_type_t type = LED_ROUND, u_int8_t textfont = 2) {
		_x = x; // Middle X coordinate!
		_y = y; // Middle Y coordinate!
		_xd = 0;
		_yd = 0;
    _h  = size;
    _w  = size;
		_size = size;
		_color = color;
		_textdatum = TC_DATUM; // Default text position above the LED
		_textfont = textfont;
		_freq = 333; 			// Default blink frequency in milliseconds
		_isOn = false;
		_isBlinking = false;
		_toggle = false;  // Initial toggle state
		_last_millis = millis();
		_laststate = false; // Last state of the LED
		_type = type; 		// Set the type of the LED
  	_label[0] = '\0'; // Initialize label to empty string
		_enabled = true;    // alive by default
		_active = true;  // active by default
		_visible = true;  // visible by default
	}

	void setBlinkFreq(uint16_t freq = 333) { _freq = freq; }
	uint16_t getBlinkFreq() const { return _freq; }

	// Check blinking state and update the display if necessary
	// If force_redraw is true, the LED is always redrawn,
	// otherwise it is only redrawn if the state or blink toggle has changed
	void draw() {
	  if (!_visible || !_enabled) return; // Do not draw if not visible or not alive

    _tft->setTextFont(_textfont); // Set text font for label
    if (_active)
      _tft->setTextColor(TFT_WHITE);
    else
      _tft->setTextColor(TFT_GREY);
    switch (_textdatum) {
    case TC_DATUM:
      _tft->setTextDatum(BC_DATUM); // Set text datum for label above LED
      _tft->drawString(_label, _x + _xd + 2, _y + _yd - _size -2);
      break;
    case BC_DATUM:
      _tft->setTextDatum(TC_DATUM); // Set text datum for label below LED
      _tft->drawString(_label, _x + _xd + 2, _y + _yd + _size + 2);
      break;
    case CL_DATUM:
    case BL_DATUM:
    case TL_DATUM:
      _tft->setTextDatum(_textdatum); // Set text datum for label
      _tft->drawString(_label, _x + _xd + _size + 2, _y + _yd);
      break;
    case CR_DATUM:
    case BR_DATUM:
    case TR_DATUM:
      _tft->setTextDatum(_textdatum); // Set text datum for label
      _tft->drawString(_label, _x + _xd - _size - 2, _y + _yd);
      break;
    default:
      _tft->drawString(_label, _x + _xd, _y + _yd);
    }
    _tft->setTextDatum(TL_DATUM); // Reset text datum to top left
    // If the state has changed or is not initialized, redraw the LED
    uint16_t dim_color_inner, dim_color_outer, bright_color;
    if (_active) {
      if (_isOn && _toggle) {
        dim_color_inner = _color;
        dim_color_outer = _tft->alphaBlend(190, _color, TFT_DARKGREY);
        bright_color = _tft->alphaBlend(110, TFT_WHITE, _color);
      } else {
        dim_color_inner = _tft->alphaBlend(60, _color, TFT_DARKGREY);
        dim_color_outer = _tft->alphaBlend(40, _color, TFT_DARKGREY);
        bright_color = dim_color_inner;
      }
    } else {
      dim_color_inner = TFT_DARKGREY;
      dim_color_outer = TFT_GREY;
      bright_color = TFT_DARKGREY;
    }
    if (_type == LED_ROUND) {
      int radius = _size / 2;
      int radius_2 = radius / 2 + 1; // Radius for inner circle or square
      _tft->fillCircle(_x, _y, radius - 1, dim_color_outer); // Draw outer circle
      _tft->fillCircle(_x, _y, radius_2, dim_color_inner); // Draw inner circle
      _tft->fillCircle(_x, _y, 2, bright_color); // bright inner circle
      if (_active) {
        _tft->drawCircle(_x, _y, radius - 1, TFT_WHITE); // Draw outline
        _tft->drawCircle(_x, _y, radius, TFT_LIGHTGREY); // Draw bezel
      } else {
        _tft->drawCircle(_x, _y, radius - 1, TFT_GREY); // Draw outline
        _tft->drawCircle(_x, _y, radius, TFT_DARKGREY); // Draw bezel
      }
    } else if (_type == LED_SQUARE) {
      int square_size = _size; // Inner rect height
      int square_size_inner = _size / 2; // Inner rect height
      int size_inner = square_size_inner / 2; // Inner rect size
      _tft->fillRect(_x - square_size_inner, _y - square_size_inner, square_size, square_size, dim_color_outer); // LED square
      _tft->fillRect(_x - size_inner, _y - size_inner, square_size_inner, square_size_inner, dim_color_inner); // Inner square
      _tft->fillRect(_x - 1, _y - 1, 2, 2, bright_color); // bright inner  square
      if (_active) {
        _tft->drawRect(_x - square_size_inner + 1, _y - square_size_inner + 1, square_size - 2, square_size - 2, TFT_WHITE); // Outline
        _tft->drawRect(_x - square_size_inner, _y - square_size_inner, square_size, square_size, TFT_LIGHTGREY); // Bezel
      } else {
        _tft->drawRect(_x - square_size_inner + 1, _y - _size + 1, square_size - 2, square_size - 2, TFT_GREY); // Outline
        _tft->drawRect(_x - square_size_inner, _y - square_size_inner, square_size, square_size, TFT_DARKGREY); // Bezel
      }

    } else if (_type == LED_RECT) {
      int width = (_size * 4) / 3;  // Inner rect width
      int height = (_size * 3) / 4; // rectangle height
      int width_2 = width / 2;   		//  half inner rect width
      int height_2 = height / 2; 		// half inner rectangle height
      _tft->fillRect(_x - width_2, _y - height_2, width, height, dim_color_outer); // LED square
      _tft->fillRect(_x - width_2 / 2, _y - height_2 / 2, width_2, height_2, dim_color_inner); // Inner square
      _tft->fillRect(_x - 2, _y - 1, 4, 2, bright_color); // bright inner square
      if (_active) {
        _tft->drawRect(_x - width_2 + 1, _y - height_2 + 1, width - 2, height - 2, TFT_WHITE); // Outline
        _tft->drawRect(_x - width_2, _y - height_2, width, height, TFT_LIGHTGREY); // Bezel
      } else {
        _tft->drawRect(_x - width_2 + 1, _y - height_2 + 1, width - 2, height - 2, TFT_GREY); // Outline
        _tft->drawRect(_x - width_2, _y - height_2, width, height, TFT_DARKGREY); // Bezel
      }
    }

	}

	// Set the LED state and optionally enable blinking
	// If force_redraw is true, the LED is always redrawn,
	// otherwise it is only redrawn if the state has changed
	void setState(bool isOn, bool isBlinking = false, bool force_redraw = false) {
		_isOn = isOn;
		_checked = isOn; // Update checked state to match LED state
		_isBlinking = isBlinking;
    if (force_redraw) {
      draw();
		  _laststate = _isOn; // prevents unnecessary redraws
    }
	}

	// Enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if ((_visible && _enabled) || redraw)
      draw();
		  _laststate = _isOn; // prevents unnecessary redraws
  }

  // Active state, object responds to user input when active; may be visible or not visible, though
	void setActive(bool active) { _active = active; }
  // set switch active (or inactive) and redraw (or not), overload with redraw
  void setActive(bool active, bool redraw) {
    _active = active;
    if (redraw)
      draw();
		  _laststate = _isOn; // prevents unnecessary redraws
  }

  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    draw(); // draw object
		_laststate = _isOn; // prevents unnecessary redraws
  }

  void update() override {
	  if (!_visible || !_enabled) return; // Do not draw if not visible or not alive
    // regular update needed when blinking
    if (_isBlinking) {
      uint32_t currentMillis = millis();
      if (currentMillis - _last_millis >= _freq) {
        _last_millis = currentMillis;
        _toggle = !_toggle;
        draw();
      }
    } else {
      if (_laststate != _isOn) {
        draw();
      }
    }
		_laststate = _isOn;
  }

private:
	uint16_t _freq; // Blink frequency in milliseconds
	bool _isBlinking; // LED blinking state
	bool _toggle; // Toggle state for blinking effect
	uint32_t _last_millis; // Last update time
  led_type_t _type; // Type of the LED, default is ROUND
};

#endif // _LEDWIDGETH_