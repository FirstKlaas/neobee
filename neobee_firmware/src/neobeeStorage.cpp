#include "neobeeStorage.h"

#include <EEPROM.h>


bool nbLoadCtx(Context& ctx) {
  EEPROM.begin(4095);
  EEPROM.get(0, ctx);
  EEPROM.end();
  if (strncmp(ctx.magic_bytes, "NEOBEE", 6) == 0) {
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
    nbResetCtx(ctx); 
    return false;
  }
}

void nbDeleteCtx(Context& ctx) {
  #ifdef DEBUG
  Serial.println("Deleting context");
  #endif
  memset(ctx.magic_bytes, 0, 7);
  ctx.flags = 0;
  nbSaveCtx(ctx);  
}

void nbResetCtx(Context& ctx) {
  #ifdef DEBUG
  Serial.println("Initialising context with defaults.");
  #endif
  strncpy(ctx.magic_bytes, "NEOBEE\0", 7);
  memset(ctx.name, 0, 20);
  ctx.flags = 0;    
  ctx.SCALE_OFFSET = 0;
  ctx.SCALE_FACTOR = 1.f;
  ctx.SCALE_GAIN = 128;  
  nbSaveCtx(ctx);  
}

void nbSaveCtx(const Context& ctx) {
  #ifdef DEBUG
  Serial.print("Saving Context");
  #endif
  EEPROM.begin(4095);
  EEPROM.put(0, ctx);
  delay(200);
  EEPROM.commit();
  EEPROM.end();
  #ifdef DEBUG
  Serial.println(" - Done!");
  #endif
  }

void nbPrintCtx(const Context& ctx) {
  Serial.print("Name        : ");
  Serial.println(ctx.name);
  
  Serial.print("Magic Bytes : ");
  Serial.println(ctx.magic_bytes);

  Serial.print("Offset      : ");
  Serial.println(ctx.SCALE_OFFSET);
  
  Serial.print("Factor      : ");
  Serial.println(ctx.SCALE_FACTOR);

  Serial.println("");
  Serial.println("-----------------------");
  Serial.println("Flags");
  Serial.println("-----------------------");

  Serial.print("Offset set  : ");
  Serial.println(bitRead(ctx.flags, FLAG_OFFSET_SET));

  Serial.print("Factor set  : ");
  Serial.println(bitRead(ctx.flags, FLAG_OFFSET_SET));

  Serial.print("Gain set    : ");
  Serial.println(bitRead(ctx.flags, FLAG_GAIN_SET));

  Serial.print("Name set    : ");
  Serial.println(bitRead(ctx.flags, FLAG_NAME_SET));
  
  Serial.print("T1 set      : ");
  Serial.println(bitRead(ctx.flags, FLAG_T1_SET));
  
  Serial.print("T2 set      : ");
  Serial.println(bitRead(ctx.flags, FLAG_T2_SET));

  Serial.println("-----------------------");
}
