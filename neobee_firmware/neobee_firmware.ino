/****************************************
 * neobee
 * 
 * Reading sensor data from a hive and 
 * push it to an mqtt broker. 
 * 
 * For more information about this project
 * check out the github page:
 * https://github.com/FirstKlaas/neobee
 *
 * (c) 2020 FirstKlaas <klaas.nebuhr@gmail.com>
 */

#include "HX711.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define SERIAL_SPEED 115200

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

// The pin, the data wire of the Dallas sensor is connected to.
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// DeepSleep time in microseconds
// If you set the number to 0, you need to trigger the
// restart via the RST pin by setting it somehow to LOW
const int DEEP_SLEEP_SECONDS = 30e6 // Equivalent to 30 seconds

// The calibration factor has to be determined
// for each load call and hx711 module combination
// individually. This library has no support for
// calibration. Check out the github page for more information.
const float CALIBRATION_FACTOR = 2280.f;

// The HX711 scale instance
HX711 scale;

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;

#define DEBUG
 
void setup() {

  #ifdef DEBUG
    Serial.begin(SERIAL_SPEED)
  #endif
  
  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Start the ds18b20 sensors to work
  sensors.begin();


  // Get the adresses for the dallas sensors
  if (!sensors.getAddress(insideThermometer, 0)) {
    #ifdef DEBUG
      Serial.println("Unable to find address for Device 0");
    #endif
  }
  if (!sensors.getAddress(outsideThermometer, 1)) {
    #ifdef DEBUG
      Serial.println("Unable to find address for Device 1");
    #endif
  }
}

void loop() {
}
