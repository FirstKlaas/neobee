/*****************************************************************************
 * NeoBee Temperature Class
 * 
 * author: Klaas Nebuhr
 * **************************************************************************/
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


NeoBeeTemperature::NeoBeeTemperature(Context &ctx):
m_ctx(ctx), m_has_started(false)
{
}

NeoBeeTemperature::~NeoBeeTemperature() {
}

void NeoBeeTemperature::begin() {
  if (m_has_started) return;
  m_has_started = true;

  #ifdef DEBUG
    Serial.println("Starting DS18B20");
  #endif

  sensors.begin();
  delay(200);
  // Todo: Muss ich das noch machen, wenn ich einmal die Adressen gefunden habe?
  // Wenn die Adressen einmal ermittelt wurden, sind diese im context hinterlegt.
  // Was ist, wenn der zweite Sensor nicht angeschlossen ist? Dann wäre das Flag nicht
  // gesetzt und man wuerde hier erneut versuchen, die Adressen zu ermitteln.
  //bitWrite(m_ctx.flags, FLAG_ADDR_INSIDE_SET, sensors.getAddress(m_ctx.temperature.addr_inside, 0));
  //bitWrite(m_ctx.flags, FLAG_ADDR_OUTSIDE_SET, sensors.getAddress(m_ctx.temperature.addr_outside, 1));
  //sensors.getAddress(m_ctx.temperature.addr_inside, 0);
  //sensors.getAddress(m_ctx.temperature.addr_outside, 1);
  
  #ifdef DEBUG
  Serial.print("Device count ");
  Serial.println(sensors.getDeviceCount());
  oneWire.search(m_ctx.temperature.addr_inside);
  oneWire.search(m_ctx.temperature.addr_outside);
  oneWire.reset_search();
  if (sensors.getDeviceCount() > 0) {
    printByteArray(m_ctx.temperature.addr_inside,8);
    printByteArray(m_ctx.temperature.addr_outside,8);
  } else {
    Serial.println("No temperature sensors found.");
  }
  #endif
  
}

uint8_t NeoBeeTemperature::getDeviceCount() {
  return sensors.getDS18Count();
}

float NeoBeeTemperature::getCTemperatureByIndex(const uint8_t index) {
    begin();
    if (sensors.getDeviceCount() == 0) {
      #ifdef DEBUG
      Serial.println("No temperature sensors present. Ignoring call and returning 0.");
      return 0.0f;
      #endif
    }

    sensors.requestTemperatures();
    return sensors.getTempCByIndex(index);
}