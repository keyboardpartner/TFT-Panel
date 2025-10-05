
#ifndef TOUCH_PROVIDER_H
#define TOUCH_PROVIDER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

#ifdef BOARD_CYD
  #include <XPT2046_Touchscreen.h>
  // #define DEBUG_TOUCH   // Enable touch debug messages
#endif

#define DISPLAY_W 320     // Anzeigebereich Breite
#define DISPLAY_H 240     // Anzeigebereich Höhe
#define TOUCH_OFFSET 5    // Offset for touch area to compensate edge touches on CYD board

// #####################################################################################
// Addittion by cm 7/25:
// TouchProvider class for handling touch events as a common provider for all widgets
// This allows the widgets to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly

// Function to get touch coordinates from the TFT display
// It returns pressed = true if a touch is detected, false otherwise
// The coordinates are stored in the tx and ty variables
// The procedure
// touchProvider.checkTouch();
// must be used in the main loop or any modal dialog wait loops
// to check for touch events
//
// Provides separate touch handling for CYD board with XPT2046 touch screen
//
// #####################################################################################

// TouchProvider class for handling touch and rotary encoder events for all widgets
// This allows the widgets to access touch events through a shared TouchProvider instance
// since tft->getTouch(&tx, &ty) is time-consuming and no longer used directly

class TouchProvider : public TFT_eSPI {
public:
	uint16_t tx, ty; // Touch coordinates
	bool pressed; // Touch pressed state

  // Must be initialized with a pointer to a TFT_eSPI object
	// This allows the TouchProvider to access the TFT_eSPI methods

#ifdef BOARD_CYD
  // Initialise with example calibration values so processor does not crash if setTouch() not called in setup()
  uint16_t tcal_x0 = XPT2046_XMIN, tcal_y0 = XPT2046_YMIN;
  float tcal_w = XPT2046_XMAX - XPT2046_XMIN, tcal_h = XPT2046_YMAX - XPT2046_YMIN;

  TouchProvider(TFT_eSPI* tft) : _xpt(XPT2046_CS), _xpt_spi(VSPI) {
    _tft = tft;
    tx = 0;
    ty = 0;
    pressed = false;
    _xpt_spi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); // defined in platformio.ini
    _xpt.begin(_xpt_spi);
    _xpt.setRotation(1); // landscape, USB ports right bottom
  }

  void reInit() {
    _xpt_spi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); // defined in platformio.ini
    _xpt.begin(_xpt_spi);
    _xpt.setRotation(1); // landscape, USB ports right bottom
  }

  void end() {
    _xpt_spi.end(); // free the SPI bus
  }

	// Check if the touch is pressed and get the coordinates
	// Must be called regularly in the main loop to check for touch events
  // read position of XPT digitizer and corresponding TFT position
  // https://github.com/PaulStoffregen/XPT2046_Touchscreen/
  bool checkTouch() {
    uint16_t x, y; uint8_t z;  // XPT
    pressed = _xpt.touched();
    if (pressed) {
      _xpt.readData(&x, &y, &z);
      #ifdef DEBUG_TOUCH
        Serial.print("Touch raw: x=" + String(x) + ", y=" + String(y));
      #endif
      if (x < tcal_x0) x = tcal_x0; // avoid invalid values
      if (y < tcal_y0) y = tcal_y0; // avoid invalid values
      // calc position for TFT display from digitizer position
      tx = DISPLAY_W * (float(x - tcal_x0)/tcal_w);    // TFT_HEIGHT=320
      ty = DISPLAY_H * (float(y - tcal_y0)/tcal_h);    // TFT_WIDTH=240

      // avoid invalid values
      if (tx >= DISPLAY_W) tx = DISPLAY_W-1;
      if (ty >= DISPLAY_H) ty = DISPLAY_H-1;
      #ifdef DEBUG_TOUCH
        Serial.println(" mapped: tx=" + String(tx) + ", ty=" + String(ty));
      #endif
    }
    return pressed;
  }

  // Touch calibration for CYD board with XPT2046 touch screen
  // parameters: Array of 7 uint16_t values to store the calibration parameters
  void calibrateTouchCYD(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size){
    int16_t values[] = {0,0,0,0,0,0,0,0};
    uint16_t x, y; uint8_t z;  // XPT raw data
    #ifdef DEBUG_TOUCH
      Serial.println("Touch calibration");
    #endif
    for(uint8_t i = 0; i<4; i++){
      _tft->fillRect(0, 0, size+1, size+1, color_bg);
      _tft->fillRect(0, DISPLAY_H-size-1, size+1, size+1, color_bg);
      _tft->fillRect(DISPLAY_W-size-1, 0, size+1, size+1, color_bg);
      _tft->fillRect(DISPLAY_W-size-1, DISPLAY_H-size-1, size+1, size+1, color_bg);
      while(_xpt.touched()) delay(10); // wait for press release
      switch (i) {
        case 0: // up left
          _tft->drawLine(0, 0, 0, size, color_fg);
          _tft->drawLine(0, 0, size, 0, color_fg);
          _tft->drawLine(0, 0, size , size, color_fg);
          break;
        case 1: // bot left
          _tft->drawLine(0, DISPLAY_H-size-1, 0, DISPLAY_H-1, color_fg);
          _tft->drawLine(0, DISPLAY_H-1, size, DISPLAY_H-1, color_fg);
          _tft->drawLine(size, DISPLAY_H-size-1, 0, DISPLAY_H-1 , color_fg);
          break;
        case 2: // up right
          _tft->drawLine(DISPLAY_W-size-1, 0, DISPLAY_W-1, 0, color_fg);
          _tft->drawLine(DISPLAY_W-size-1, size, DISPLAY_W-1, 0, color_fg);
          _tft->drawLine(DISPLAY_W-1, size, DISPLAY_W-1, 0, color_fg);
          break;
        case 3: // bot right
          _tft->drawLine(DISPLAY_W-size-1, DISPLAY_H-size-1, DISPLAY_W-1, DISPLAY_H-1, color_fg);
          _tft->drawLine(DISPLAY_W-1, DISPLAY_H-1-size, DISPLAY_W-1, DISPLAY_H-1, color_fg);
          _tft->drawLine(DISPLAY_W-1-size, DISPLAY_H-1, DISPLAY_W-1, DISPLAY_H-1, color_fg);
          break;
      }

      // user has to get the chance to release
      if(i>0) {
        delay(200);
      }

      for(uint8_t j= 0; j<4; j++){
        delay(10);
        while(!_xpt.touched()); // wait for press
        _xpt.readData(&x, &y, &z);
        values[i*2  ] += x;
        values[i*2+1] += y;
      }
      values[i*2  ] /= 4;
      values[i*2+1] /= 4;
      #ifdef DEBUG_TOUCH
        Serial.println("Touch calibration: x=" + String(values[i*2]) + ", y=" + String(values[i*2+1]));
      #endif
    }
    _tft->fillRect(DISPLAY_W-size-1, DISPLAY_H-size-1, size+1, size+1, color_bg);
    // export parameters, if pointer valid
    if(parameters != NULL){
      parameters[0] = (values[0] + values[2]) / 2 + TOUCH_OFFSET; // x0
      parameters[1] = (values[1] + values[5]) / 2 + TOUCH_OFFSET; // y0
      parameters[2] = (values[4] + values[6]) / 2 - TOUCH_OFFSET; // x1
      parameters[3] = (values[3] + values[7]) / 2 - TOUCH_OFFSET; // y1
    }
  }

  // Set the touch calibration values for CYD board with XPT2046 touch screen
  // parameters: Array of 5 uint16_t values with the calibration parameters
  void setTouchCYD(uint16_t *parameters){
    tcal_x0 = parameters[0];
    tcal_y0 = parameters[1];
    tcal_w = float(parameters[2] - parameters[0]); // touch width area
    tcal_h = float(parameters[3] - parameters[1]); // touch height area
    #ifdef DEBUG_TOUCH
      Serial.println("Touch calibration: x0=" + String(tcal_x0) + ", y0=" + String(tcal_y0) + ", w=" + String(tcal_w) + ", h=" + String(tcal_h));
    #endif
  }

#else
  TouchProvider(TFT_eSPI* tft) { _tft = tft; tx = 0; ty = 0; pressed = false; }

	// Check if the touch is pressed and get the coordinates
	// Must be called regularly in the main loop to check for touch events
  bool checkTouch() {
    pressed = _tft->getTouch(&tx, &ty); // Get touch coordinates from the TFT display
    return pressed;
  }

  void reInit() {
  }

  void end() {
  }

#endif

  // Wait for Touch release
  void waitReleased() {
    while (checkTouch()) {
      delay(10);
    }
  }

  // Check if the last touch coordinates are within the specified rectangle
  // x1, y1 are the top left corner, x2, y2 are the bottom right corner
  bool isPressedWithin(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    if (pressed && (tx >= x1) && (tx <= x2) && (ty >= y1) && (ty <= y2))
      return true;
    return false;
  }

  // Check if the new touch coordinates are within the specified rectangle
  // x1, y1 are the top left corner, x2, y2 are the bottom right corner
  bool checkWithin(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    checkTouch();
    if (isPressedWithin(x1, y1, x2, y2))
      return true;
    return false;
  }

  //#############################################################################

  // Get encoder delta since last call
  // After execution, the encoder delta is reset by request
  int getEncDelta(bool reset_after_call = true) {
    int new_delta = _enc_delta;
    if (reset_after_call) {
      _enc_delta = 0;
    }
    return new_delta;
  }

  // Reset encoder delta
  void resetEncDelta() {
    _enc_delta = 0;
  }

  // Callback for Encoder Ticker, must be called regularly, at least every 5 ms
  void encoderTick() {
    #ifdef ENCODER_ENABLED
      // Check if the encoder is turned
      uint8_t enc_a = digitalRead(ENCA_PIN);
      uint8_t enc_b = digitalRead(ENCB_PIN);
      if ((enc_a != _enc_a_old) || (enc_b != _enc_b_old)) {
        // A oder B haben sich geändert
        if (!(enc_a | enc_b)) // beide auf 0, Rastung überschritten
          _enc_armed = 1;
        if (enc_a & enc_b) {
          // beide wieder in Ruhe = "1"
          if (_enc_armed) {
            // Beschleunigungsfunktion
            uint32_t enc_now = millis();
            uint32_t enc_interval = enc_now - _enc_accel_timer;
            _enc_accel_timer = enc_now;
            int enc_abs = 1;
            if (enc_interval < 20)  // acceleration margin 20ms per step
              enc_abs = 3;
            else if (enc_interval < 40)
              enc_abs = 2;

            if (!_enc_a_old) {
              _enc_delta += enc_abs;
            }
            if (!_enc_b_old) {
              _enc_delta -= enc_abs;
            }
            _enc_armed = 0;
          }
        }
      }
      _enc_a_old = enc_a;
      _enc_b_old = enc_b;
    #else
      _enc_delta = 0;
    #endif // ENCODER_ENABLED
  }

private:
  uint8_t _enc_a_old, _enc_b_old, _enc_ready, _enc_armed;
  uint32_t _enc_accel_timer; // für Encoder-Beschleunigung
  int _enc_delta = 0; // Änderung des Dreh-Encoders
  TFT_eSPI* _tft;
  #ifdef BOARD_CYD
    SPIClass _xpt_spi;      // SPI-Interface for XPT2046_Touchscreen
    XPT2046_Touchscreen _xpt;    // polling
    // XPT2046_Touchscreen xpt(XPT2046_CS, XPT2046_IRQ); // using IRQ
  #endif
};

#endif