#ifndef neobeeStorage_h
#define neobeeStorage_h

#include "neobeeTypes.h"

bool nbLoadCtx(Context& ctx);

void nbSaveCtx(const Context& ctx);

void nbPrintCtx(const Context& ctx);

void nbResetCtx(Context& ctx);

#endif
