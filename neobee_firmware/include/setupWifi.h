#ifndef setupWifi_h
#define setupWifi_h

#include <ESP8266WiFi.h>

#include "neobeeContext.h"
#include "neobeeThings.h"


WiFiMode setupWifi(Context& ctx, OperationMode& mode, NeoBeeLED& statusLed);

#endif