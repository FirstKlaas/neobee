/***********************************************************
 * NeoBee - WiFi Setup
 * 
 * @author: Klaas Nebuhr <klaas.nebuhr@gmail.com>
 * Copyright 2020
 **********************************************************/
#include "setupWifi.h"
#include <WString.h>

#define MAX_WIFI_CONNECT_TRIES 50

static const char AP_SSID[]  PROGMEM = "NeoBee";

WiFiMode setupWifi(Context& ctx, OperationMode& mode, NeoBeeLED& statusLed) {
    if (WiFi.isConnected()) return WiFiMode::WIFI_STA;
    
    WiFi.disconnect();
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    // Check, if we have all the information, to connect to 
    // a wifi network.
    if(ctx.wifi_network.canConnect()) {
        uint8_t number_of_tries(0);

        WiFi.setAutoConnect(true);
        WiFi.setAutoReconnect(true);
        WiFi.mode(WIFI_STA);

        #ifdef DEBUG
        Serial.print("Trying to connect to ");
        Serial.println(ctx.wifi_network.ssid);
        Serial.print("Using password: ");
        Serial.println(ctx.wifi_network.password);
        #endif

        // Try to connect to configured network.
        // If the connection fails, we will fallback
        // tp AP (Access Point) mode.
        WiFi.begin(ctx.wifi_network.ssid, ctx.wifi_network.password);
        while (!WiFi.isConnected() && number_of_tries < MAX_WIFI_CONNECT_TRIES) {
        #ifdef DEBUG
        Serial.print(".");
        #endif
        // When in COMMAND_MODE, we can use the LED to indicate
        // that we are trying to connect to the wifi network.
        // In IOT Mode we don't to reduce the power consumtion.
        if (mode == OperationMode::CMD_MODE) {
            statusLed.pulse(250,1,250);
        } else {
            delay(500);
        };
        number_of_tries++;
        };


        // Print some information for the user.
        #ifdef DEBUG
        Serial.println();
        if (WiFi.isConnected()) {
            Serial.print("Succesfully connected to wifi after ");
            Serial.print(number_of_tries);
            Serial.println(" tries.");
            Serial.print("NeoBee IP is ");
            Serial.println(WiFi.localIP());
      
        }
        #endif

        if (WiFi.isConnected()) {
            // Sucessfully connected to a local network
            return WiFiMode::WIFI_STA;
        }

    } else {
        // If we cannot connect, then call explicitly WiFI.disconnect.
        // Without disconnecting, the STA mode will interfere the AP
        // mode. No idea why, but there are several threads covering
        // this topic.
        #ifdef DEBUG
        Serial.println("No wifi information. Going directly into AP mode.");
        #endif
        WiFi.disconnect();
        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(false);
        
    };


    // Now activating AP mode.
    statusLed.switchOn();

    // Connecting to the network maybe failed.
    // or no network settings have been provided.
    // Starting as an accesspoint.
    #ifdef DEBUG
    Serial.println("Acting as an accesspoint.");
    #endif 

    WiFi.mode(WIFI_AP);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
        
    return (WiFi.softAP(FPSTR(AP_SSID)) ? WiFiMode::WIFI_AP : WiFiMode::WIFI_OFF);
}