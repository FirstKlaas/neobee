#ifndef neobeecontext_h
#define neobeecontext_h

#include <Arduino.h>
#include "neobeeTypes.h"
#include "neobeeWifi.h"

enum class MqttFlags : uint8_t {
  FLAG_SSID_SET     = 0,
  FLAG_PASSWORD_SET = 1,
  FLAG_AUTH         = 2,
  FLAG_HOST_SET     = 3,
  FLAG_PORT_SET     = 4
};

// Data strucure to store relevant information
// between two deep sleep cycles
//
typedef struct {
  uint8_t flags;                    // Mqtt Flags
  char host_name[30];               // hostname or ip of the mqtt server
  uint16_t port;                    // mqtt port
  char login[30];                   // login (optional)
  char password[30];                // password (optional)

  bool hostnameSet() const { return host_name[0] != 0; };
  bool portSet() const { return port != 0; };
  bool loginSet() const { return login[0] != 0; };
  bool passwordSet() const { return password[0] != 0; };
  bool credentialsSet() const { return passwordSet() && loginSet(); };
  bool serverSet() const { return hostnameSet() && portSet(); };
  
  // Setter

  void setHostname(const uint8_t* src) {
    memcpy(host_name, src, sizeof(host_name));
  };

  void setPort(uint16_t new_port) {
    port = new_port;
  };

  void setLogin(const uint8_t* src) {
    memcpy(login, src, sizeof(login));
  };

  void setPassword(const uint8_t* src) {
    memcpy(password, src, sizeof(password));
  };

  void copyHostnameTo(uint8_t* dest) {
    memcpy(dest, host_name, sizeof(host_name));
  }

  void copyLoginTo(uint8_t* dest) {
    memcpy(dest, login, sizeof(login));
  }

  void copyPasswordTo(uint8_t* dest) {
    memcpy(dest, password, sizeof(password));
  }
  
} MqttServer;

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
  inline void setDeepSleepSeconds(uint8_t highbyte, uint8_t lowbyte) {
    deep_sleep_seconds = ((highbyte << 8) | lowbyte);  
  }  
  
  inline void setName(const uint8_t* new_name)
  {
    memcpy(name, new_name, sizeof(name));
    bitSet(flags, FLAG_NAME_SET);
  }

  inline bool _flag(uint8_t flag) const { return bitRead(flags, flag); };

} Context;

const uint8_t ContextSize = sizeof(Context);


#endif