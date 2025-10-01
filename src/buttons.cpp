#include "buttons.h"


/***************************************************************************************
** Code for the GFX button UI element
** Grabbed from Adafruit_tft library and enhanced to handle any label font
** Additions by cm to set button active or inactive
***************************************************************************************/

// Classic init() function: pass center & size
void PushButton::initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline,
                              uint16_t fill, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont) {
  // Tweak arguments and pass to the newer init() function...
  init(x - (w / 2), y - (h / 2), w, h, outline, fill, textcolor, borderwidth, textfreefont);
}

// Overloaded function to use a text font instead of a FreeFont
void PushButton::initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline,
                              uint16_t fill, uint16_t textcolor, uint16_t borderwidth, u_int8_t textfont) {
  // Tweak arguments and pass to the newer init() function...
  init(x - (w / 2), y - (h / 2), w, h, outline, fill, textcolor, borderwidth, textfont);
}

// Overloaded function for invisible buttons, just set position and size
void PushButton::initCenter(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  // Tweak arguments and pass to the newer init() function...
  init(x - (w / 2), y - (h / 2), w, h);
}

// Overloaded function for invisible buttons
void PushButton::init(int16_t x, int16_t y, uint16_t w, uint16_t h) {
  // Tweak arguments and pass to the newer init() function...
  _x           = x;
  _y           = y;
  _xd           = 0;
  _yd           = 0;
  _w            = w;
  _h            = h;
  _bordercolor = 0;
  _borderwidth  = 0;
  _fillcolor    = 0;
  _textdatum    = MC_DATUM;
  _textcolor    = 0;
  _textfreefont = nullptr; // No FreeFont used
  _textfont     = 0;
  _currstate  = false;
  _laststate  = false;
  _inverted   = false;
  _label[0] = '\0'; // Initialize label to empty string
  _enabled = true;    // alive by default
  _active = true;  // active by default
  _visible = false;  // invisible by default
}

// Newer function instead accepts upper-left corner & size
void PushButton::init(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor,
                        uint16_t fill, uint16_t textcolor, uint16_t borderwidth, const GFXfont *textfreefont) {
  _x           = x1;
  _y           = y1;
  _xd           = 0;
  _yd           = -2;
  _w            = w;
  _h            = h;
  _bordercolor  = bordercolor;
  _borderwidth  = borderwidth;
  _fillcolor    = fill;
  _textdatum    = MC_DATUM;
  _textcolor    = textcolor;
  _textfreefont = textfreefont;
  _textfont     = 255; // Default to 255, meaning use the FreeFont
  _currstate  = false;
  _laststate  = false;
  _inverted   = false;
  _label[0] = '\0'; // Initialize label to empty string
  _enabled = true;    // alive by default
  _active = true;  // active by default
  _visible = true;  // visible by default
}

// Overloaded function to use a text font instead of a FreeFont
void PushButton::init(int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t bordercolor,
                                uint16_t fill, uint16_t textcolor, uint16_t borderwidth, u_int8_t textfont) {
  _x           = x1;
  _y           = y1;
  _xd           = 0;
  _yd           = 1;
  _w            = w;
  _h            = h;
  _bordercolor  = bordercolor;
  _borderwidth  = borderwidth;
  _fillcolor    = fill;
  _textdatum    = MC_DATUM;
  _textcolor    = textcolor;
  _textfreefont = nullptr; // No FreeFont used
  _textfont     = textfont;
  _currstate  = false;
  _laststate  = false;
  _inverted   = false;
  _label[0] = '\0'; // Initialize label to empty string
  _enabled = true;    // alive by default
  _active = true;  // active by default
  _visible = true;  // visible by default
}


void PushButton::draw(bool inverted) {
  if (!_visible || !_enabled) return; // Do not draw if not visible or not alive
  uint16_t fill, outline, text, border;
  border = _borderwidth;
  if (border < 0) border = 2;

  _inverted  = inverted;

  if(_active) {
    if(!inverted) {
      fill    = _fillcolor;
      outline = _bordercolor;
      text    = _textcolor;
    } else {
      fill    = _textcolor;
      outline = _bordercolor;
      text    = _fillcolor;
    }
  } else {
    fill    = TFT_BLACK;
    outline = TFT_DARKGREY;
    text    = TFT_DARKGREY;
  }

  uint8_t r = min(_w, _h) / 4; // Corner radius
  if (border > 0)
    _tft->fillRoundRect(_x, _y, _w, _h, r, outline);
  _tft->fillRoundRect(_x+border, _y+border, _w-(2*border), _h-(2*border), r-border, fill);
  _tft->setTextColor(text, fill);
  _tft->setTextSize(1);
  uint8_t tempdatum = _tft->getTextDatum();
  _tft->setTextDatum(_textdatum);

  if (_textfont == 255) {
    _tft->setFreeFont(_textfreefont); // Use FreeFont if set
  } else {
    _tft->setTextFont(_textfont);
  }
  uint16_t tempPadding = _tft->getTextPadding();
  _tft->setTextPadding(0);
  _tft->drawString(_label, _x + (_w/2) + _xd, _y + (_h/2) + _yd);
  _tft->setTextDatum(tempdatum);
  _tft->setTextPadding(tempPadding);
}

