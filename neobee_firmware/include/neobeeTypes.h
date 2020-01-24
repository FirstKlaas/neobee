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
  OK               = 20,
  BAD_REQUEST      = 40,
  NOT_FOUND        = 44
};

// Data strucure to store relevant information
// between two deep sleep cycles
//
typedef struct {
  uint8_t flags;                    // Mqtt Flags
  char host_name[31];               // 0-terminated hostname or ip of the mqtt server
  uint16_t port;                    // mqtt port
} MqttServer;

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

typedef struct wifi_network {
  uint8_t flags;                    // Wifi flags
  char ssid[31];                    // 0-terminated name of the wifi network
  char password[31];                // 0-terminated password
  uint8_t channel;

  wifi_network() {
    reset();
  };

  inline bool hasPassword() const {
    return bitRead(flags, int(WifiFlags::FLAG_PASSWORD_SET));
  }

  inline void setPassword(const uint8_t* data) {
    const uint8_t size = sizeof(password)-1;
    memcpy(password, data, size);
    password[size] = 0; // Make shure the array is 0 terminated
    bitSet(flags, int(WifiFlags::FLAG_PASSWORD_SET));
  }

  inline void getPassword(uint8_t* dst) {
    const uint8_t size = sizeof(password)-1;
    if (hasPassword()) memcpy(dst, password, size);
  }

  inline void clearPassword() {
    memset(password, 0, sizeof(password));
    bitClear(flags, int(WifiFlags::FLAG_PASSWORD_SET));
  }

  inline bool hasSSID() const {
    return bitRead(flags, int(WifiFlags::FLAG_SSID_SET));
  }
  
  inline void setSSID(const uint8_t* data) {
    const uint8_t size = sizeof(ssid)-1;
    memcpy(ssid, data, size);
    ssid[size] = 0;
    bitSet(flags, int(WifiFlags::FLAG_SSID_SET));
  }

  inline void getSSID(uint8_t* dst) {
    const uint8_t size = sizeof(ssid)-1;
    if (hasSSID()) memcpy(dst, ssid, size);
  }

  inline void clearSSID() {
    memset(ssid, 0, sizeof(ssid));
    bitClear(flags, int(WifiFlags::FLAG_SSID_SET));
  }

  inline void reset() {
    flags = 0;
    memset(ssid, 0, sizeof(ssid));
    memset(password, 0, sizeof(password));
    channel = 1;
  }
  
} WifiNetwork;

typedef struct context {
  uint8_t magic_bytes[6];           // Must be NEOBEE for a valid data block
  uint8_t name[20];                 // Human readable name of the device
  uint8_t flags;
  uint16_t deep_sleep_seconds;      // Seconds to go to sleep

  Scale scale;
  Temperature temperature;
  MqttServer mqttServer;
  WifiNetwork wifi_network;

  context() {
    reset();
  };

  inline bool hasOffset() const { return bitRead(flags, FLAG_OFFSET_SET); };
  inline bool hasName() const { return _flag(FLAG_NAME_SET); };
  inline bool hasGain() const { return _flag(FLAG_GAIN_SET); };
  inline bool hasFactor() const { return _flag(FLAG_FACTOR_SET); };
  inline bool hasAddrInside() const { return _flag(FLAG_ADDR_INSIDE_SET); };
  inline bool hasAddrOutside() const { return _flag(FLAG_ADDR_OUTSIDE_SET); };

  inline void setName(const uint8_t* new_name)
  {
    memcpy(name, new_name, sizeof(name));
    bitSet(flags, FLAG_NAME_SET);
  }

  void reset() {
    memcpy(magic_bytes, "NEOBEE", 6);
    flags=0;
    memset(name,0,sizeof(name));
    scale.offset = 0.f;
    scale.factor = 1.f;
    scale.gain = 128;
  };

  inline bool _flag(uint8_t flag) const { return bitRead(flags, flag); };

  bool load() {
    EEPROM.begin(4095);
    EEPROM.get(0, this);
    EEPROM.end();
    if (memcmp(magic_bytes, "NEOBEE", 6) == 0) {
      #ifdef DEBUG
      Serial.println("Context loaded successfully");
      #endif
      
      return true;
    } else {
      /****
       * Magic bytes not found. So we initialize the structure 
       * with some meaningful defaults and return false so indicate
       * that this ist the first use after a reset or used for the
       * very first time.
       */ 
      #ifdef DEBUG
      Serial.println("Seems to be new device. No context.");
      #endif
      reset(); 
      return false;
    }
  }

  void save() const {
    #ifdef DEBUG
    Serial.print("Saving Context");
    #endif
    EEPROM.begin(4095);
    EEPROM.put(0, this);
    delay(200);
    EEPROM.commit();
    EEPROM.end();
    #ifdef DEBUG
    Serial.println(" - Done!");
    #endif
  }

  void erase() {
    #ifdef DEBUG
    Serial.println("Deleting context");
    #endif
    reset();
    memset(magic_bytes, 0, 7);
    flags = 0;
    save();  
  }

  void print() const {
    #ifdef DEBUG
    /**
    Serial.print("Name        : ");
    Serial.println(name);
    
    Serial.print("Magic Bytes : ");
    Serial.println(magic_bytes);
    **/
    Serial.print("Offset      : ");
    Serial.println(scale.offset);
    
    Serial.print("Factor      : ");
    Serial.println(scale.factor);

    Serial.println("");
    Serial.println("-----------------------");
    Serial.println("Flags");
    Serial.println("-----------------------");

    Serial.print("Offset set  : ");

    Serial.println(hasOffset());

    Serial.print("Factor set  : ");
    Serial.println(hasFactor());

    Serial.print("Gain set    : ");
    Serial.println(hasGain());

    Serial.print("Name set    : ");
    Serial.println(hasName());
    
    Serial.print("Temp. Inside set  : ");
    Serial.println(bitRead(flags, FLAG_ADDR_INSIDE_SET));
    
    Serial.print("Temp. Outside set : ");
    Serial.println(bitRead(flags, FLAG_ADDR_OUTSIDE_SET));

    Serial.println("-----------------------");
    #endif
  }

} Context;

const uint8_t ContextSize = sizeof(Context);

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
