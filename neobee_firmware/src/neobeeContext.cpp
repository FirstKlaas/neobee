#include "neobeeContext.h"
  
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

static const uint8_t MAGIC_BYTES[] PROGMEM = "NEOBEE";

void resetContext(Context* ctx) {
    memcpy_P(ctx->magic_bytes, MAGIC_BYTES, 6);
    memset(ctx->name,0,sizeof(ctx->name));

    ctx->wifi_network.reset();
    ctx->scale.reset();
    ctx->mqttServer.reset();
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
    #endif
    resetContext(ctx);
    return false;
  }
}

void eraseContext(Context* ctx) {
    #ifdef DEBUG
    Serial.println("Deleting context");
    #endif
    resetContext(ctx);
    memset(ctx->magic_bytes, 0, 7);
    saveContext(ctx);  
}
