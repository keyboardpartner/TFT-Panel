#ifndef SCROLLINGSCOPE_H
#define SCROLLINGSCOPE_H

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
Scrolling Scope for TFT_eSPI library
Needs Font 2 (also Font 4 if using large scale label)

#########################################################################
###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
#########################################################################

https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h

Speed optimisation by cm 4/2024
*/
#define NUM_TRACES 2 	// Anzahl der Spuren, die gleichzeitig angezeigt werden können

#define SCOPE_MAXPOINTS 320 // Default-Wert für Arrays, Breite des Displays in Pixel
#define SCOPE_DIVX 6  // Anzahl der vert. Divisions
#define SCOPE_DIVY 5  // Anzahl der hor. Divisions
#define SCOPE_TEXT_H 14 // Höhe der unteren Textbeschriftung in Pixel
#define SCOPE_TEXT_W 30 // Breite der rechten Textbeschriftung in Pixel

// #########################################################################

class ScrollingScopeWidget {
public:
    ScrollingScopeWidget(TFT_eSPI *tftptr) {
        begin(tftptr);
    }

    void begin(TFT_eSPI *tftptr) {
       _tft = tftptr;
    }

    void trace(int trace_idx) {
        int baseY = scope.screen_h + scope.posY;
        uint16_t bg_color = _tft->color565(0, 60, 30);
        uint16_t color = scope.traces[trace_idx].color;
        int16_t traceY;
        int16_t last_traceY = 0;
        int16_t last_traceY_old = 0;
        for (int i = 1; i < (scope.screen_w - 2); i++) {
            traceY = scope.traces[trace_idx].traceVals[i - 1];
            _tft->drawLine(i + scope.posX, baseY - last_traceY_old,
                          i + scope.posX + 1, baseY - traceY, bg_color);
            last_traceY_old = traceY;
            traceY = scope.traces[trace_idx].traceVals[i];
            if (i > 1)
                _tft->drawLine(i + scope.posX, baseY - last_traceY,
                              i + scope.posX + 1, baseY - traceY, color);
            last_traceY = traceY;
        }
    }

    void grid() {
        _tft->drawRect(scope.posX, scope.posY, scope.screen_w + 1, scope.screen_h + 1, TFT_DARKGREEN);
        int grid_pixels = scope.screen_h / SCOPE_DIVY;
        for (int i = grid_pixels; i <= scope.screen_h - SCOPE_DIVY; i += grid_pixels)
            _tft->drawFastHLine(scope.posX, scope.posY + scope.screen_h - i, scope.screen_w, TFT_DARKGREEN);
        grid_pixels = scope.screen_w / SCOPE_DIVX;
        for (int i = grid_pixels; i <= scope.screen_w - SCOPE_DIVX; i += grid_pixels)
            _tft->drawFastVLine(scope.posX + i, scope.posY, scope.screen_h, TFT_DARKGREEN);
    }

    void init(uint16_t posX, uint16_t posY, uint16_t width, uint16_t height) {
        scope.posX = posX;
        scope.posY = posY;
        scope.width = width;
        scope.height = height;
        scope.screen_w = width - SCOPE_TEXT_W - 1;
        scope.screen_h = height - SCOPE_TEXT_H - 2;
        _tft->fillRect(scope.posX, scope.screen_h + 2, scope.screen_w, SCOPE_TEXT_H - 2, TFT_BLACK);
        _tft->fillRect(scope.posX + scope.screen_w + 1, scope.posY, SCOPE_TEXT_W - 1, scope.screen_h, TFT_BLACK);
        _tft->fillRect(posX, posY, scope.screen_w + 1, scope.screen_h + 1, _tft->color565(0, 60, 30));
        grid();
    }

    void newTrace(uint16_t color, int range_idx, int trace_idx, bool show_y_labels) {
        scope.traces[trace_idx].color = color;
        scope.traces[trace_idx].range_idx = range_idx;
        scope.traces[trace_idx].scaledecimals = meterScaleSmallDecimals[range_idx];
        scope.traces[trace_idx].maxVal = meterScaleMaxVal[range_idx];
        scope.traces[trace_idx].scaledecimals = meterScaleDecimals[range_idx];
        for (int i = 0; i < (scope.screen_w); i++) {
            scope.traces[trace_idx].traceVals[i] = 0;
        }
        int grid_pixels = 2 * scope.screen_w / SCOPE_DIVX;
        int time_val = 15;
        int posY = scope.posY + scope.screen_h + 4;
        _tft->setTextFont(1);
        _tft->setTextColor(TFT_WHITE, TFT_BLACK);
        _tft->fillRect(scope.posX, posY, scope.screen_w + 2, SCOPE_TEXT_H - 2, TFT_BLACK);
        for (int i = 0; i <= scope.screen_w; i += grid_pixels) {
            if (i == 0) {
                _tft->setTextDatum(TL_DATUM);
            } else if (i >= scope.screen_w) {
                _tft->setTextDatum(TR_DATUM);
            } else {
                _tft->setTextDatum(TC_DATUM);
            }
            _tft->drawString(String(time_val) + "s", scope.posX + i, posY, 1);
            time_val -= 5;
        }
        if (show_y_labels) {
            _tft->setTextColor(color, TFT_BLACK);
            _tft->fillRect(scope.posX + scope.screen_w + 1, scope.posY, SCOPE_TEXT_W - 1, scope.screen_h - 1, TFT_BLACK);
            grid_pixels = scope.screen_h / SCOPE_DIVY;
            posY = scope.posY + scope.screen_h;
            float val = 0.0;
            for (int i = 0; i <= scope.screen_h; i += grid_pixels) {
                if (i == 0) {
                    _tft->setTextDatum(BL_DATUM);
                } else if (i >= scope.screen_h) {
                    _tft->setTextDatum(TL_DATUM);
                } else {
                    _tft->setTextDatum(CL_DATUM);
                }
                _tft->drawFloat(val, meterScaleSmallDecimals[range_idx], scope.posX + scope.screen_w + 3, posY - i, 1);
                val += scope.traces[trace_idx].maxVal / SCOPE_DIVY;
            }
        }
        _tft->setTextDatum(TL_DATUM);
    }

    void newSample(float level, int trace_idx) {
        for (int i = 0; i < (scope.screen_w - 2); i++) {
            scope.traces[trace_idx].traceVals[i] = scope.traces[trace_idx].traceVals[i + 1];
        }
        int16_t traceY = (int16_t)(level * scope.screen_h);
        if (traceY >= scope.screen_h)
            traceY = scope.screen_h - 1;
        scope.traces[trace_idx].traceVals[scope.screen_w - 2] = traceY;
    }

private:
    TFT_eSPI *_tft;
    struct trace_t {
        int range_idx;
        int scaledecimals;
        float maxVal;
        uint16_t color;
        int16_t traceVals[SCOPE_MAXPOINTS];
    };
    struct {
        int posX, posY, width, height;
        int screen_w;
        int screen_h;
        trace_t traces[NUM_TRACES];
    } scope;
};

#endif