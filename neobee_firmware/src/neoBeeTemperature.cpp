#include "neobeeTemperature.h"

// The pin, the data wire of the Dallas sensor is connected to.
#define ONE_WIRE_BUS D4

#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;


NeoBeeTemperature::NeoBeeTemperature(Context &ctx): m_ctx(ctx) {
}

NeoBeeTemperature::~NeoBeeTemperature() {
}

void NeoBeeTemperature::begin() {
  #ifdef DEBUG
    Serial.println("Starting DS18B20");
  #endif

  sensors.begin();
  delay(200);
  // Todo: Muss ich das noch machen, wenn ich einmal die Adressen gefunden habe?
  bitWrite(m_ctx.flags, FLAG_T1_SET, sensors.getAddress(m_ctx.addr_inside, 0));
  bitWrite(m_ctx.flags, FLAG_T2_SET, sensors.getAddress(m_ctx.addr_inside, 1));
}

bool NeoBeeTemperature::hasInsideSensorAddress() {
  return bitRead(m_ctx.flags, FLAG_T1_SET) == 1;
}
