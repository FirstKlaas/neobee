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
#include "neobeeMqtt.h"
#include "setupWifi.h"

#define ITERATIONS 10
#define SERIAL_SPEED 9600
#define STATUS_PIN D8
#define RESET_BTN_PIN D0
#define CMD_BTN_PIN D5

Context ctx;

NeoBeeMqtt mqtt = NeoBeeMqtt(ctx);
NeoBeeTemperature temperature(ctx);
NeoBeeScale scale(ctx);
NeoBeeCmd cmd(ctx, scale, temperature);
NeoBeeLED statusLed = NeoBeeLED(STATUS_PIN);
NeoBeeButton resetButton = NeoBeeButton(RESET_BTN_PIN);
NeoBeeButton cmdButton = NeoBeeButton(CMD_BTN_PIN);

// The operation mode for the board. We start with
// the assumption, that we can read sensor data and 
// broadcast the data via mqtt. 
OperationMode mode = OperationMode::IOT_MODE;
WiFiMode wifiMode;

#define NBWiFi      ctx.wifi_network
#define MEMSTART    0
#define EEPROM_SIZE 4096
#define LED_PIN     D8

void setup() {

    // Start optimistic ;)
    mode = OperationMode::IOT_MODE;

    #ifdef DEBUG
    Serial.begin(SERIAL_SPEED);
    //delay(1000);
    while(!Serial) {
        // Wait for serial connection
    };
    delay(50);
    Serial.println("#############################################");
    Serial.println("# NeoBee - Hive Data Logger                 #");
    Serial.println("#############################################");
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    #endif

    // Check, if the command mode button is pressed.
    // If pressed, the mode is set to CMD_MODE
    if (cmdButton.isPressed()) {
        #ifdef DEBUG
        Serial.println("CMD Button pressed during boot.");
        #endif
        mode = OperationMode::CMD_MODE;  
        statusLed.pulse(200,10,50);  
    } else {
        #ifdef DEBUG
        Serial.println("CMD Button not pressed");
        #endif
    }; 


    // Initialize the configuration data
    if (loadContext(&ctx)) {
        #ifdef DEBUG
        Serial.println("Context restored");
        Serial.println(stringFromByteAray((uint8_t*)ctx.wifi_network.ssid, 30));
        Serial.println(stringFromByteAray((uint8_t*)ctx.wifi_network.password, 30));
        #endif
    } else {
        #ifdef DEBUG
        Serial.println("Context created");
        #endif
    }

    // Start the sensors
    temperature.begin();
    scale.begin();
  
    // Now setup the wifi network
    wifiMode = setupWifi(ctx, mode, statusLed);

    if (wifiMode == WiFiMode::WIFI_AP) {
        // When in AP mode, sending sensor data
        // doesn't make any sense.
        // Command mode is the only possible
        // mode.
        #ifdef DEBUG
        Serial.println("In AP mode no mqtt messages are broadcasted.");
        Serial.print("Local IP is: ");
        Serial.println(WiFi.softAPIP());
        #endif

        if (mode != OperationMode::CMD_MODE) {
            #ifdef DEBUG
            Serial.println("Wifi is in AP mode. So switching to CMD mode.");
            #endif
            mode = OperationMode::CMD_MODE;
        };

    } else {
        // We are in station mode, so it does make sense,
        // to connect to the mqtt server (if configured)
        #ifdef DEBUG
        Serial.println("We are in station mode. Checking mqtt.");
        #endif
        mqtt.connect();
    };

    // Now starting the command mode. (if we
    // are in the right mode)
    if (mode == OperationMode::CMD_MODE) {
        cmd.begin(); 
    };
  
    // =================================================
    // Now check for IOT Mode and deepSleep
    // =================================================
    if (mode == OperationMode::IOT_MODE) {
        #ifdef DEBUG
        Serial.print("Going to deep sleep for ");
        Serial.print(ctx.getDeepSleepSeconds());
        Serial.println(" seconds.");
        #endif
        mqtt.publishData(scale.getWeight(), temperature.getCTemperatureByIndex(0), temperature.getCTemperatureByIndex(1));
        delay(1000); // Needs a delay, so the message is sent.
        ESP.deepSleep(ctx.getDeepSleepSeconds() * 1E6 - (millis()) * 1E3);
    };


    // Turn the status led on
    // to give feedback ti the user, that we are in
    // command mode.
    statusLed.switchOn();
};

void loop() {
    static unsigned long currentMillis;

    // If we are in command mode check for 
    // new commands.
    if (mode == OperationMode::CMD_MODE) {
        cmd.checkForCommands();
    }

    // When in AP Mode, mqtt doesn't make any sense.
    if (wifiMode != WiFiMode::WIFI_AP) {        
        // Use a non blocking method to send
        // sensor data every 5 seconds
        if (millis() - currentMillis >= 5000) {
            currentMillis = millis();
            if (mqtt.isConnected()) {
            #ifdef DEBUG
            Serial.println("Measure and delay");
            #endif
            mqtt.publishData(scale.getWeight(), temperature.getCTemperatureByIndex(0), temperature.getCTemperatureByIndex(1));
            }
        } 
    };
}
