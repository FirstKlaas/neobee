#ifndef neobeewifi_h
#define neobeewifi_h

#include <Arduino.h>
#include "neobeeTypes.h"

typedef struct wifi_network {

    uint8_t flags;                    // Wifi flags
    char ssid[31];                    // 0-terminated name of the wifi network
    char password[31];                // 0-terminated password
    uint8_t channel;

    inline bool canConnect() const {
        return hasPassword() & hasSSID() & isActive();
    }

    inline bool isActive() const {
        #ifdef DEBUG
        Serial.print("Is Active: ");
        Serial.println(bitRead(flags, int(WifiFlags::FLAG_ACTIVE)));
        #endif
        return bitRead(flags, int(WifiFlags::FLAG_ACTIVE));
    }

    inline bool hasPassword() const {
        #ifdef DEBUG
        Serial.print("Has password: ");
        Serial.println(bitRead(flags, int(WifiFlags::FLAG_PASSWORD_SET)));
        #endif

        return bitRead(flags, int(WifiFlags::FLAG_PASSWORD_SET));
    }

    inline void setPassword(const uint8_t* data) {
        const uint8_t size = sizeof(password)-1;
        memcpy(password, data, size);
        password[size] = 0; // Make shure the array is 0 terminated
        bitSet(flags, int(WifiFlags::FLAG_PASSWORD_SET));
    }

    inline void getPassword(uint8_t* dst) const {
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
        #ifdef DEBUG
        Serial.print("Setting ssid ");
        Serial.print(ssid);
        Serial.print(" flag = ");
        Serial.println(bitRead(flags, int(WifiFlags::FLAG_SSID_SET)));
        #endif
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
    bitClear(flags, int(WifiFlags::FLAG_SSID_SET));
  }

  inline void reset() {
    flags = 0;
    memset(ssid, 0, sizeof(ssid));
    memset(password, 0, sizeof(password));
    channel = 1;
  }
  
} WifiNetwork;

#endif