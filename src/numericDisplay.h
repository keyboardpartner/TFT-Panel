#ifndef NUMERICDISPLAY_H
#define NUMERICDISPLAY_H

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

// Simple NUMERIC DISPLAY

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts
#include "meterScaleDefaults.h"

#define NUM_NUMERICS 3

#define NUMERICS_HEIGHT 46
#define NUMERICS_WIDTH 140
#define NUMERICS_WIDTH_SMALL 100 // Width of numeric display without units

// #########################################################################

// Numeric display widget class
// This class provides a numeric display with a specified range and units
// It can be used to display values in a meter-like format
// The display can be updated with new values, and it supports different ranges and units
class NumericDisplay : public GUIObject
{
public:
  NumericDisplay(TFT_eSPI *tft, TouchProvider *touchProvider) : GUIObject(tft, touchProvider) { }

  void init(int x, int y, uint16_t color = TFT_DARKGREEN, bool draw_units = true) {
    _currstate = false;
    _laststate = false;
    _w = NUMERICS_WIDTH - 1;
    _h = NUMERICS_HEIGHT - 1;
    _x = x;
    _y = y;
    _textcolor = TFT_BLACK;
    _bordercolor = TFT_GREY;
    _fillcolor = TFT_LIGHTGREY;
    _levelIntegrator = 0.0f;
    _draw_units = draw_units; // Store whether to draw units or not
    if (draw_units)
      _w = NUMERICS_WIDTH - 1;
    else
      _w = NUMERICS_WIDTH_SMALL - 1;
    _h = NUMERICS_HEIGHT - 1;
    _enabled = true;   // enabled by default
    _active = true;  // active by default
    _visible = true; // visible by default
    _textfont = 255;   // default to FreeFont
    _textfreefont = FF23;
  }

  void setRangeIdxColor(int range_idx, uint16_t color = TFT_BLACK) {
    _textcolor = color;
    _range_idx = range_idx;
    _maxVal = meterScaleMaxVal[range_idx];
    _valdecimals = meterValDecimals[range_idx];
    setLevel(0.0f, true); // Reset level and redraw
  }

  // Update the numeric display with a new level
  void setLevel(float level, bool full_redraw = false) {
    if (!_visible || !_enabled) return; // Do not draw if not visible or not enabled
    uint16_t my_bordercolor = _bordercolor;
    uint16_t my_textcolor = _textcolor;
    uint16_t my_fillcolor = _fillcolor;
    if (!_active) {
      my_bordercolor = TFT_DARKGREY; // Inactive display, use dark grey bezel color
      my_textcolor = TFT_GREY;       // Inactive display, use light grey text color
      my_fillcolor = TFT_BLACK;      // Inactive display, use black scale color
    }
    if (_textfont == 255)
      _tft->setFreeFont(_textfreefont);
    else
      _tft->setTextFont(_textfont);
    if (full_redraw) {
      for (int i = 0; i < 3; i++)
        _tft->drawRect(_x + i, _y + i, _w - 2 * i, _h - 2 * i, my_bordercolor);
      _tft->drawRect(_x + 3, _y + 3, _w - 6, _h - 6, TFT_DARKGREY);
      _tft->fillRect(_x + 4, _y + 4, _w - 8, _h - 8, my_fillcolor);
      if (_draw_units) {
        _tft->setTextDatum(TR_DATUM);
        _tft->setTextColor(my_textcolor, my_fillcolor);
        _tft->drawString(meterScaleUnits[_range_idx], _x + _w - 10, _y + 8);
        _tft->setTextDatum(TL_DATUM);
      }
    }
    // _tft->setTextFont(7); // Seven-segment font
    int str_len = strlen(meterScaleUnits[_range_idx]); // get number of chars to display
    float last_level = _levelIntegrator;
    _levelIntegrator = level * LVLINTEGRATOR + _levelIntegrator * (1 - LVLINTEGRATOR);
    if (full_redraw || (_levelIntegrator > last_level + 0.001) || (_levelIntegrator < last_level - 0.001))    {
      _tft->setTextColor(my_textcolor, my_fillcolor);
      if (str_len < 2) // If single letter unit, assume higher resolution and wider value to display
        _tft->setTextPadding(85);
      else
        _tft->setTextPadding(65);
      float scaled_level = _levelIntegrator * _maxVal;
      if (scaled_level > 999.9)
        scaled_level = 999.9;
      _tft->drawFloat(scaled_level, _valdecimals, _x + 8, _y + 8);
      _tft->setTextPadding(0);
    }
  }

  // Redraw of object, will be called when the object needs to be redrawn without level change
  void redraw(bool active) override {
    _active = active; // Set active state
    setLevel(_level, true);
  }

  // set numeric field active (or inactive) and redraw (or not)
  void setActive(bool active, bool redraw = false) override {
    _active = active;
    if (redraw)
      setLevel(_level, redraw);
  }

  // #########################################################################

  bool checkPressed(bool wait_released) override {
    // Simple proc to check if the numeric field is pressed, waits for release if wait_released is true
    // Result will be set true if there is a valid touch on the screen
    _currstate = false;
    if (!_enabled || !_active)
      return false; // Do not check if not enabled
    bool was_just_pressed = false;
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      _currstate = true;
      if (_laststate != _currstate) {
        was_just_pressed = true; // Button was pressed
        _pressAction();          // Call the press action callback if defined
        delay(20); // debounce delay
        // If the switch is pressed, wait until the touch is released
        if (wait_released) {
          _touchProvider->waitReleased();
          _currstate = false; // Reset current state to false after release
        }
      }
    }
    _laststate = _currstate;
    return was_just_pressed; // Return the current state of the numeric field
  }

private:
  int _range_idx;
  int _valdecimals;
  float _maxVal, _levelIntegrator;
  bool _draw_units; // Whether to draw units or not
  bool _level;        // Button states
};

#endif