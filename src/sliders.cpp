#include "sliders.h"

/***************************************************************************************
** Code for the GFX slider UI element
** Grabbed from Adafruit_tft library and enhanced to handle any label font
** Additions by cm to set button active or inactive
***************************************************************************************/

// ##############################################################################
//
//  ##     ##  #######  ########  #### ########
//  ##     ## ##     ## ##     ##  ##       ##
//  ##     ## ##     ## ##     ##  ##      ##
//  ######### ##     ## ########   ##     ##
//  ##     ## ##     ## ##   ##    ##    ##
//  ##     ## ##     ## ##    ##   ##   ##
//  ##     ##  #######  ##     ## #### ########
//
// ##############################################################################


// Classic init() function: pass center & size
void SliderHor::initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor,
                              uint16_t textcolor, uint16_t borderwidth, uint8_t textfont)
{
  // Tweak arguments and pass to the newer initUL() function...
  init(x - (w / 2), y - (h / 2), w, h, bordercolor, thumbcolor, textcolor, borderwidth, textfont);
}

// Overloaded function to use a FreeFont instead of a text font
void SliderHor::initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor,
                              uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont)
{
  // Tweak arguments and pass to the newer initUL() function...
  init(x - (w / 2), y - (h / 2), w, h, bordercolor, thumbcolor, textcolor, borderwidth, textfreefont);
}

// Newer function instead accepts upper-left corner & size
void SliderHor::init(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor,
                                uint16_t textcolor, uint16_t borderwidth, uint8_t textfont) {
  _x           	= x;
  _y           	= y;
  _w            = w;
  _h            = h;
  _borderwidth  = borderwidth;
  _thumbcolor   = thumbcolor;
  _bordercolor  = bordercolor;
  _borderwidth  = borderwidth;
  _textcolor    = textcolor;
  _textfont     = textfont;
  resetCommon();
}

// Overloaded function to use a FreeFont instead of a text font
void SliderHor::init(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor,
                                uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont) {
  _x           = x;
  _y           = y;
  _w            = w;
  _h            = h;
  _borderwidth  = borderwidth;
  _thumbcolor   = thumbcolor;
  _bordercolor  = bordercolor;
  _textcolor    = textcolor;
  _textfreefont = textfreefont;
  _textfont     = 255; // Default to 255, meaning use the FreeFont
  resetCommon();
}

void SliderHor::draw(float level, bool draw_border) {
  if (!_visible || !_enabled) return; // Do not draw if not visible
  uint16_t my_bordercolor, my_thumbcolor, my_textcolor, my_fillcolor;
  if (_active) {
    my_bordercolor = _bordercolor;
    my_thumbcolor = _thumbcolor;
    my_textcolor = _textcolor;
    my_fillcolor =  _fillcolor;
  } else {
    my_bordercolor = TFT_DARKGREY;
    my_thumbcolor = TFT_DARKGREY;
    my_textcolor = TFT_DARKGREY;
    my_fillcolor = TFT_BLACK;
  }

  int16_t bw2 = _borderwidth * 2;
  int16_t r = _h / 6; // Corner radius
	int16_t center_x = _x + (_w/2);
	int16_t center_y = _y + (_h/2);

  if ((_borderwidth > 0) && draw_border) {
    for (int i = 0; i < _borderwidth; i++) {
      _tft->drawRoundRect(_x + i, _y + i, _w - (i * 2), _h - (i * 2), r, my_bordercolor);
    }
  }

  _thumb_delta = rint(_thumb_travel * (level)); // Calculate thumb position based on level
	// slider area color
	_tft->fillRoundRect(_x+_borderwidth, _y+_borderwidth, _w - bw2, _h - bw2, r-_borderwidth, my_fillcolor);

  int16_t knob_x = _thumb_start + _thumb_delta - _thumb_width/2; // Calculate thumb x position
  int16_t knob_center_x = knob_x + _thumb_width/2; // middle of thumb
	_tft->fillRoundRect(knob_x, _thumb_y, _thumb_width, _thumb_height, r, my_thumbcolor);

  // Riffelung
	int16_t my_thumb_y = _thumb_y + 3;
  int16_t line_len = _thumb_height - 6;
  _tft->drawFastVLine(knob_center_x - 5, my_thumb_y, line_len, my_bordercolor);
  _tft->drawFastVLine(knob_center_x, my_thumb_y, line_len, my_bordercolor);
  _tft->drawFastVLine(knob_center_x + 5, my_thumb_y, line_len, my_bordercolor);

  // Draw the slider label
  if (_label[0] == '\0') return; // Do not draw label if empty

  _tft->setTextColor(my_textcolor);
  uint8_t tempdatum = _tft->getTextDatum();
  uint16_t tempPadding = _tft->getTextPadding();
  if (_textfont == 255) {
    _tft->setFreeFont(_textfreefont); // Use FreeFont if set
  } else {
    _tft->setTextFont(_textfont);
  }
  _tft->setTextPadding(0);

	switch (_textdatum) {
	case TC_DATUM:
		_tft->setTextDatum(BC_DATUM); // Set text datum for label above slider
		_tft->drawString(_label, center_x + _xd, _y + _yd - 2);
		break;
	case BC_DATUM:
		_tft->setTextDatum(TC_DATUM); // Set text datum for label below slider
		_tft->drawString(_label, center_x + _xd, _y + _yd + _h + 2);
		break;
	case CL_DATUM:
	case BL_DATUM:
	case TL_DATUM:
		_tft->setTextDatum(_textdatum); // Set text datum for label
		_tft->drawString(_label, _x + _xd - 2, center_y + _yd);
		break;
	case CR_DATUM:
	case BR_DATUM:
	case TR_DATUM:
		_tft->setTextDatum(_textdatum); // Set text datum for label
		_tft->drawString(_label, _x + _xd + _w + 2, center_y + _yd);
		break;
	default:
		_tft->drawString(_label, _x + _xd, _y + _yd);
	}
  _tft->setTextDatum(tempdatum);
  _tft->setTextPadding(tempPadding);
}

// ##############################################################################
//
//  ##     ## ######## ########  ########
//  ##     ## ##       ##     ##    ##
//  ##     ## ##       ##     ##    ##
//  ##     ## ######   ########     ##
//   ##   ##  ##       ##   ##      ##
//    ## ##   ##       ##    ##     ##
//     ###    ######## ##     ##    ##
//
// ##############################################################################


// Classic init() function: pass center & size
void SliderVert::initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor,
                              uint16_t textcolor, uint16_t borderwidth, uint8_t textfont)
{
  // Tweak arguments and pass to the newer initUL() function...
  init(x - (w / 2), y - (h / 2), w, h, bordercolor, thumbcolor, textcolor, borderwidth, textfont);
}

// Overloaded function to use a FreeFont instead of a text font
void SliderVert::initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor,
                              uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont)
{
  // Tweak arguments and pass to the newer initUL() function...
  init(x - (w / 2), y - (h / 2), w, h, bordercolor, thumbcolor, textcolor, borderwidth, textfreefont);
}

// Newer function instead accepts upper-left corner & size
void SliderVert::init(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor, uint16_t thumbcolor,
                                uint16_t textcolor, uint16_t borderwidth, uint8_t textfont) {
  _x           	= x;
  _y           	= y;
  _w            = w;
  _h            = h;
  _borderwidth  = borderwidth;
  _thumbcolor   = thumbcolor;
  _bordercolor  = bordercolor;
  _borderwidth  = borderwidth;
  _textcolor    = textcolor;
  _textfont     = textfont;
  resetCommon();
}

// Overloaded function to use a FreeFont instead of a text font
void SliderVert::init(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t bordercolor,
                                uint16_t thumbcolor, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont) {
  _x           = x;
  _y           = y;
  _w            = w;
  _h            = h;
  _borderwidth  = borderwidth;
  _thumbcolor   = thumbcolor;
  _bordercolor  = bordercolor;
  _textcolor    = textcolor;
  _textfreefont = textfreefont;
  _textfont     = 255; // Default to 255, meaning use the FreeFont
  resetCommon();
}

void SliderVert::draw(float level, bool draw_border) {
  if (!_visible || !_enabled) return; // Do not draw if not visible
  uint16_t my_bordercolor, my_thumbcolor, my_textcolor, my_fillcolor;
  if (_active) {
    my_bordercolor = _bordercolor;
    my_thumbcolor = _thumbcolor;
    my_textcolor = _textcolor;
    my_fillcolor =  _fillcolor;
  } else {
    my_bordercolor = TFT_DARKGREY;
    my_thumbcolor = TFT_DARKGREY;
    my_textcolor = TFT_DARKGREY;
    my_fillcolor = TFT_BLACK;
  }

  int16_t bw2 = _borderwidth * 2;
  int16_t r = _w / 6; // Corner radius
	int16_t center_x = _x + (_w/2);
	int16_t center_y = _y + (_h/2);

  if ((_borderwidth > 0) && draw_border) {
    for (int i = 0; i < _borderwidth; i++) {
      _tft->drawRoundRect(_x + i, _y + i, _w - (i * 2), _h - (i * 2), r, my_bordercolor);
    }
  }

  _thumb_delta = rint(_thumb_travel * (1.0 - level)); // Calculate thumb position based on level
	// slider area color
	_tft->fillRoundRect(_x+_borderwidth, _y+_borderwidth, _w - bw2, _h - bw2, r-_borderwidth, my_fillcolor);

  int16_t knob_y = _thumb_start + _thumb_delta - _thumb_height/2; // Calculate thumb y position
  int16_t knob_center_y = knob_y + _thumb_height/2; // middle of thumb
	_tft->fillRoundRect(_thumb_x, knob_y, _thumb_width, _thumb_height, r, my_thumbcolor);

  // Riffelung
	int16_t my_thumb_x = _thumb_x + 3;
  int16_t line_len = _thumb_width - 6;
  _tft->drawFastHLine(my_thumb_x, knob_center_y - 5, line_len, my_bordercolor);
  _tft->drawFastHLine(my_thumb_x, knob_center_y, line_len, my_bordercolor);
  _tft->drawFastHLine(my_thumb_x, knob_center_y + 5, line_len, my_bordercolor);

  // Draw the slider label
  if (_label[0] == '\0') return; // Do not draw label if empty

  _tft->setTextColor(my_textcolor);
  uint8_t tempdatum = _tft->getTextDatum();
  uint16_t tempPadding = _tft->getTextPadding();
  if (_textfont == 255) {
    _tft->setFreeFont(_textfreefont); // Use FreeFont if set
  } else {
    _tft->setTextFont(_textfont);
  }
  _tft->setTextPadding(0);

	switch (_textdatum) {
	case TC_DATUM:
		_tft->setTextDatum(BC_DATUM); // Set text datum for label above slider
		_tft->drawString(_label, center_x + _xd, _y + _yd - 2);
		break;
	case BC_DATUM:
		_tft->setTextDatum(TC_DATUM); // Set text datum for label below slider
		_tft->drawString(_label, center_x + _xd, _y + _yd + _h + 2);
		break;
	case CL_DATUM:
	case BL_DATUM:
	case TL_DATUM:
		_tft->setTextDatum(_textdatum); // Set text datum for label
		_tft->drawString(_label, _x + _xd - 2, center_y + _yd);
		break;
	case CR_DATUM:
	case BR_DATUM:
	case TR_DATUM:
		_tft->setTextDatum(_textdatum); // Set text datum for label
		_tft->drawString(_label, _x + _xd + _w + 2, center_y + _yd);
		break;
	default:
		_tft->drawString(_label, _x + _xd, _y + _yd);
	}
  _tft->setTextDatum(tempdatum);
  _tft->setTextPadding(tempPadding);
}






