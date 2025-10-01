#ifndef METER_SCALE_DEFAULTS_H
#define METER_SCALE_DEFAULTS_H

// #########################################################################
// Meter Scale Defaults, Messbereiche und Skalierungen
// für Analog-Meter, Scrolling Scope und Bargraph-Widgets
// #########################################################################

// Integrator-Konstanten für Numeric, Meter und Bargraphen
// Glättung der Anzeige, damit sie nicht so zappelt
#define LVLINTEGRATOR 0.25
#define PEAK_DECAY    0.05    // Decay factor for peak tracking, 0.002 for linear decay, 0.05 for lowpass decay
// #define LINEAR_PEAK_DECAY  // If defined, peak decay is linear, otherwise exponential decay is used

// Diese Skalenwerte sind für die Analog-Meter-Anzeige und die Bargraphen

const float meterScaleMaxVal[10] = {
    30,  // #0 mA Vollausschlag
    100, // #1
    300, // #2
    1,   // #3 A Vollausschlag
    3,   // #4
    1,   // #5 V Vollausschlag
    3,   // #6 5V
    10,  // #7 20V
    30,  // #8 50V
    100  // #9 200V
  } ;

const char* meterScaleUnits[10]= {
    "mA", // #0 30mA
    "mA", // #1 100mA
    "mA", // #2 300mA
    "A", // #3 1A
    "A", // #4 3A
    "V",  // #5 1V
    "V",  // #6 3V
    "V", // #7 10V
    "V", // #8 30V
    "V"  // #9 100V
} ;

const int meterValDecimals[10] = {
    1, // #0 30mA
    0, // #1 100mA
    0, // #2 300mA
    2, // #3 1A
    2, // #4 3A
    2, // #5 1V
    2, // #6 3V
    2, // #7 10V
    1, // #8 30V
    1  // #9 100V
  } ;

const int meterScaleDecimals[10] = {
    0, // #0 30mA
    0, // #1 100mA
    0, // #2 300mA
    2, // #3 1A
    1, // #4 3A
    2, // #5 1V
    2, // #6 3V
    1, // #7 10V
    0, // #8 30V
    0  // #9 100V
  } ;

const int meterScaleSmallDecimals[10] = {
    0, // #0 30mA
    0, // #1 100mA
    0, // #2 300mA
    1, // #3 1A
    1, // #4 3A
    2, // #5 1V
    1, // #6 3V
    0, // #7 10V
    0, // #8 30V
    0  // #9 100V
  } ;

// ###########################################################################
// Farben für die Skalenbereiche
// ###########################################################################

int greenRangeStart[10] = {
  0, // #0 30mA
  0, // #1 100mA
  0, // #2 300mA
  0, // #3 1A
  0, // #4 3A
  0, // #5 1V
  0, // #6 3V
  45, // #7 10V
  0, // #8 30V
  0  // #9 100V
};

int greenRangeEnd[10] = {
  20, // #0 30mA
  20, // #1 100mA
  20, // #2 300mA
  20, // #3 1A
  0, // #4 3A
  0, // #5 1V
  0, // #6 3V
  55, // #7 10V
  0, // #8 30V
  0  // #9 100V
};

int orangeRangeStart[10] = {
  0, // #0 30mA
  0, // #1 100mA
  0, // #2 300mA
  0, // #3 1A
  0, // #4 3A
  0, // #5 1V
  0, // #6 3V
  75, // #7 10V
  0, // #8 30V
  0  // #9 100V
};

int orangeRangeEnd[10] = {
  0, // #0 30mA
  0, // #1 100mA
  0, // #2 300mA
  0, // #3 1A
  0, // #4 3A
  0, // #5 1V
  0, // #6 3V
  90, // #7 10V
  0, // #8 30V
  0  // #9 100V
};

int redRangeStart[10] = {
  90, // #0 30mA
  90, // #1 100mA
  90, // #2 300mA
  90, // #3 1A
  90, // #4 3A
  0, // #5 1V
  0, // #6 3V
  90, // #7 10V
  0, // #8 30V
  0  // #9 100V
};

int redRangeEnd[10] = {
  100, // #0 30mA
  100, // #1 100mA
  100, // #2 300mA
  100, // #3 1A
  100, // #4 3A
  0, // #5 1V
  0, // #6 3V
  100, // #7 10V
  0, // #8 30V
  0  // #9 100V
};

#endif // METER_SCALE_DEFAULTS_H
