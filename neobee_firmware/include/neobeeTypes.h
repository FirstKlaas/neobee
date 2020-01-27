#ifndef neobeeTypes_h
#define neobeeTypes_h

#include <Arduino.h>
#include <EEPROM.h>

#define LOADCELL_DOUT_PIN D2
#define LOADCELL_SCK_PIN  D3
#define REFERENCE_WEIGHT  1550.0f

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>


// Global FLAGS
#define FLAG_OFFSET_SET           0
#define FLAG_FACTOR_SET           1
#define FLAG_GAIN_SET             2
#define FLAG_NAME_SET             3
#define FLAG_ADDR_INSIDE_SET      4
#define FLAG_ADDR_OUTSIDE_SET     5
#define DEEP_SLEEP_SET            6

enum class OperationMode : uint8_t {
  AP_MODE           = 1,  // Starting wifi in access point or station mode
  CMD_MODE          = 2   // Running in command mode or in broadcast mode
};

enum class WifiFlags : uint8_t {
  FLAG_SSID_SET     = 1,
  FLAG_PASSWORD_SET = 2,
  FLAG_ACTIVE       = 4
};

enum class CmdCode : uint8_t {
  NOP              =   0,
  GET_NAME         =   1,
  SET_NAME         =   2,
  GET_FLAGS        =   3,
  RESET_SETTINGS   =   4,
  SAVE_SETTINGS    =   5,
  ERASE_SETTINGS   =   6,

  GET_SCALE_OFFSET =  10,
  SET_SCALE_OFFSET =  11,
  GET_SCALE_FACTOR =  12,
  SET_SCALE_FACTOR =  13,

  GET_SSID         =  20,
  SET_SSID         =  21,
  CLEAR_SSID       =  22,
  GET_PASSWORD     =  23,
  SET_PASSWORD     =  24,
  CLEAR_PASSWORD   =  25,
  SET_WIFI_ACTIVE  =  26,
  GET_WIFI_FLAGS   =  27,

  GET_MQTT_HOST    =  30,
  SET_MQTT_HOST    =  31,
  GET_MQTT_PORT    =  32,
  SET_MQTT_PORT    =  33,
  SET_MQTT_ACTIVE  =  36,
  GET_MQTT_FLAGS   =  37,

  GET_MAC_ADDRESS  =  80,
  GET_VERSION      =  81,
  SET_IDLE_TIME    =  82,
  GET_IDLE_TIME    =  83, 

  TARE             = 200,
  CALIBRATE        = 201,
  GET_WEIGHT       = 202
};

enum class StatusCode : uint8_t {
  NONE             =  0,
  OK               =  1,
  BAD_REQUEST      =  2,
  NOT_FOUND        =  3,
  ILLEGAL_STATE    =  4,
};


typedef struct {
  double offset;                    // The offset as a result of taring the load cell
  float factor;                     // Factor to be used to convert readings into units
  uint8_t gain;                     // Which channel to select.
} Scale;

typedef struct {
  uint8_t flags;                    // Temperature flags
  DeviceAddress addr_inside;        // Address of the inside temperature sensor
  DeviceAddress addr_outside;       // Address of the outside temperature sensor
} Temperature;


inline void writeInt32(uint32_t value, uint8_t* dst) {
    dst[0] = (value >> 24) & 0xff;
    dst[1] = (value >> 16) & 0xff;
    dst[2] = (value >> 8) & 0xff;
    dst[3] = value & 0xff;
}

inline uint32_t readInt32(uint8_t* dst)
{
    return (dst[0] << 24) + (dst[1] << 16) + (dst[2] << 8) + (dst[3]); 
}

inline void printByteArray(const uint8_t* buffer, const uint8_t size = 32)
{
    #ifdef DEBUG
    for (uint8_t i=0; i<size; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(":");
    }
    Serial.println();
    #endif
}

#endif
