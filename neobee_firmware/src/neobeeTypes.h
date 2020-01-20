#ifndef neobeeTypes_h
#define neobeeTypes_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define DEBUG

#define LOADCELL_DOUT_PIN D2
#define LOADCELL_SCK_PIN  D3
#define REFERENCE_WEIGHT  1550.0f


#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

// Known Flags

#define FLAG_OFFSET_SET 0
#define FLAG_FACTOR_SET 1
#define FLAG_GAIN_SET   2
#define FLAG_NAME_SET   3
#define FLAG_T1_SET     4
#define FLAG_T2_SET     5

// Data strucure to store relevant information
// between two deep sleep cycles
//
typedef union {
  struct {
    char magic_bytes[7];
    char name[20];
    uint8_t flags;
    uint16_t DEEP_SLEEP_SECONDS;
    double SCALE_OFFSET;
    float SCALE_FACTOR;
    uint8_t SCALE_GAIN;
    DeviceAddress addr_inside;
    DeviceAddress addr_outside;
  };
  uint8_t buffer[64];
  char txt[64];
} Context;

const uint8_t ContextSize = sizeof(Context);

#endif
