
/***************************************************************************************
** Code for the GFX Checkbox UI element
** Grabbed from Adafruit_tft library and enhanced to handle any label font
** Additions by cm to set button active or inactive
***************************************************************************************/

#include "checkBox.h"


// Classic init() function: pass center & size
void Checkbox::initCenter(int16_t x, int16_t y, uint16_t size, uint16_t bordercolor, uint16_t bgcolor, uint16_t checkedcolor,
                              uint16_t borderwidth, uint8_t textfont)
{
  // Tweak arguments and pass to the newer init(<x1>, <y1>) function...
  init(x - (size / 2), y - (size / 2), size, bordercolor, bgcolor, checkedcolor, borderwidth, textfont);
}

// Overloaded function to use a FreeFont instead of a text font
void Checkbox::initCenter(int16_t x, int16_t y, uint16_t size, uint16_t bordercolor, uint16_t bgcolor,
                              uint16_t checkedcolor, uint16_t borderwidth, const GFXfont *textfreefont)
{
  // Tweak arguments and pass to the newer init() function...
  init(x - (size / 2), y - (size / 2), size, bordercolor, bgcolor, checkedcolor, borderwidth, textfreefont);
}

// Newer function instead accepts upper-left corner & size
void Checkbox::init(int16_t x1, int16_t y1, uint16_t size, uint16_t bordercolor, uint16_t bgcolor, uint16_t checkedcolor,
                                uint16_t borderwidth, uint8_t textfont) {
  _x           = x1;
  _y           = y1;
  _xd          = 0;
  _yd          = 0;
  _size        = size;
  _h           = size;
  _w           = size;
  _borderwidth = borderwidth;
  _bordercolor = bordercolor;
  _bgcolor     = bgcolor;
  _checkedcolor = _tft->alphaBlend(200, checkedcolor, TFT_BLACK);
  _borderwidth  = borderwidth;
  _textdatum    = ML_DATUM;
  _textcolor    = bordercolor;
  _textfont     = textfont;
  _textfreefont = NULL;
  _checked      = false;
  _currstate    = false;
  _laststate    = false;
  _enabled = true;    // alive by default
  _active = true;  // active by default
  _visible = true;  // visible by default
}

// Overloaded function to use a FreeFont instead of a text font
void Checkbox::init(int16_t x1, int16_t y1, uint16_t size, uint16_t bordercolor, uint16_t bgcolor,
                                uint16_t checkedcolor, uint16_t borderwidth, const GFXfont *textfreefont) {
  _x           = x1;
  _y           = y1;
  _xd          = 0;
  _yd          = 0;
  _size        = size;
  _h           = size;
  _w           = size;
  _borderwidth = borderwidth;
  _bordercolor = bordercolor;
  _bgcolor     = bgcolor;
  _borderwidth  = borderwidth;
  _textdatum = ML_DATUM;
  _textcolor    = bordercolor;
  _checkedcolor = _tft->alphaBlend(200, checkedcolor, TFT_BLACK);
  _textfreefont = textfreefont;
  _textfont     = 255; // Default to 255, meaning use the FreeFont
  _checked      = false;
  _currstate    = false;
  _laststate    = false;
  _enabled = true;    // alive by default
  _active = true;  // active by default
  _visible = true;  // visible by default
}

void Checkbox::draw(bool checked) {
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
  // Adjust text x position based on the text datum
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

  _tft->setTextColor(my_textcolor, my_bgcolor);

 	_tft->fillRect(_x, _y, _size, _size, my_bgcolor);
	for (int i = 0; i < _borderwidth; i++) {
		_tft->drawRect(_x + i, _y + i, _size - i*2, _size - i*2, my_bordercolor);
	}

	// Draw the checkmark if the Checkbox is checked
	if (_checked) {
 	  _tft->fillRect(_x + _borderwidth + 2, _y + _borderwidth + 2, _size - bw2 - 4, _size - bw2 - 4, my_checkcolor);
    _tft->drawWideLine(_x + _borderwidth + 2, _y + _borderwidth + 2, _x + _size - bw2 - 1, _y + _size - bw2 - 1, _borderwidth, my_bordercolor, my_bgcolor);
    _tft->drawWideLine(_x + _borderwidth + 2, _y + _size - bw2 - 1, _x + _size - bw2 - 1, _y + _borderwidth + 2, _borderwidth, my_bordercolor, my_bgcolor);

	} else {
		// Draw the box if not checked
		_tft->fillRect(_x + _borderwidth + 2, _y + _borderwidth + 2, _size - bw2 - 4, _size - bw2 - 4, _tft->alphaBlend(150, my_checkcolor, TFT_BLACK));
	}
	// Draw the checkbox label
  uint8_t tempdatum = _tft->getTextDatum();
  uint16_t tempPadding = _tft->getTextPadding();
  _tft->setTextDatum(_textdatum);
  if (_textfont == 255) {
    _tft->setFreeFont(_textfreefont); // Use FreeFont if set
  } else {
    _tft->setTextFont(_textfont);
  }
  int size_4 = _size + _size / 4;
  // Set size for inherited contains() function
  if (_textfont == 255) {
    _w = size_4 + _tft->textWidth(_label);
  } else {
    _w = size_4 + _tft->textWidth(_label, _textfont);
  }
  _tft->setTextPadding(0);
  _tft->drawString(_label, text_x, _y + (_size/2) + _yd + 2);
  _tft->setTextDatum(tempdatum);
  _tft->setTextPadding(tempPadding);
}




