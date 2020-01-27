/************************************************
 * neobee - The Smart Hive
 * 
 * Reading sensor data from a hive and 
 * push it to an mqtt broker. 
 * 
 * For more information about this project
 * check out the github page:
 * https://github.com/FirstKlaas/neobee
 *
 * (c) 2020 FirstKlaas <klaas.nebuhr@gmail.com>
 ************************************************/

#include "neobee.h"
#include "neobeeUtil.h"

#define ITERATIONS 10
#define SERIAL_SPEED 9600
#define MAX_WIFI_CONNECT_TRIES 50

Context ctx;

NeoBeeTemperature temperature(ctx);
NeoBeeScale scale(ctx);
NeoBeeCmd cmd(ctx, scale, temperature);
Adafruit_NeoPixel pixels(1, D1, NEO_GRB + NEO_KHZ800);

OperationMode mode = OperationMode::AP_MODE;

#define NBWiFi      ctx.wifi_network
#define MEMSTART    0
#define EEPROM_SIZE 4096

uint8_t test[6];

void setup() {
  WiFi.disconnect();
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  
  #ifdef DEBUG
    Serial.begin(SERIAL_SPEED);
    delay(1000);
    while(!Serial) {
      // Wait for serial connection
    };
    delay(500);
    Serial.println("#############################################");
    Serial.println("# NeoBee - Hive Data Logger                 #");
    Serial.println("#############################################");
  #endif

  if (loadContext(&ctx)) {
    #ifdef DEBUG
    Serial.println("Context restored");
    Serial.print("Wifi Flags: ");
    Serial.println(ctx.wifi_network.flags);
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("Context created");
    #endif
  }
  
  #ifdef DEBUG
  Serial.print("Configuration Size: ");
  Serial.print((uint8_t) sizeof(ctx));
  Serial.println(" bytes");
  #endif

  if(ctx.wifi_network.canConnect()) {
    uint8_t number_of_tries(0);

    WiFi.mode(WIFI_STA);

    #ifdef DEBUG
    Serial.print("Trying to connect to ");
    Serial.println(NBWiFi.ssid);
    Serial.print("Using password: ");
    Serial.println(NBWiFi.password);
    #endif
    WiFi.begin(NBWiFi.ssid, NBWiFi.password);
    while (!WiFi.isConnected() && number_of_tries < MAX_WIFI_CONNECT_TRIES) {
      #ifdef DEBUG
      Serial.print(".");
      #endif
      delay(500);
      number_of_tries++;
    };
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
  } else {
    #ifdef DEBUG
    Serial.println("No wifi information. Going directly into AP mode.");
    #endif
    WiFi.disconnect();
  };

  // Connecting to the network maybe failed.
  // or no network settings have been provided.
  // Starting as an accesspoint.
  if (!WiFi.isConnected()) {  
    #ifdef DEBUG
    Serial.println("Acting as an accesspoint.");
    #endif 
    if (NBWiFi.canConnect()) {
      #ifdef DEBUG
      Serial.println("Could not connect. Switching to AP mode.");
      #endif 
    }
    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP("BumbleBees");
    #ifdef DEBUG
    Serial.println(success);
    #endif 
    
    #ifdef DEBUG
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    #endif 
  }

  cmd.begin(); 
}

void loop() {
  cmd.checkForCommands();
}
