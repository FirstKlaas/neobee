#include "neobeeUtil.h"

#include <Arduino.h>

void saveContext(Context* ctx) {
    #ifdef DEBUG
    Serial.print("Saving Context");
    #endif
    EEPROM.begin(4095);
    EEPROM.put(0, *ctx);
    delay(200);
    EEPROM.commit();
    EEPROM.end();
    #ifdef DEBUG
    Serial.println(" - Done!");
    #endif
}

void resetContext(Context* ctx) {
    memcpy(ctx->magic_bytes, "NEOBEE", 6);
    ctx->flags=0;
    memset(ctx->name,0,sizeof(ctx->name));
    ctx->scale.offset = 0.f;
    ctx->scale.factor = 1.f;
    ctx->scale.gain = 128;
};

bool loadContext(Context* ctx) {
  EEPROM.begin(4095);
  EEPROM.get(0, *ctx);
  EEPROM.end();
  if (memcmp(ctx->magic_bytes, "NEOBEE", 6) == 0) {
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
    resetContext(ctx);
    #endif
    return false;
  }
}

void eraseContext(Context* ctx) {
    #ifdef DEBUG
    Serial.println("Deleting context");
    #endif
    memset(ctx->magic_bytes, 0, 7);
    ctx->flags = 0;
    saveContext(ctx);  
}

String stringFromByteAray(const uint8_t* src, const uint8_t size) {
  String dest = String();
  for (uint8_t i=0; i< size; i++) {
    if (src[i] == 0) break;
    dest += char(src[i]);
  };
  return dest;
}

String toHex8String(uint8_t *data, uint8_t size)
{
    // Create a buffer for storing the hex characters plus one 
    char tmp[(size << 1) + 1];
    for (byte i=0; i<size; i++) {
            sprintf(tmp+(i<<1), "%.2X",data[i]);
    }
    tmp[size<<1] = '\0';
    return String(tmp);
}
