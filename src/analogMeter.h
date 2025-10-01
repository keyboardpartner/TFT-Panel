#ifndef ANALOGMETERWIDGET_H
#define ANALOGMETERWIDGET_H

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
An example analogue meter using a ILI9341 TFT LCD screen
Needs Font 2 (also Font 4 if using large scale label)

Bargraph display with up to 3 bargraphs, see NUM_BARGRAPHS

Simple large numerical display of the current value

Make sure all the display driver and pin connections are correct by
editing the User_Setup.h file in the TFT_eSPI library folder.

#########################################################################
###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
#########################################################################

https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h

Variable meter size and tremendous speed optimisation by cm 4/2024
*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts
#include "meterScaleDefaults.h"

// Scale is 100% long, 0% start.
#define SCALE_MAX 100
#define TICK_STEP 5
#define TICK_COUNT (SCALE_MAX / TICK_STEP)
#define TICK_ARRSIZE (TICK_COUNT + 1)
// constant for sin/cos/tan offsets, mid scale is 50%
#define DEFL_CONST (90.0 + (SCALE_MAX / 2.0))  // 90 + (SCALE_MAX / 2) deg

class AnalogMeter {
public:
  AnalogMeter(TFT_eSPI *tft) { _tft = tft; }

  // Initialize the meter widget, draw bezel and fill arrays with tick/label positions based on range_idx.
  // Meter parameters are defined in "meterScaleDefaults.h"
  void init(int posX, int posY, int width, int height) {
      meter.posX = posX;
      meter.posY = posY;
      meter.width = width;
      meter.height = height;
      meter.bezelColor = TFT_GREY;
      meter.deflection = -1;
      meter.levelIntegrator = 0.0;

      meter.zoneGreenStart = 0;
      meter.zoneGreenEnd = 0;
      meter.zoneOrangeStart = 0;
      meter.zoneOrangeEnd = 0;
      meter.zoneRedStart = 0;
      meter.zoneRedEnd = 0;

      meter.needle.midX = float(posX) + (float(width) / 2);
      meter.needle.midY = float(posY) + (float(height) * 1.1);

      meter.scale_radius = float(height) * 0.8;
      meter.tick_short_radius = meter.scale_radius * 1.07;
      meter.tick_long_radius = meter.scale_radius * 1.13;
      meter.label_radius = meter.scale_radius * 1.18;
      meter.needle.radius = meter.scale_radius * 1.10;

      //meter.scale_radius_32 = rint(meter.scale_radius);
      //meter.scale_outer_radius_32 = rint(meter.tick_short_radius);

      meter.needle.x0 = meter.needle.midX;
      meter.needle.y0 = posY + height - 14;
      meter.needle.x1 = meter.needle.midX;
      meter.needle.y1 = posY + height - 14;
      meter.needle.width = 2;

      // create tick and label position arrays for faster redraw
      for (int i= 0; i < TICK_ARRSIZE; i++) {
          float deg = float(i * TICK_STEP);
          float sx_t = cos((deg - DEFL_CONST) * 0.0174532925); // -140°, -90-50 to -90+50
          float sy_t = sin((deg - DEFL_CONST) * 0.0174532925);
          scale_X[i] = (int)(sx_t * meter.scale_radius + meter.needle.midX);
          scale_Y[i] = (int)(sy_t * meter.scale_radius + meter.needle.midY);
          scale_short_X[i] = (int)(sx_t * meter.tick_short_radius + meter.needle.midX);
          scale_short_Y[i] = (int)(sy_t * meter.tick_short_radius + meter.needle.midY);
          scale_long_X[i] = (int)(sx_t * meter.tick_long_radius + meter.needle.midX);
          scale_long_Y[i] = (int)(sy_t * meter.tick_long_radius + meter.needle.midY);
          scale_label_X[i] = (int)(sx_t * meter.label_radius + meter.needle.midX);
          scale_label_Y[i] = (int)(sy_t * meter.label_radius + meter.needle.midY);
      }
      for (int i = 0; i < 3; i++)
          _tft->drawRect(meter.posX + i, meter.posY + i, meter.width - 2 * i, meter.height - 2 * i, meter.bezelColor);
      _tft->fillRect(meter.posX + 2, meter.posY + meter.height - 14, meter.width - 4, 14, meter.bezelColor);
      _tft->drawRect(meter.posX + 3, meter.posY + 3, meter.width - 5, meter.height - 15, TFT_DARKGREY);
  }

  // Set the colored zones for the meter scale. A zone is defined by its start and end positions
  // in percent (0 < zoneStart < zoneEnd < 100) and will be drawn only if xxxEnd > xxxStart.
  void setZones(int greenStart = 0, int greenEnd = 0, int orangeStart = 0, int orangeEnd = 0, int redStart = 0, int redEnd = 0) {
    meter.zoneGreenStart = greenStart;
    meter.zoneGreenEnd = greenEnd;
    meter.zoneOrangeStart = orangeStart;
    meter.zoneOrangeEnd = orangeEnd;
    meter.zoneRedStart = redStart;
    meter.zoneRedEnd = redEnd;
    drawPartialScale(0, TICK_COUNT, true); // full redraw
    setLevel(meter.levelIntegrator, true); // force level update and redraw
  }

  // Sets the color for the meter needle and the range index
  // The range index determines the maximum value and the number of decimals for the display
  // Redraws the meter with the new settings
  void setRangeIdxColor(int range_idx, uint16_t color = TFT_RED) {
    meter.needleColor = color;
    meter.maxVal = meterScaleMaxVal[range_idx];
    meter.valDecimals = meterValDecimals[range_idx];
    meter.scaleDecimals = meterScaleDecimals[range_idx];
    meter.range_idx = range_idx;
    setZones(greenRangeStart[range_idx], greenRangeEnd[range_idx],
             orangeRangeStart[range_idx], orangeRangeEnd[range_idx],
             redRangeStart[range_idx], redRangeEnd[range_idx]);
  }

  // ##############################################################################

  // Draws the partial scale between two tick marks start_step and end_step
  void drawPartialScale(int start_step, int end_step, bool redraw_vals) {
    if (start_step < 0) start_step = 0;
    if (end_step > TICK_COUNT) end_step = TICK_COUNT;

    int x0, y0, x1, y1, x0_next, y0_next, x1_next, y1_next, x_temp, y_temp;
    bool has_red_zone = meter.zoneRedEnd > meter.zoneRedStart;
    bool has_orange_zone = meter.zoneOrangeEnd > meter.zoneOrangeStart;
    bool has_green_zone = meter.zoneGreenEnd > meter.zoneGreenStart;
    int start_pos = start_step * TICK_STEP;
    int idx = start_step;
    int end_pos = end_step * TICK_STEP;
    _tft->setTextColor(TFT_BLACK);

    for (int i = start_pos; i <= end_pos; i += TICK_STEP) {
      x0 = scale_X[idx];
      y0 = scale_Y[idx];
      x1 = scale_short_X[idx];
      y1 = scale_short_Y[idx];

      if (i % 25 == 0) {
        x_temp = scale_long_X[idx]; // longer ticks
        y_temp = scale_long_Y[idx];
        _tft->drawLine(x0, y0, x_temp, y_temp, TFT_BLACK);
      } else {
        x_temp = x1; // normal tick length
        y_temp = y1;
      }
      if (idx < TICK_COUNT) {
        x0_next = scale_X[idx + 1];
        y0_next = scale_Y[idx + 1];
        x1_next = scale_short_X[idx + 1];
        y1_next = scale_short_Y[idx + 1];
        if (has_green_zone && (i >= meter.zoneGreenStart && i < meter.zoneGreenEnd)) {
          _tft->fillTriangle(x0, y0, x1, y1, x0_next, y0_next, TFT_GREEN);
          _tft->fillTriangle(x1, y1, x0_next, y0_next, x1_next, y1_next, TFT_GREEN);
          _tft->drawLine(x0, y0, x_temp, y_temp, TFT_BLACK); // redraw, was filled
        }
        if (has_orange_zone && (i >= meter.zoneOrangeStart && i < meter.zoneOrangeEnd)) {
          _tft->fillTriangle(x0, y0, x1, y1, x0_next, y0_next, TFT_ORANGE);
          _tft->fillTriangle(x1, y1, x0_next, y0_next, x1_next, y1_next, TFT_ORANGE);
          _tft->drawLine(x0, y0, x_temp, y_temp, TFT_BLACK); // redraw, was filled
        }
        if (has_red_zone && (i >= meter.zoneRedStart && i < meter.zoneRedEnd)) {
          _tft->fillTriangle(x0, y0, x1, y1, x0_next, y0_next, TFT_RED);
          _tft->fillTriangle(x1, y1, x0_next, y0_next, x1_next, y1_next, TFT_RED);
          _tft->drawLine(x0, y0, x_temp, y_temp, TFT_BLACK); // redraw, was filled
        }
        _tft->drawLine(x0, y0, x0_next, y0_next, TFT_BLACK);
        _tft->drawLine(x1, y1, x1_next, y1_next, TFT_BLACK);
        _tft->drawLine(x0_next, y0_next, x1_next, y1_next, TFT_BLACK); // next tick
      }

      if (redraw_vals && (i % 25 == 0)) {
        x_temp = scale_label_X[idx];
        y_temp = scale_label_Y[idx];
           switch (i / 25) {
          case 0: _tft->drawCentreString("0", x_temp, y_temp - 12, 2); break;
          case 1: _tft->drawCentreString(String(meter.maxVal * 0.25, meter.scaleDecimals), x_temp, y_temp - 9, 2); break;
          case 2: _tft->drawCentreString(String(meter.maxVal * 0.5, meter.scaleDecimals), x_temp, y_temp - 7, 2); break;
          case 3: _tft->drawCentreString(String(meter.maxVal * 0.75, meter.scaleDecimals), x_temp, y_temp - 9, 2); break;
          case 4: _tft->drawCentreString(String(meter.maxVal, meter.scaleDecimals), x_temp, y_temp - 12, 2); break;
        }
      }
      idx++;
    }
  }

  // Draws the scale for the meter. For initial call, set full_redraw to true,
  // otherwise it will only redraw the area around the (erased) old needle position
  // as needle may have moved and erased the scale.
  void drawScale(int needle_pos, bool full_redraw) {
    if (full_redraw) {
      drawPartialScale(0, TICK_COUNT, true); // full redraw
    } else {
      // integer needle position runs from 0 to 100 for full scale
      needle_pos = constrain(needle_pos, -5, 105);
      int redraw_step = needle_pos / TICK_STEP;
      drawPartialScale(redraw_step - 1, redraw_step + 1, false); // redraw around old needle position
    }
  }

  // ##############################################################################

  // Draws the scale for the meter and updates needle. For initial call, set full_redraw to true,
  // otherwise it will only redraw the area around the (erased) old needle position
  // as needle may have moved and erased the scale.
  // Level ranges from 0 to 1.0 (float) with 1.0 = full deflection.
  void setLevel(float level, bool full_redraw = false) {
      if (full_redraw) {
        _tft->fillRect(meter.posX + 4, meter.posY + 4, meter.width - 7, meter.height - 17, TFT_WHITE);
        meter.deflection = -1;
        meter.levelIntegrator = level;
      }

      float last_level = meter.levelIntegrator;
      meter.levelIntegrator = level * LVLINTEGRATOR + meter.levelIntegrator * (1 - LVLINTEGRATOR);

      if ((meter.levelIntegrator > last_level + 0.001) || (meter.levelIntegrator < last_level - 0.001) || full_redraw) {
        _tft->setTextDatum(TL_DATUM);
        _tft->setFreeFont(FF22);
        _tft->setTextColor(meter.needleColor, TFT_WHITE);
        _tft->setTextPadding(60);
        int x0 = meter.posX + 10;
        int y0 = meter.posY + (meter.height * 4) / 5 - 12;
        _tft->drawFloat(meter.levelIntegrator * meter.maxVal, meter.valDecimals, x0, y0);
        _tft->setTextPadding(0);
      }
      if (meter.levelIntegrator > 1.05) {
        meter.levelIntegrator = 1.05;
      }
      if (meter.levelIntegrator < -0.05) {
        meter.levelIntegrator = -0.05;
      }
      // Update needle position only if deviation is greater than 0.25 %
      int newdefl = rint(400.0 * meter.levelIntegrator);
      if ((newdefl == meter.deflection) && !full_redraw) {
        return;
      }

      // restore unit label, may be erased by old needle drawing
      _tft->setTextColor(TFT_BLACK, TFT_WHITE);
      _tft->setFreeFont(FF22);
      _tft->setTextDatum(TC_DATUM);
      _tft->drawString(meterScaleUnits[meter.range_idx], meter.posX + meter.width / 2, meter.posY + meter.height / 2);

      // erase needle with old position (erase to white).
      // int nw = meter.needle.width; // needle base width for triangular needle
      // _tft->fillTriangle(meter.needle.x0 - nw, meter.needle.y0, meter.needle.x0 + nw, meter.needle.y0, meter.needle.x1, meter.needle.y1, TFT_WHITE);
      _tft->drawWideLine(meter.needle.x0, meter.needle.y0, meter.needle.x1, meter.needle.y1, 2, TFT_WHITE, TFT_WHITE);

      drawScale(meter.deflection/4, full_redraw); // after old needle is erased
      meter.deflection = newdefl;

      float sdeg = meter.levelIntegrator * SCALE_MAX - DEFL_CONST;
      float sx = cos(sdeg * 0.0174532925);
      float sy = sin(sdeg * 0.0174532925);
      float tx = tan((sdeg + 90) * 0.0174532925);

      float shift_x = (meter.height * tx) / 8;
      // nw = rint(abs(shift_x/10)) + 2; // needle base width for triangular needle
      // meter.needle.width = nw;
      meter.needle.x0 = meter.needle.midX + rint(shift_x); // shift of needle bottom X to simulate pivot point
      meter.needle.y0 = meter.posY + meter.height - 15;
      meter.needle.x1 = sx * meter.needle.radius + meter.needle.midX;
      meter.needle.y1 = sy * meter.needle.radius + meter.needle.midY;

      // Draw new needle. You may prefer a triangular shaped needle:
      // _tft->fillTriangle(meter.needle.x0 - nw, meter.needle.y0, meter.needle.x0 + nw, meter.needle.y0, meter.needle.x1, meter.needle.y1, meter.needleColor);
      // draw new meter.needle, 2 lines to thicken meter.needle
      _tft->drawWideLine(meter.needle.x0, meter.needle.y0, meter.needle.x1, meter.needle.y1, 2, meter.needleColor, TFT_WHITE);

      _tft->setTextDatum(TL_DATUM);
  }

  // ##############################################################################

  // Returns true if the point (x, y) is inside the meter area
  bool contains(int16_t x, int16_t y) {
    return ((x >= meter.posX) && (x < (meter.posX + meter.width)) &&
            (y >= meter.posY) && (y < (meter.posY + meter.height)));
  }

  // ##############################################################################

private:
  TFT_eSPI *_tft;

  struct needle_t {
    int x0, y0, x1, y1, width;
    float midX, midY, radius;
  };

  struct meter_t {
    int range_idx;
    int posX, posY, width, height, scaleDecimals, valDecimals, deflection;
    int zoneGreenStart, zoneGreenEnd, zoneOrangeStart, zoneOrangeEnd, zoneRedStart, zoneRedEnd;
    float maxVal, levelIntegrator;
    uint16_t scaleColor, needleColor, textColor, bezelColor;
    bool peakTracking;
    needle_t needle;
    float scale_radius;
    float tick_short_radius;
    float tick_long_radius;
    float label_radius;
    int32_t scale_radius_32;
    int32_t scale_outer_radius_32;
  } meter;

  int scale_X[TICK_ARRSIZE];
  int scale_Y[TICK_ARRSIZE];
  int scale_short_X[TICK_ARRSIZE];
  int scale_short_Y[TICK_ARRSIZE];
  int scale_long_X[TICK_ARRSIZE];
  int scale_long_Y[TICK_ARRSIZE];
  int scale_label_X[TICK_ARRSIZE];
  int scale_label_Y[TICK_ARRSIZE];
};

#endif