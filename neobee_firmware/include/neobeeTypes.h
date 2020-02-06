#ifndef neobeeTypes_h
#define neobeeTypes_h

#include <Arduino.h>
#include <EEPROM.h>

#define LOADCELL_DOUT_PIN D2
#define LOADCELL_SCK_PIN  D3
#define REFERENCE_WEIGHT  1550.0f

#include <OneWire.h>
#include <DallasTemperature.h>

// Global FLAGS
#define FLAG_OFFSET_SET           0
#define FLAG_FACTOR_SET           1
#define FLAG_GAIN_SET             2
#define FLAG_NAME_SET             3
#define FLAG_ADDR_INSIDE_SET      4
#define FLAG_ADDR_OUTSIDE_SET     5
#define FLAG_DEEP_SLEEP_SET       6

enum class OperationMode : uint8_t {
  IOT_MODE          = 1,  // Starting wifi in access point or station mode
  CMD_MODE          = 2,   // Running in command mode or in broadcast mode
};


enum class WifiFlags : uint8_t {
  FLAG_SSID_SET     = 1,
  FLAG_PASSWORD_SET = 2,
  FLAG_ACTIVE       = 4
};

enum class CmdCode : uint8_t {
  NOP              =   0,
  NAME             =   1,
  GET_FLAGS        =   3,
  RESET_SETTINGS   =   4,
  SAVE_SETTINGS    =   5,
  ERASE_SETTINGS   =   6,
  RESET_ESP        =   7,

  SCALE_OFFSET     =  10,
  SCALE_FACTOR     =  12,
  
  SSID             =  20,
  PASSWORD         =  23,
  SET_WIFI_ACTIVE  =  26,
  GET_WIFI_FLAGS   =  27,

  MQTT_HOST        =  30,
  MQTT_PORT        =  32,
  MQTT_LOGIN       =  34,
  MQTT_PASSWORD    =  36,
  
  MQTT_ACTIVE      =  38, // NOT IMPLEMENTED
  GET_MQTT_FLAGS   =  39, // NOT IMPLEMENTED

  GET_TEMPERATURE  =  40,
  
  GET_MAC_ADDRESS  =  80,
  GET_VERSION      =  81,
  IDLE_TIME        =  82,
  SET_DEEP_SLEEP   =  84, // Activating or deactivating deep sleep mode

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
  BAD_METHOD       =  5,
};

enum class RequestMethod : uint8_t {
  NONE             =  0,
  GET              =  1,
  PUT              =  2,
  DELETE           =  3
};

#define REQUEST_METHOD_MASK 3

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
