#ifndef neobeecontext_h
#define neobeecontext_h

#include <Arduino.h>
#include "neobeeTypes.h"
#include "neobeeWifi.h"
#include "neobeeMqtt.h"


typedef struct context {
  uint8_t magic_bytes[6];           // Must be NEOBEE for a valid data block
  uint8_t name[20];                 // Human readable name of the device
  uint8_t flags;
  uint16_t deep_sleep_seconds;      // Seconds to go to sleep

  Scale scale;
  Temperature temperature;
  MqttServer mqttServer;
  WifiNetwork wifi_network;  

  inline bool hasOffset() const { return bitRead(flags, FLAG_OFFSET_SET); };
  inline bool hasName() const { return _flag(FLAG_NAME_SET); };
  inline bool hasGain() const { return _flag(FLAG_GAIN_SET); };
  inline bool hasFactor() const { return _flag(FLAG_FACTOR_SET); };
  inline bool hasAddrInside() const { return _flag(FLAG_ADDR_INSIDE_SET); };
  inline bool hasAddrOutside() const { return _flag(FLAG_ADDR_OUTSIDE_SET); };
  inline bool isDeepSleepEnabled() const { return _flag(FLAG_DEEP_SLEEP_SET); };
  inline void enableDeepSleep() { bitSet(flags, FLAG_DEEP_SLEEP_SET); };
  inline void disableDeepSleep() { bitClear(flags, FLAG_DEEP_SLEEP_SET); };
  inline uint16_t getDeepSleepSeconds() { return deep_sleep_seconds > 0 ? deep_sleep_seconds : 30; }; 
  inline void setName(const uint8_t* new_name)
  {
    memcpy(name, new_name, sizeof(name));
    bitSet(flags, FLAG_NAME_SET);
  }

  inline bool _flag(uint8_t flag) const { return bitRead(flags, flag); };

} Context;

const uint8_t ContextSize = sizeof(Context);


#endif