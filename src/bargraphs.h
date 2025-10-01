#ifndef BARGRAPH_HORIZONTAL_WIDGET_H
#define BARGRAPH_HORIZONTAL_WIDGET_H

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

/*
Horizontal bargraph display by cm 08/2025

New approach to ensure less drawing, only changed areas are redrawn

Make sure all the display driver and pin connections are correct by
editing the User_Setup.h file in the TFT_eSPI library folder.

#########################################################################
###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
#########################################################################

https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h

*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts
#include "meterScaleDefaults.h"

#define TICK_COUNT_BG 16      // 16 fine ticks for bargraph


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


class HorizontalBargraph {
public:
  HorizontalBargraph(TFT_eSPI *tftptr) {
    _tft = tftptr;
    _touchProvider = NULL; // No touch provider by default
    bargraph.touchEnabled = false; // Default no touch enabled
  }

  // Overload with touch provider
  HorizontalBargraph(TFT_eSPI *tftptr, TouchProvider *touchProvider) {
    _tft = tftptr;
    _touchProvider = touchProvider;
    bargraph.touchEnabled = true; // Default touch enabled
  }

  // Initialize the bargraph
  // Bargraph scale parameters are defined in "meterScaleDefaults.h"
  // setRangeIdxColor() must follow to set the color for the current range and draw the meter
  void init(int x, int y, int width, int height) {
    bargraph.x = x;
    bargraph.y = y;
    bargraph.width = width;
    bargraph.height = height;
    bargraph.textColor = TFT_BLACK;
    bargraph.bezelColor = TFT_GREY;
    bargraph.scaleColor = TFT_LIGHTGREY;

    bargraph.barXstart = x + 8;       // calculate bar area
    bargraph.barYstart = y + 8;
    bargraph.barWidth = width - 16;
    bargraph.barLength = bargraph.barWidth;
    bargraph.barXend = bargraph.barXstart + bargraph.barWidth - 1; // horizontal orientation
    bargraph.lastBarPos = 0;
    bargraph.lastPeakPos = bargraph.barLength - 2;
    bargraph.levelMark = -1.0;  // enforce full redraw after init

    _enabled = true; // Bargraph is enabled by default
    _visible = true; // Bargraph is visible by default
    _active = true; // Bargraph is active by default
    _baseline_y = y + height / 2;  // Vertical center of Bargraph
    bargraph.barHeight = _baseline_y - bargraph.barYstart - 1;
    bargraph.barYend = bargraph.barYstart + bargraph.barHeight - 1;
  }

  void drawFrame() {
    // Draw the frame and scale around the bar graph
    if (!_visible || !_enabled) return; // Do not draw if not visible
    float tick_p = (float)bargraph.barXstart - 1.0;
    float tick_end = (float)bargraph.barXend;
    float tick_inc = (tick_end - tick_p) / TICK_COUNT_BG;
    int tl = bargraph.barHeight  / 2;

    for (int i = 0; i < 3; i++)
        _tft->drawRect(bargraph.x + i, bargraph.y + i, bargraph.width - 2 * i, bargraph.height - 2 * i, bargraph.bezelColor);
    _tft->drawRect(bargraph.x + 3, bargraph.y + 3, bargraph.width - 6, bargraph.height - 6, TFT_DARKGREY);
    _tft->fillRect(bargraph.x + 4, bargraph.y + 4, bargraph.width - 8, bargraph.height - 8, bargraph.scaleColor);

    _tft->drawRect(bargraph.barXstart - 1, bargraph.barYstart - 1, bargraph.barWidth + 2, bargraph.barHeight + 2, bargraph.textColor);
    _tft->fillRectVGradient(bargraph.barXstart, bargraph.barYstart, bargraph.barWidth, bargraph.barHeight, bargraph.scaleColor, bargraph.scaleGradientColor);

    _tft->setTextDatum(ML_DATUM);
    float mult = 0.0;
    int idx = 0;
    _tft->setTextColor(bargraph.textColor, bargraph.scaleColor);
    _tft->setTextSize(1);
    int y1 = _baseline_y + tl + 2;
    while (tick_p <= tick_end) {
        int x1_tick = rint(tick_p);
        _tft->drawFastVLine(x1_tick, _baseline_y, tl, bargraph.textColor);
        if (idx % 4 == 0) {
            _tft->setTextFont(2);
            _tft->drawFastVLine(x1_tick + 1, _baseline_y, tl, bargraph.textColor);
            if (idx == 0) {
                if (bargraph.height > 50) {
                    _tft->setTextDatum(BL_DATUM);
                    _tft->setFreeFont(FF21);
                    _tft->drawString(meterScaleUnits[bargraph.range_idx], x1_tick, bargraph.y + bargraph.height - 5);
                } else {
                    _tft->setTextDatum(TL_DATUM);
                    _tft->drawString(meterScaleUnits[bargraph.range_idx], x1_tick, y1, 1);
                }
            } else if (idx == 16) {
                _tft->setTextDatum(TR_DATUM);
                _tft->drawString(String(bargraph.maxVal * mult, bargraph.scaleDecimals), x1_tick, y1, 1);
            } else {
                _tft->setTextDatum(TC_DATUM);
                _tft->drawString(String(bargraph.maxVal * mult, bargraph.scaleDecimals), x1_tick, y1, 1);
            }
            mult += 0.25;
        }
        tick_p += tick_inc;
        idx++;
    }
    _tft->setTextDatum(TL_DATUM);
    _tft->setTextFont(2);
  }

  // Sets the color for the meter needle and the range index
  // The range index determines the maximum value and the number of decimals for the display
  // Redraws the meter frame with the new settings
  void setRangeIdxColor(int range_idx, uint16_t color = TFT_GREEN, bool peak_tracking = false) {
    bargraph.needleColor = color;
    bargraph.maxVal = meterScaleMaxVal[range_idx];
    bargraph.valDecimals = meterValDecimals[range_idx];
    bargraph.scaleDecimals = meterScaleDecimals[range_idx];
    bargraph.gradientColor = _tft->alphaBlend(100, color, TFT_BLACK);
    bargraph.scaleGradientColor = _tft->alphaBlend(160, bargraph.scaleColor, TFT_BLACK);
    bargraph.range_idx = range_idx;
    bargraph.peakTracking = peak_tracking;
    drawFrame();
    setLevel(bargraph.levelIntegrator, true);
  }

  void setLevel(float level, bool full_redraw = false) {
    if (full_redraw) {
      drawFrame();
    }
    update(level, bargraph.levelMark, full_redraw);
  }

  void setLevelMarker(float marker_level, bool redraw = false) {
    bargraph.levelMark = marker_level;
    if (_visible && _enabled && redraw)
      update(bargraph.levelIntegrator, bargraph.levelMark, redraw);
  }

  // Set level (or inactive) and redraw (or not), overload with redraw
  float getLevelMarker() const {
    return bargraph.levelMark; // Return the set mark value
  }

  // #########################################################################

  // Update the bar graph with new values. It will only redraw partial bars needed
  // if the value has changed significantly (visible change) to avoid flicker.
  // A full redraw can be forced by setting full_redraw to true or changing levelMark indicator value.
  // Level and levelMark range from 0 to 1.0 (float) with 1.0 = full deflection.
  // The levelMark triangle indicator is only draw if levelMark >= 0.0.

  void update(float level, float levelMark, bool full_redraw = false) {
    if (!_visible || !_enabled) return; // Do not draw if not visible
    if (level > 1.0) level = 1.0;
    if (level < 0.0) level = 0.0;
    float level_integrator = bargraph.levelIntegrator;
    level_integrator = level * LVLINTEGRATOR + level_integrator * (1 - LVLINTEGRATOR);
    bargraph.levelIntegrator = level_integrator;
    int new_length = (int)((float)(bargraph.barLength - 1) * level_integrator);
    float peak_integrator = bargraph.peakIntegrator;
    if (bargraph.peakTracking) {
      if (level > peak_integrator) {
        peak_integrator = level;
        bargraph.peakIntegrator = level;
      } else {
        #ifdef LINEAR_PEAK_DECAY
          peak_integrator -= PEAK_DECAY;
        #else
          peak_integrator = (level * PEAK_DECAY + peak_integrator * (1 - PEAK_DECAY)) - 0.001;
        #endif
        if (peak_integrator < 0) peak_integrator = 0;
        bargraph.peakIntegrator = peak_integrator;
      }
    } else {
      peak_integrator = level;
    }

    int peak_pos = (int)((float)(bargraph.barLength - 1) * peak_integrator);

    if (peak_pos >= bargraph.barLength - 3) peak_pos = bargraph.barLength - 3; // ensure peak_pos is lower than max
    if (new_length >= bargraph.barLength - 2) new_length = bargraph.barLength - 2; // ensure new_length is lower than max
    if (peak_pos < new_length) peak_pos = new_length; // ensure peak_pos is at least new_length
    if (bargraph.lastPeakPos < new_length)  bargraph.lastPeakPos = new_length; // ensure erase will occur not in bar

    if (bargraph.levelMark != levelMark || full_redraw) {
      // enforce full redraw
      bargraph.levelMark = levelMark;
      bargraph.lastBarPos = 0;
      bargraph.lastPeakPos = bargraph.barLength - 3;
      _tft->fillRectVGradient(bargraph.barXstart, bargraph.barYstart, bargraph.barWidth, bargraph.barHeight, bargraph.scaleColor, bargraph.scaleGradientColor);
    }
    int old_length = bargraph.lastBarPos;
    if ((new_length == old_length) && (peak_pos == bargraph.lastPeakPos)) {
      return;
    }
    // erase area if the bar length has changed
    int16_t new_x = bargraph.barXstart + new_length;
    int16_t diff_x = new_length - old_length; // calculate area to erase
    // draw new value bar
    if (diff_x > 0) {
      // draw colored value bar
      _tft->fillRectVGradient(bargraph.barXstart + old_length, bargraph.barYstart, diff_x, bargraph.barHeight, bargraph.needleColor, bargraph.gradientColor);
    }
    _tft->drawFastVLine(new_x, bargraph.barYstart, bargraph.barHeight, bargraph.textColor); // draw bar's end line
    if (diff_x < 0) {
      // fill rest of bar with background
      _tft->fillRectVGradient(new_x + 1, bargraph.barYstart, -diff_x, bargraph.barHeight, bargraph.scaleColor, bargraph.scaleGradientColor);
    }

    if (bargraph.peakTracking && (peak_pos > 0)) {
      // erase old peak indicator with gradient
      _tft->fillRectVGradient(bargraph.barXstart + bargraph.lastPeakPos, bargraph.barYstart, 2, bargraph.barHeight, bargraph.scaleColor, bargraph.scaleGradientColor);
      // draw new peak indicator
      _tft->drawFastVLine(bargraph.barXstart + peak_pos, bargraph.barYstart, bargraph.barHeight, TFT_RED);
      _tft->drawFastVLine(bargraph.barXstart + peak_pos + 1, bargraph.barYstart, bargraph.barHeight, TFT_RED);
    }

    if (levelMark >= 0.0) {
      // draw set value indicator
      int sv_indicator = (int)((float)(bargraph.barLength) * levelMark) + bargraph.barXstart;
      _tft->fillTriangle(sv_indicator, bargraph.barYstart + bargraph.barHeight - 4, sv_indicator - 4, bargraph.barYstart + 6, sv_indicator + 3, bargraph.barYstart + 6, TFT_BLACK);
      _tft->drawTriangle(sv_indicator, bargraph.barYstart + bargraph.barHeight - 3, sv_indicator - 5, bargraph.barYstart + 5, sv_indicator + 4, bargraph.barYstart + 5, TFT_WHITE);
    }
    bargraph.lastBarPos = new_length;
    bargraph.lastPeakPos = peak_pos;
  }

  // #########################################################################

  // Checks if coordinates are within the object boundaries
  bool contains(int16_t x, int16_t y) const {
    return ((x >= bargraph.barXstart) && (x <= bargraph.barXend) &&
            (y >= bargraph.barYstart) && (y <= bargraph.barYend));
  }

	// Enabled state overrides Active and Visible states.
	// Object will be ignored and not drawn if not enabled
  // Overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      update(bargraph.levelIntegrator, bargraph.levelMark);
  }
	// enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
	bool getEnabled(void) const { return _enabled; }
	bool isEnabled(void) const { return _enabled; }

  // Check if the bargraph is pressed
  // If the bargraph is pressed, it will enter a modal state,
  // updating the bargraph level, calling PressAction and redraw thumb control
  bool checkPressed() {
    if (!_enabled || !bargraph.touchEnabled) return false; // Do not check if not enabled
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      // Calculate the relative position of the touch within the bargraph
      uint16_t temp_color = bargraph.needleColor;
      bargraph.needleColor = bargraph.scaleColor; // different color when touched
      _old_level = bargraph.levelIntegrator;
      _levelMark = bargraph.levelMark;
      _level = (float)(_touchProvider->tx - bargraph.barXstart) / (float)bargraph.barWidth;
      bargraph.levelIntegrator = _level;
      bargraph.peakIntegrator  = _level; // force faster update
      update(_level, _levelMark, true);
      while (_touchProvider->checkTouch()) {
        _level = (float)(_touchProvider->tx - bargraph.barXstart) / (float)bargraph.barWidth;
        if (_level < 0) _level = 0; // Ensure level is not negative
        if (_level > 1.0) _level = 1.0; // Ensure level is not greater than maximum
        bargraph.peakIntegrator  = _level; // force faster update
        update(_level, _levelMark);
        delay(10);
      }
      // Wait until the touch is released
      _touchProvider->waitReleased();
      // _pressAction(); // Call the pressed action callback AFTER release - NOT IMPLEMENTED
      _levelMark = _level;
      bargraph.needleColor = temp_color;
      bargraph.levelIntegrator = _old_level;
      bargraph.peakIntegrator  = _old_level; // force fast update
      update(_old_level, _levelMark, true);  // return to old level
      return true;
    }
    return false; // Return the current level of the slider
  }

private:
  struct bargraph_t {
    int range_idx;
    int x, y, width, height, scaleDecimals, valDecimals;
    int barXstart, barYstart, barXend, barYend;
    int barWidth, barHeight, barLength;
    int lastBarPos, lastPeakPos;
    float maxVal, levelMark, peakIntegrator, levelIntegrator;
    uint16_t scaleColor, needleColor, textColor, bezelColor, gradientColor, scaleGradientColor;
    bool peakTracking;
    bool touchEnabled;
  } bargraph;

  bool _active, _visible, _enabled; // Object states
  float _level, _old_level, _levelMark;  // Default level
  int _baseline_y;
	TouchProvider *_touchProvider;
  TFT_eSPI *_tft;
};



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


class VerticalBargraph {
public:
  VerticalBargraph(TFT_eSPI *tft) {
    _tft = tft;
    _touchProvider = NULL; // No touch provider by default
    bargraph.touchEnabled = false; // Default no touch enabled
  }

  // Overload with touch provider
  VerticalBargraph(TFT_eSPI *tft, TouchProvider *touchProvider) {
    _tft = tft;
    _touchProvider = touchProvider;
    bargraph.touchEnabled = true; // Default touch enabled
  }

  // Initialize the bargraph, draw outline and scale based on range_idx
  void init(int x, int y, int width, int height) {
    bargraph.x = x;
    bargraph.y = y;
    bargraph.width = width;
    bargraph.height = height;
    bargraph.textColor = TFT_BLACK;
    bargraph.bezelColor = TFT_GREY;
    bargraph.scaleColor = TFT_LIGHTGREY; // _tft->alphaBlend(140, bargraph.bezelColor, TFT_WHITE);

    bargraph.barXstart = x + 10; // Start of the bar
    bargraph.barYstart = y + 8;
    bargraph.barWidth = width - 48;
    bargraph.barHeight = height - 16;
    bargraph.barLength = bargraph.barHeight;
    bargraph.barXend = bargraph.barXstart + bargraph.barWidth - 1;
    bargraph.barYend = bargraph.barYstart + bargraph.barHeight - 1;
    bargraph.lastBarPos = 0;
    bargraph.lastPeakPos = bargraph.barLength - 2;
    bargraph.levelMark = -99.9;  // enforce full redraw after init

	  _level    = 0.5;  // Default level
    _enabled = true; // Bargraph is enabled by default
    _visible = true; // Bargraph is visible by default
    _active = true;  // Bargraph is active by default
    _baseline_x = bargraph.x + bargraph.width - 38; // Ticks base
  }

  void drawFrame() {
    // Draw the frame around the bar graph
    if (!_visible || !_enabled) return; // Do not draw if not visible
    float tick_p = (float)bargraph.barYstart - 1.0;
    float tick_end = (float)bargraph.barYend;
    float tick_inc = (tick_end - tick_p) / TICK_COUNT_BG;

    int tl = 6; // bargraph.barWidth / 2;
    int x1 = _baseline_x + tl + 2;

    for (int i = 0; i < 3; i++)
        _tft->drawRect(bargraph.x + i, bargraph.y + i, bargraph.width - 2 * i, bargraph.height - 2 * i, bargraph.bezelColor);
    _tft->drawRect(bargraph.x + 3, bargraph.y + 3, bargraph.width - 6, bargraph.height - 6, TFT_DARKGREY);
    _tft->fillRect(bargraph.x + 4, bargraph.y + 4, bargraph.width - 8, bargraph.height - 8, bargraph.scaleColor);

    _tft->drawRect(bargraph.barXstart - 1, bargraph.barYstart - 1, bargraph.barWidth + 2, bargraph.barHeight + 2, bargraph.textColor);
    _tft->fillRectHGradient(bargraph.barXstart, bargraph.barYstart, bargraph.barWidth, bargraph.barHeight, bargraph.scaleColor, bargraph.scaleGradientColor);

    // Draw background ticks
    _tft->setTextDatum(ML_DATUM);
    float mult = 1.0;
    int idx = 0;
    _tft->setTextColor(bargraph.textColor, bargraph.scaleColor);
    while (tick_p <= tick_end) {
      // Draw long tick
      int y1 = rint(tick_p);
      _tft->drawFastHLine(_baseline_x, y1, tl, bargraph.textColor);
      if (idx % 4 == 0) {
        _tft->drawFastHLine(_baseline_x, y1 + 1, tl, bargraph.textColor); // bold line every 4th tick
        if (idx < 16) { // Only draw text for first 4 ticks
          if (idx == 0) {
            _tft->setTextDatum(TL_DATUM);
            _tft->drawString(String(bargraph.maxVal * mult, bargraph.scaleDecimals), _baseline_x + tl + 1, y1 + 1, 1);
          } else {
            _tft->setTextDatum(ML_DATUM);
            _tft->drawString(String(bargraph.maxVal * mult, bargraph.scaleDecimals), _baseline_x + tl + 1, y1 + 1, 1);
          }
        } else if (idx == 16) {
          _tft->setTextDatum(BL_DATUM);
          _tft->drawString(meterScaleUnits[bargraph.range_idx], _baseline_x + tl + 4, y1 + 3, 2);
        }
        mult -= 0.25; // Decrease index for next tick
      }
      tick_p += tick_inc;
      idx++;
    }
    _tft->setTextDatum(TL_DATUM);
  }

  // Sets the color for the meter needle and the range index
  // The range index determines the maximum value and the number of decimals for the display
  // Redraws the meter frame with the new settings
  void setRangeIdxColor(int range_idx, uint16_t color = TFT_GREEN, bool peak_tracking = false) {
    bargraph.needleColor = color;
    bargraph.maxVal = meterScaleMaxVal[range_idx];
    bargraph.valDecimals = meterValDecimals[range_idx];
    bargraph.scaleDecimals = meterScaleDecimals[range_idx];
    bargraph.gradientColor = _tft->alphaBlend(100, color, TFT_BLACK);
    bargraph.scaleGradientColor = _tft->alphaBlend(160, bargraph.scaleColor, TFT_BLACK);
    bargraph.range_idx = range_idx;
    bargraph.peakTracking = peak_tracking;
    drawFrame();
    setLevel(bargraph.levelIntegrator, true);
  }

// #########################################################################

  // Update the bar graph with new values. It will only redraw partial bars needed
  // if the value has changed significantly (visible change) to avoid flicker.
  // A full redraw can be forced by setting full_redraw to true or changing levelMark indicator value.
  // Level and levelMark range from 0 to 1.0 (float) with 1.0 = full deflection.
  // The levelMark triangle indicator is only draw if levelMark >= 0.0.
  void update(float level, float levelMark, bool full_redraw = false) {
    if (!_visible || !_enabled) return; // Do not draw if not visible
    if (level > 1.0) level = 1.0;
    if (level < 0.0) level = 0.0;
    float level_integrator = bargraph.levelIntegrator;
    level_integrator = level * LVLINTEGRATOR + level_integrator * (1 - LVLINTEGRATOR);
    bargraph.levelIntegrator = level_integrator;
    int new_length = (int)((float)(bargraph.barLength - 1) * level_integrator);
    float peak_integrator = bargraph.peakIntegrator;
    if (bargraph.peakTracking) {
      if (level > peak_integrator) {
        peak_integrator = level;
        bargraph.peakIntegrator = level;
      } else {
        #ifdef LINEAR_PEAK_DECAY
          peak_integrator -= PEAK_DECAY;
        #else
          peak_integrator = (level * PEAK_DECAY + peak_integrator * (1 - PEAK_DECAY)) - 0.001;
        #endif
        if (peak_integrator < 0) peak_integrator = 0;
        bargraph.peakIntegrator = peak_integrator;
      }
    } else {
      peak_integrator = level;
    }

    int peak_pos = (int)((float)(bargraph.barLength - 1) * peak_integrator);

    if (peak_pos >= bargraph.barLength - 3) peak_pos = bargraph.barLength - 3; // ensure peak_pos is lower than max
    if (new_length >= bargraph.barLength - 2) new_length = bargraph.barLength - 2; // ensure new_length is lower than max
    if (peak_pos < new_length) peak_pos = new_length; // ensure peak_pos is at least new_length
    if (bargraph.lastPeakPos < new_length)  bargraph.lastPeakPos = new_length; // ensure erase will occur not in bar

    if (bargraph.levelMark != levelMark || full_redraw) {
      // enforce full redraw
      bargraph.levelMark = levelMark;
      bargraph.lastBarPos = 0;
      bargraph.lastPeakPos = bargraph.barLength - 3;
      _tft->fillRectHGradient(bargraph.barXstart, bargraph.barYstart, bargraph.barWidth, bargraph.barHeight, bargraph.scaleColor, bargraph.scaleGradientColor);
    }
    int old_length = bargraph.lastBarPos;
    if ((new_length == old_length) && (peak_pos == bargraph.lastPeakPos)) {
      return;
    }
    // erase area if the bar length has changed
    int16_t new_y = bargraph.barYend - new_length;
    int16_t diff_y = new_length - old_length; // calculate area to erase
    // draw new value bar
    if (diff_y > 0) {
      // draw colored value bar
      _tft->fillRectHGradient(bargraph.barXstart, new_y, bargraph.barWidth, diff_y + 1, bargraph.needleColor, bargraph.gradientColor);
    }
    _tft->drawFastHLine(bargraph.barXstart, new_y, bargraph.barWidth, bargraph.textColor);    // draw bar's end line
    if (diff_y < 0) {
      // fill rest of bar with background
      _tft->fillRectHGradient(bargraph.barXstart, bargraph.barYend - old_length, bargraph.barWidth, -diff_y, bargraph.scaleColor, bargraph.scaleGradientColor);
    }

    if (bargraph.peakTracking && (peak_pos > 0)) {
      // erase old peak indicator with gradient
      _tft->fillRectHGradient(bargraph.barXstart, bargraph.barYend - bargraph.lastPeakPos - 1, bargraph.barWidth, 2, bargraph.scaleColor, bargraph.scaleGradientColor);
      // draw new peak indicator
      _tft->drawFastHLine(bargraph.barXstart, bargraph.barYend - peak_pos, bargraph.barWidth, TFT_RED);
      _tft->drawFastHLine(bargraph.barXstart, bargraph.barYend - peak_pos - 1, bargraph.barWidth, TFT_RED);
    }

    // Draw triangle indicator for set value
    if (levelMark >= 0.0) {
      int sv_indicator = bargraph.barYend - (int)((float)(bargraph.barLength) * levelMark); // Position of set value indicator
      _tft->fillTriangle(bargraph.barXend - 4, sv_indicator, bargraph.barXstart + 6, sv_indicator - 4, bargraph.barXstart + 6, sv_indicator + 3, TFT_BLACK);
      _tft->drawTriangle(bargraph.barXend - 3, sv_indicator, bargraph.barXstart + 5, sv_indicator - 5, bargraph.barXstart + 5, sv_indicator + 4, TFT_WHITE);
    }
    bargraph.lastBarPos = new_length;
    bargraph.lastPeakPos = peak_pos;
  }

  // #########################################################################

  // Checks if coordinates are within the object boundaries
  bool contains(int16_t x, int16_t y) const {
    return ((x >= bargraph.barXstart) && (x <= bargraph.barXend) &&
            (y >= bargraph.barYstart) && (y <= bargraph.barYend));
  }

  void setLevel(float level, bool full_redraw = false) {
    if (full_redraw) {
      drawFrame();
    }
    update(level, bargraph.levelMark, full_redraw);
  }

  void setLevelMarker(float marker_level, bool redraw = false) {
    bargraph.levelMark = marker_level;
    if (_visible && _enabled && redraw)
      update(bargraph.levelIntegrator, bargraph.levelMark);
  }

  // Set level (or inactive) and redraw (or not), overload with redraw
  float getLevelMarker() const {
    return bargraph.levelMark; // Return the set mark value
  }

	// Enabled state overrides Active and Visible states.
	// Object will be ignored and not drawn if not enabled
  // Overload with redraw
  void setEnabled(bool enabled, bool redraw = false) {
    _enabled = enabled;
    if (_visible && _enabled && redraw)
      update(bargraph.levelIntegrator, bargraph.levelMark);
  }
	// enabled state overrides Active and Visible states. Object will be ignored and not drawn if not enabled
	bool getEnabled(void) const { return _enabled; }
	bool isEnabled(void) const { return _enabled; }


  // Check if the bargraph is pressed
  // If the bargraph is pressed, it will enter a modal state,
  // updating the bargraph level, calling PressAction and redraw thumb control
  bool checkPressed() {
    if (!_enabled || !bargraph.touchEnabled) return false; // Do not check if not enabled
    if (_active && _touchProvider->pressed && contains(_touchProvider->tx, _touchProvider->ty)) {
      // Calculate the relative position of the touch within the bargraph
      uint16_t temp_color = bargraph.needleColor;
      bargraph.needleColor = bargraph.scaleColor; // different color when touched
      _old_level = bargraph.levelIntegrator;
      _levelMark = bargraph.levelMark;
      _level = (float)(bargraph.barYend - _touchProvider->ty) / (float)bargraph.barHeight;
      bargraph.levelIntegrator = _level;
      bargraph.peakIntegrator  = _level; // force faster update
      update(_level, _levelMark, true);
      while (_touchProvider->checkTouch()) {
        _level = (float)(bargraph.barYend - _touchProvider->ty) / (float)bargraph.barHeight;
        if (_level < 0) _level = 0; // Ensure level is not negative
        if (_level > 1.0) _level = 1.0; // Ensure level is not greater than maximum
        bargraph.peakIntegrator  = _level; // force faster update
        update(_level, _levelMark);
        delay(10);
      }
      // Wait until the touch is released
      _touchProvider->waitReleased();
      // _pressAction(); // Call the pressed action callback AFTER release - NOT IMPLEMENTED
      _levelMark = _level;
      bargraph.needleColor = temp_color;
      bargraph.levelIntegrator = _old_level;
      bargraph.peakIntegrator  = _old_level; // force fast update
      update(_old_level, _levelMark, true);  // return to old level
      return true;
    }
    return false; // Return the current level of the slider
  }


private:
  struct bargraph_t {
    int range_idx;
    int x, y, width, height, scaleDecimals, valDecimals;
    int barXstart, barYstart, barXend, barYend;
    int barWidth, barHeight, barLength;
    int lastBarPos, lastPeakPos;
    float maxVal, levelMark, peakIntegrator, levelIntegrator;
    uint16_t scaleColor, needleColor, textColor, bezelColor, gradientColor, scaleGradientColor;
    bool peakTracking;
    bool touchEnabled;
  } bargraph;

  bool _active, _visible, _enabled; // Object states
  float _level, _old_level, _levelMark;  // Default level
  int _baseline_x;
	TouchProvider *_touchProvider;
  TFT_eSPI *_tft;
};

#endif