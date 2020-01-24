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

#define ITERATIONS 10

#define SERIAL_SPEED 9600

// HX711 circuit wiring

// DeepSleep time in microseconds
// If you set the number to 0, you need to trigger the
// restart via the RST pin by setting it somehow to LOW
const int DEEP_SLEEP_SECONDS = 30e6; // Equivalent to 30 seconds

// The calibration factor has to be determined
// for each load call and hx711 module combination
// individually. This library has no support for
// calibration. Check out the github page for more information.
//const float scaleFactor = 21.8687258687f;
const float scaleFactor = 21.66f;

Context ctx;

NeoBeeTemperature temperature(ctx);
NeoBeeScale scale(ctx);
NeoBeeCmd cmd(ctx, scale, temperature);
Adafruit_NeoPixel pixels(1, D1, NEO_GRB + NEO_KHZ800);


void setupScaleOffset() {
  #ifdef DEBUG
  Serial.println("Setting up the offset of the scale (taring.");
  Serial.println("Make sure, the scale is 'empty'. You've got 10 seconds to clear.");
  #endif

  for (uint8_t i=0; i<10; i++) {
    delay(1000);
    #ifdef DEBUG
    Serial.print("*");
    #endif
  }
  #ifdef DEBUG
  Serial.println("");
  Serial.println("Reading offset. (Taring the scale.)");
  #endif
  scale.tare(ITERATIONS);
  #ifdef DEBUG
  Serial.print("Offset is: ");
  #endif
  Serial.println(scale.getOffset());
}

void setupScaleFactor() {

  #ifdef DEBUG
  Serial.println("Now calibrating the scale.");
  Serial.println("Please put the reference weight onto the scale.");
  Serial.println("You've got 10 seconds.");
  #endif

  for (uint8_t i=0; i<10; i++) {
    delay(1000);
    #ifdef DEBUG
    Serial.print("*");
    #endif
  }
  #ifdef DEBUG
  Serial.println("");
  #endif
  scale.calibrate(ITERATIONS);
 }

void setup() {

  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0,151,0));
  pixels.show();
  
  //bool contextChanged = false;
  
  #ifdef DEBUG
    Serial.begin(SERIAL_SPEED);
    delay(1000);
    while(!Serial) {
    };
    Serial.println("#############################################");
    Serial.println("# NeoBee - Hive Data Logger                 #");
    Serial.println("#############################################");
  #endif

  if (ctx.load()) {
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

  //ctx.flags = 0;
  /**
  scale.begin();
  temperature.begin();
  
  if (!scale.hasOffset()) {
    setupScaleOffset();
    contextChanged = true;
  };

  if (!scale.hasFactor()) {
    setupScaleFactor();
    contextChanged = true;
  };
  if (contextChanged) {
    ctx.save();
  }
  ctx.print(); 
  ESP.deepSleep(DEEP_SLEEP_SECONDS);
  **/
 cmd.begin(); 
}

void loop() {
  cmd.checkForCommands();
  /**
  Serial.print("Weight: ");
  Serial.println(scale.getWeight(ITERATIONS));
  Serial.println("------------------------");
  delay(5000);
  **/
}
