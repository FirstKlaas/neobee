#ifndef neobeeUtil_h
#define neobeeUtil_h

#include "neobeeContext.h"

void saveContext(Context* ctx);
bool loadContext(Context* ctx);
void eraseContext(Context* ctx);
void resetContext(Context* ctx);

#endif