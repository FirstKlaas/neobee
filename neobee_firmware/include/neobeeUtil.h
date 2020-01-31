#ifndef neobeeUtil_h
#define neobeeUtil_h

#include "neobeeContext.h"

void saveContext(Context* ctx);
bool loadContext(Context* ctx);
void eraseContext(Context* ctx);
void resetContext(Context* ctx);

String& copyFromByteAray(String& dest, const uint8_t* src, const uint8_t size);

String toHex8String(uint8_t *data, uint8_t size=1);

#endif