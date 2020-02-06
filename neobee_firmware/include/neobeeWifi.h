#ifndef neobeewifi_h
#define neobeewifi_h

#include <Arduino.h>
#include "neobeeTypes.h"

typedef struct wifi_network {

    uint8_t reserved;                 // Wifi flags
    char ssid[31];                    // 0-terminated name of the wifi network
    char password[31];                // 0-terminated password
    uint8_t channel;

    inline bool canConnect() const {
        return hasPassword() & hasSSID();
    }

    inline bool hasPassword() const {
        return password[0] > 0;
    }

    inline void setPassword(const uint8_t* data) {
        const uint8_t size = sizeof(password)-1;
        memcpy(password, data, size);
        password[size] = 0; // Make shure the array is 0 terminated
    }

    inline void getPassword(uint8_t* dst) const {
        const uint8_t size = sizeof(password)-1;
        if (hasPassword()) memcpy(dst, password, size);
    }

    inline void clearPassword() {
        memset(password, 0, sizeof(password));
    }

    inline bool hasSSID() const {
        return ssid[0] > 0;
    }
  
    // TODO: If the first byte is 0, we have an invalid ssid.
    // Should be the same like clear_ssid
    inline void setSSID(const uint8_t* data) {
        const uint8_t size = sizeof(ssid)-1;
        memcpy(ssid, data, size);
        ssid[size] = 0;
    }

    inline bool getSSID(uint8_t* dst) const {
        const uint8_t size = sizeof(ssid)-1;
        if (hasSSID()) {
            memcpy(dst, ssid, size);
            return true;
        } else {
            return false;
        }
    }

  inline void clearSSID() {
    memset(ssid, 0, sizeof(ssid));
  }

  inline void reset() {
    memset(ssid, 0, sizeof(ssid));
    memset(password, 0, sizeof(password));
    channel = 1;
  }
  
} WifiNetwork;

#endif