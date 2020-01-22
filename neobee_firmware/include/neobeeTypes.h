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

// Known Flags

#define FLAG_OFFSET_SET           0
#define FLAG_FACTOR_SET           1
#define FLAG_GAIN_SET             2
#define FLAG_NAME_SET             3
#define FLAG_ADDR_INSIDE_SET      4
#define FLAG_ADDR_OUTSIDE_SET     5
#define DEEP_SLEEP_SET            6

// Data strucure to store relevant information
// between two deep sleep cycles
//
typedef struct {
  char host_name[32];
  uint16_t port;
} MqttServer;

typedef struct {
  double offset;
  float factor;
  uint8_t gain;
} Scale;

typedef struct {
  DeviceAddress addr_inside;
  DeviceAddress addr_outside;
} Temperature;

typedef struct context {
  char magic_bytes[6];
  char name[20];
  uint8_t flags;
  uint16_t deep_sleep_seconds;
  Scale scale;
  Temperature temperature;
  MqttServer mqttServer;

  context() {
    reset();
  };

  inline bool hasOffset() const { return bitRead(flags, FLAG_OFFSET_SET); };
  inline bool hasName() const { return _flag(FLAG_NAME_SET); };
  inline bool hasGain() const { return _flag(FLAG_GAIN_SET); };
  inline bool hasFactor() const { return _flag(FLAG_FACTOR_SET); };
  inline bool hasAddrInside() const { return _flag(FLAG_ADDR_INSIDE_SET); };
  inline bool hasAddrOutside() const { return _flag(FLAG_ADDR_OUTSIDE_SET); };

  void reset() {
    strncpy(magic_bytes, "NEOBEE", 6);
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
    if (strncmp(magic_bytes, "NEOBEE", 6) == 0) {
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
    Serial.print("Name        : ");
    Serial.println(name);
    
    Serial.print("Magic Bytes : ");
    Serial.println(magic_bytes);

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

#endif
