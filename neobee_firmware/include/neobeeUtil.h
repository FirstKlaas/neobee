#ifndef neobeeUtil_h
#define neobeeUtil_h

#include "Arduino.h"

void writeUInt32(uint32_t value, uint8_t* dst);

void writeInt32(int32_t value, uint8_t* dst);

int32_t readInt32(uint8_t* dst);

uint32_t readUInt32(uint8_t* dst);

String stringFromByteAray(const uint8_t* src, const uint8_t size=1);

String toHex8String(uint8_t *data, uint8_t size=1);

void writeFloat100(float val, uint8_t* dest);

void writeDouble100(double val, uint8_t* dest);

float readFloat100(uint8_t* dst);

double readDouble100(uint8_t* dst);

inline void printByteArray(const uint8_t* buffer, const uint8_t size=32) {

    #ifdef DEBUG
    for (uint8_t i=0; i<size; i++) {
        Serial.print(buffer[i], HEX);
        if (i<size) Serial.print(":");
    }
    Serial.println();
    #endif

}

#endif