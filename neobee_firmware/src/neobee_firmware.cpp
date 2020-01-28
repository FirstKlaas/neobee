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
#include "neobeeThings.h"

#define ITERATIONS 10
#define SERIAL_SPEED 9600
#define MAX_WIFI_CONNECT_TRIES 50
#define AP_SSID "NeoBee"
#define STATUS_PIN D8
#define RESET_BTN_PIN D4
#define CMD_BTN_PIN D5

Context ctx;

NeoBeeTemperature temperature(ctx);
NeoBeeScale scale(ctx);
NeoBeeCmd cmd(ctx, scale, temperature);
NeoBeeLED statusLed = NeoBeeLED(STATUS_PIN);
NeoBeeButton resetButton = NeoBeeButton(RESET_BTN_PIN);
NeoBeeButton cmdButton = NeoBeeButton(RESET_BTN_PIN);

// The operation mode for the board. We start with
// the assumption, that we can read sensor data and 
// broadcast the data via mqtt. 
OperationMode mode = OperationMode::IOT_MODE;

#define NBWiFi      ctx.wifi_network
#define MEMSTART    0
#define EEPROM_SIZE 4096
#define LED_PIN D8

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

  // Check, if the command mode button is pressed.
  // If pressed, the mode is set to CMD_MODE
  if (cmdButton.isPressed()) {
    mode = OperationMode::CMD_MODE;  
    statusLed.pulse(200,5,50);  
  }; 


  #ifdef DEBUG
  mode = OperationMode::CMD_MODE;
  #endif

  // Initialize the configuration data
  if (loadContext(&ctx)) {
    #ifdef DEBUG
    Serial.println("Context restored");
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

  // Check, if we have all the information, to connect to 
  // a wifi network.
  if(ctx.wifi_network.canConnect()) {
    uint8_t number_of_tries(0);

    WiFi.mode(WIFI_STA);

    #ifdef DEBUG
    Serial.print("Trying to connect to ");
    Serial.println(NBWiFi.ssid);
    Serial.print("Using password: ");
    Serial.println(NBWiFi.password);
    #endif

    // Try to connect to configured network.
    // If the connection fails, we will fallback
    // tp AP (Access Point) mode.
    WiFi.begin(NBWiFi.ssid, NBWiFi.password);
    while (!WiFi.isConnected() && number_of_tries < MAX_WIFI_CONNECT_TRIES) {
      #ifdef DEBUG
      Serial.print(".");
      #endif
      // Led On and Led Off for 250 ms. So we have delay of
      // 250 ms in total
      statusLed.pulse(250,1,250);
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
      statusLed.pulse(1000,3,250);
    }
    #endif
  } else {
    // If we cannot connect, then call explicitly WiFI.disconnect.
    // Without disconnecting, the STA mode will interfere the AP
    // mode. No idea why, but there are several threads covering
    // this topic.
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
    // So, we are not connected. If we have all the
    // information to connect, but still are not connected,
    // then the connection failed. 
    if (NBWiFi.canConnect()) {
      #ifdef DEBUG
      Serial.println("Could not connect. Switching to AP mode.");
      #endif 
    }
    // Now activating Access Point
    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP(AP_SSID);
    #ifdef DEBUG
    Serial.println(success);
    #endif 
    
    // switching status LED on permanently, to show,
    // acess point is active.
    statusLed.switchOn();
    
    #ifdef DEBUG
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    #endif 
  }

  // Now starting the command mode. (if we
  // are in the right mode)
  if (mode == OperationMode::CMD_MODE) {
    cmd.begin(); 
  } else {
    // Now we have two szenarios. If deepsleep
    // is enabled, we send iot data and go to sleep.
    // If not, we send the data in the loop.
    #ifdef DEBUG
    Serial.println("Measure and DeepSleep");
    #endif
  };
};

void loop() {

  // If we are in command mode check for 
  // new commands.
  if (mode == OperationMode::CMD_MODE) { 
    cmd.checkForCommands();
  } else {
    #ifdef DEBUG
    Serial.println("Measure and delay");
    #endif
    delay(10000);
  }
}
