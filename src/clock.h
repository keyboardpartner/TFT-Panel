#ifndef ANALOGCLOCK_H
#define ANALOGCLOCK_H

/*
// #############################################################################
//       __ ________  _____  ____  ___   ___  ___
//      / //_/ __/\ \/ / _ )/ __ \/ _ | / _ \/ _ \
//     / ,< / _/   \  / _  / /_/ / __ |/ , _/ // /
//    /_/|_/___/_  /_/____/\____/_/_|_/_/|_/____/
//      / _ \/ _ | / _ \/_  __/ |/ / __/ _ \
//     / ___/ __ |/ , _/ / / /    / _// , _/
//    /_/  /_/ |_/_/|_| /_/ /_/|_/___/_/|_|
//
// #############################################################################
*/
// Wetterstation für TFT 1.8" an ESP32 by cm@make-magazin.de, Make 2/2023

//Standard support
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all Hors
#include "guiObject.h" // Common GUI object for all Hors
#include "Free_Fonts.h" // Include large fonts

int dayspermonth_arr[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
String day_names[7] = {"Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam"};

// -------------------------------------------------------------------------------

// Wetterdaten von https://randomnerdtutorials.com/esp8266-weather-forecaster/

// Set of tabs on bottom of panel area with a common area for tab content
class AnalogClock : public GUIObject {

public:

  AnalogClock(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  // New/alt init() uses upper-left corner & height
  void init(int16_t x, int16_t y, int16_t size, uint16_t bordercolor, uint16_t handscolor,
							uint16_t tickscolor, uint16_t textcolor, uint16_t bgcolor, int8_t textfont = 2) {
		_x            = x;
		_y            = y;
		_size         = size;
    _bgcolor      = bgcolor;
		_bordercolor  = bordercolor;
		_tickscolor   = tickscolor;
		_handscolor   = handscolor;
    _center_x     = x + _size / 2;
    _center_y     = y + _size / 2;
    _radius       = (size - 2) / 2;
    _outer_radius = (float)_radius - 2;
		_textcolor    = textcolor;
		_textfont     = textfont;
  	_textfreefont = NULL;
		_enabled = true;    // enabled by default
		_active = true;  // active by default
		_visible = true;  // visible by default
    for (int z = 0; z < 360; z += 6) {
      // Begin at 0° and stop at 360° in minute/second ticks
      // Convert degrees to radians
      _sin_table[z / 6] = sin(z / 57.296); // Store sine values for each second
    }
  }

  // Overloaded function to use a FreeFont instead of a text font
  void init(int16_t x, int16_t y, int16_t size, uint16_t bordercolor, uint16_t handscolor,
							uint16_t tickscolor, uint16_t textcolor, uint16_t bgcolor, const GFXfont *textfreefont) {
		_x            = x;
		_y            = y;
		_size         = size;
    _bgcolor      = bgcolor;
		_bordercolor  = bordercolor;
		_tickscolor   = tickscolor;
		_handscolor   = handscolor;
    _center_x     = x + _size / 2;
    _center_y     = y + _size / 2;
    _radius       = (size - 2) / 2;
    _outer_radius = (float)_radius - 2;
		_textcolor    = textcolor;
		_textfont     = 255;// Default to 255, meaning use the FreeFont
		_textfreefont = textfreefont;
		_enabled = true;    // enabled by default
		_active = true;  // active by default
		_visible = true;  // visible by default
    for (int z = 0; z < 360; z += 6) {
      // Begin at 0° and stop at 360° in minute/second ticks
      // Convert degrees to radians
      _sin_table[z / 6] = sin(z / 57.296); // Store sine values for each second
    }
	}


  void drawHourHand(int hour60, uint16_t color_mh) {
    // clock hours hand, 60 divs resolution
    int x1, x2, x3, y1, y2, y3;
    uint16_t dimhandscolor = _tft->alphaBlend(128, color_mh, _bgcolor);
    float hand_length = _outer_radius * 0.65;
    float hand_width = _outer_radius / 9;
    x3 = _center_x + (get_sin(hour60) * hand_length); // Endpunkt Zeiger
    y3 = _center_y - (get_cos(hour60) * hand_length);
    //_tft->fillTriangle(x1, y1, x2, y2, x3, y3, dimhandscolor);
    // Draw an anti-aliased wide line from ax,ay to bx,by with different width at each end aw, bw and with radiused ends
    // If bg_color is not included the background pixel colour will be read from TFT or sprite
    _tft->drawWedgeLine(_center_x, _center_y, x3, y3, hand_width, 1, dimhandscolor);
    _tft->drawWideLine(_center_x, _center_y, x3, y3, 2, color_mh, _bgcolor);
  }

  void drawMinuteHand(int min60, uint16_t color_mh) {
    // clock minute hand
    int x1, x2, x3, y1, y2, y3;
    uint16_t dimhandscolor = _tft->alphaBlend(128, color_mh, _bgcolor);
    float hand_length = _outer_radius * 0.9;
    float hand_width = _outer_radius / 12.5;
    x3 = _center_x + (get_sin(min60) * hand_length); // Endpunkt Zeiger
    y3 = _center_y - (get_cos(min60) * hand_length);
    //_tft->fillTriangle(x1, y1, x2, y2, x3, y3, dimhandscolor);
    _tft->drawWedgeLine(_center_x, _center_y, x3, y3, hand_width, 1, dimhandscolor);
    _tft->drawWideLine(_center_x, _center_y, x3, y3, 2, color_mh, _bgcolor);
  }

  void drawSecondHand(int sec60, uint16_t color_sec) {
    // clock seconds hand
    int x1, x2, y1, y2;
    float hand_length = _outer_radius * 0.95;
    float hand_width = _outer_radius / 5; // here: opposite direction
    x1 = _center_x + (get_sin(sec60 - 30) * hand_width);
    y1 = _center_y - (get_cos(sec60 - 30) * hand_width);
    x2 = _center_x + (get_sin(sec60) * hand_length);
    y2 = _center_y - (get_cos(sec60) * hand_length);
    _tft->drawWideLine(x1, y1, x2, y2, 2, color_sec, _bgcolor);
    _tft->fillCircle(x1, y1, hand_width / 3, color_sec);
  }

  void drawHands(int hour60, int min, int sec, uint16_t color_mh, uint16_t color_sec) {
    // Draw hour hand
    drawHourHand(hour60, color_mh);
    // Draw minute hand
    drawMinuteHand(min, color_mh);
    // Draw second hand
    _tft->fillCircle(_center_x, _center_y, _radius / 10, color_mh);
    drawSecondHand(sec, color_sec);
  }



  void draw(int hour, int min, int sec, bool redraw)  {
    if (!_visible || !_enabled) return; // Do not draw if not visible or not enabled
    hour = hour % 12; // Convert hour to 12-hour format

    uint16_t my_textcolor = _textcolor;
    uint16_t my_handscolor = _handscolor;
    uint16_t my_facecolor = _bordercolor;
    uint16_t my_tickscolor = _tickscolor;
    uint16_t my_bgcolor = _bgcolor;

    if (!_active) {
      my_textcolor = TFT_DARKGREY;
      my_handscolor = TFT_GREY;
      my_facecolor = TFT_GREY;
      my_tickscolor = TFT_GREY;
      my_bgcolor = TFT_BLACK;
    }

    if (redraw) {
      _tft->drawCircle(_center_x, _center_y, _radius, my_facecolor);
      _tft->drawCircle(_center_x, _center_y, _radius - 1, _tft->alphaBlend(128, my_facecolor, my_bgcolor));
      _tft->fillCircle(_center_x, _center_y, _radius - 2, my_bgcolor);
    }

    if (redraw || sec != _sec_old || min != _min_old) {
      int hour60 = hour * 5 + min / 12; // Convert hour to minute scale
      // erase hands to background if changed
      if ((hour60 != _hour60_old) || redraw) {
        drawHourHand(_hour60_old, my_bgcolor); // Print time in HH:MM:SS format
        _hour60_old = hour60;
      }
      if ((min != _min_old) || redraw) {
        drawMinuteHand(_min_old, my_bgcolor); // Print time in HH:MM:SS format
        _min_old = min;
      }
      if ((sec != _sec_old) || redraw) {
        drawSecondHand(_sec_old, my_bgcolor); // Print time in HH:MM:SS format
        _sec_old = sec;
      }
      if (redraw || sec % 5 == 1 || (min % 5 == 1 && sec % 5 == 0)) {
        // redraw clock face, might have been destroyed by erasing old hands
        int x1, x2, y1, y2;
        // clock face hour ticks
        float outer_radius = _outer_radius - 2;
        float inner_radius = _outer_radius * 0.87;
        float tick_radius;
        for (int z = 0; z < 60; z += 5) {
          if (z % 15 == 0) {
            tick_radius = inner_radius * 0.87; // Longer ticks for hour marks
          } else {
            tick_radius = inner_radius;
          }
          x1 = (_center_x + (get_sin(z) * outer_radius));
          y1 = (_center_y - (get_cos(z) * outer_radius));
          x2 = (_center_x + (get_sin(z) * tick_radius));
          y2 = (_center_y - (get_cos(z) * tick_radius));
          if (min == z || sec == z) {
            _tft->drawWideLine(x1, y1, x1, y1, 2, my_tickscolor, my_bgcolor); // draw dot only if hands match tick
          } else  {
            _tft->drawWideLine(x1, y1, x2, y2, 2, my_tickscolor, my_bgcolor);
          }
        }
      }
      drawHands(hour60, min, sec, my_handscolor, TFT_RED); // draw new hands
    }
  /*
    // now add temperature if needed
    _tft->setCursor(88, 56); // - 6 = Baseline
    _tft->print(tempNow, 1);
    // _tft->write(0x7F);
    _tft->setCursor(120, 56); // Baseline
    _tft->print("C");
    _tft->setCursor(114, 52); // Baseline
    _tft->print("o");
    yield();
  */
  }

  void update(int hour, int min, int sec, bool redraw)  {
    if (!_visible || !_enabled) return; // Do not draw if not visible or not enabled
    if (redraw) {
      _tft->fillCircle(_center_x, _center_y, _radius - 1, _bgcolor);
    }
    if (_sec_old != sec || redraw) {
      draw(hour, min, sec, redraw); // draw new hands
    }
  }

  // Overload with timeinfo
  void update(struct tm *timeinfo, bool redraw)  {
    if (!_visible || !_enabled) return; // Do not draw if not visible or not enabled
    if (_sec_old != timeinfo->tm_sec || redraw)
      draw(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, redraw); // draw new hands
  }

  // Redraw of object, will be called when the object needs to be redrawn without state change
  void redraw(bool active) override {
    _active = active; // Set active state
    update(_hour_old, _min_old, _sec_old, true); // draw object
  }

  // Active state, object responds to user input when active; may be visible or not visible, though
  // set switch active (or inactive) and redraw (or not), overload with redraw
  void setActive(bool active, bool redraw = false) {
    _active = active;
    if (redraw)
      update(_hour_old, _min_old, _sec_old, true);
  }

	// Enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
	// overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      update(_hour_old, _min_old, _sec_old, true);
  }

  // -------------------------------------------------------------------------------

private:
  uint16_t  _handscolor, _tickscolor; // Selected tab index
  int _min_old = 0, _sec_old = 0, _hour_old = 0, _hour60_old = 0;
  int _min = 0, _sec = 0, _hour = 0;
  float _sin_table[60];
  float get_sin(int idx) {
    if (idx < 0) idx += 60; // Ensure positive index
    if (idx >= 60) idx -= 60; // Ensure index is within bounds
    return _sin_table[idx];
  }

  float get_cos(int idx) {
    idx += 15;
    if (idx < 0) idx += 60; // Ensure positive index
    if (idx >= 60) idx -= 60; // Ensure index is within bounds
    return _sin_table[idx];
  }

  float _outer_radius;

};

#endif