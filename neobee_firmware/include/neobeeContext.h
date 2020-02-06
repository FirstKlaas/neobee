#ifndef neobeecontext_h
#define neobeecontext_h

#include <Arduino.h>
#include "neobeeTypes.h"
#include "neobeeWifi.h"
#include "neobeeUtil.h"

enum class MqttFlags : uint8_t {
};

// Data strucure to store relevant information
// between two deep sleep cycles
//
typedef struct {
  uint8_t reserved;                    // Mqtt Flags (currently unused)
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

  // Getter
  String getHostName() {
    return stringFromByteAray((uint8_t*) host_name, sizeof(host_name));
  };

  String getLogin() {
    return stringFromByteAray((uint8_t*) login, sizeof(login));
  };

  String getPassword() {
    return stringFromByteAray((uint8_t*) password, sizeof(password));
  };

  void setHostname(const uint8_t* src) {
    memcpy(host_name, src, sizeof(host_name));
  };

  void clearHostname() {
    memset(host_name, 0, sizeof(host_name));
  };

  void setPort(uint16_t new_port) {
    port = new_port;
  };

  void clearPort() {
    setPort(0);
  }

  void clearLogin() {
    memset(login, 0, sizeof(login));
  };

  void setLogin(const uint8_t* src) {
    memcpy(login, src, sizeof(login));
  };

  void clearPassword() {
    memset(password, 0, sizeof(password));
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
  uint8_t reserved;
  uint16_t deep_sleep_seconds;      // Seconds to go to sleep

  Scale scale;
  Temperature temperature;
  MqttServer mqttServer;
  WifiNetwork wifi_network;  

  inline bool hasName() const { return name[0] != 0; };
  inline bool isDeepSleepEnabled() const { return (deep_sleep_seconds > 0); };
  inline uint16_t getDeepSleepSeconds() { return deep_sleep_seconds > 0 ? deep_sleep_seconds : 30; };
  inline void setDeepSleepSeconds(uint8_t highbyte, uint8_t lowbyte) {
    deep_sleep_seconds = ((highbyte << 8) | lowbyte);  
  }  
  
  inline void setName(const uint8_t* new_name)
  {
    memcpy(name, new_name, sizeof(name));
  }
} Context;

const uint8_t ContextSize = sizeof(Context);

void saveContext(Context* ctx);
bool loadContext(Context* ctx);
void eraseContext(Context* ctx);
void resetContext(Context* ctx);

#endif