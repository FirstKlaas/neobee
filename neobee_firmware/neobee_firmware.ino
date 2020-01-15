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

#define SERIAL_SPEED 115200

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

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
}

void loop() {
}
