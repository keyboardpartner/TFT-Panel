
#ifndef TOUCH_PROVIDER_H
#define TOUCH_PROVIDER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

#ifdef BOARD_CYD
  #include <XPT2046_Touchscreen.h>
#endif

#define DISPLAY_W 320     // Anzeigebereich Breite
#define DISPLAY_H 240     // Anzeigebereich Höhe

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
  TouchProvider(TFT_eSPI* tft) : _xpt(XPT2046_CS), _xpt_spi(VSPI) {
    _tft = tft;
    tx = 0;
    ty = 0;
    pressed = false;
    _xpt_spi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); // defined in platformio.ini
    _xpt.begin(_xpt_spi);
    _xpt.setRotation(1); // landscape, USB ports right bottom
  }

	// Check if the touch is pressed and get the coordinates
	// Must be called regularly in the main loop to check for touch events
  // read position of XPT digitizer and corresponding TFT position
  // https://github.com/PaulStoffregen/XPT2046_Touchscreen/
  bool checkTouch() {
    uint16_t x, y; uint8_t z;  // XPT

    float xx = (XPT2046_XMAX - XPT2046_XMIN); // width XPT-Points
    float yy = (XPT2046_YMAX - XPT2046_YMIN); // height XPT-Points
    pressed = _xpt.touched();
    if (pressed) {
      _xpt.readData(&x, &y, &z);
      // calc position for TFT display from digitizer position
      tx = (x / xx * DISPLAY_W) - (XPT2046_XMIN / xx * DISPLAY_W);    // TFT_HEIGHT=320
      ty = (y / yy * DISPLAY_H) - (XPT2046_YMIN / yy * DISPLAY_H);    // TFT_WIDTH=240

      // avoid invalid values
      if (tx < 0) tx = 0;
      if (ty < 0) ty = 0;
      if (tx > DISPLAY_W-1) tx = DISPLAY_W-1;
      if (ty > DISPLAY_H-1) ty = DISPLAY_H-1;
      // Serial.println("Touch coordinates: " + String(tx) + ", " + String(ty));

    }
    return pressed;
  }


#else
  TouchProvider(TFT_eSPI* tft) { _tft = tft; tx = 0; ty = 0; pressed = false; }

	// Check if the touch is pressed and get the coordinates
	// Must be called regularly in the main loop to check for touch events
  bool checkTouch() {
    pressed = _tft->getTouch(&tx, &ty); // Get touch coordinates from the TFT display
    return pressed;
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