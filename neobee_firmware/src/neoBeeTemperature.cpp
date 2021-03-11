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
m_ctx(ctx), m_has_started(false), sensor_count(0)
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

  // Clear flags
  m_ctx.temperature.flags = 0;

  sensors.begin();
  delay(200);
  
  // Search for the first device
  if (oneWire.search(m_ctx.temperature.addr_inside)) {
    sensor_count++;
    if (oneWire.search(m_ctx.temperature.addr_outside)) {
      sensor_count++;
    } 
  }

  oneWire.reset_search();
  
  #ifdef DEBUG
  Serial.print("Device count ");
  Serial.println(sensor_count);
  
  if (sensor_count > 0) {
    printByteArray(m_ctx.temperature.addr_inside,8);
    printByteArray(m_ctx.temperature.addr_outside,8);
  } else {
    Serial.println("No temperature sensors found.");
  }
  #endif
  
}

uint8_t NeoBeeTemperature::getDeviceCount() {
  return sensor_count;
  //return sensors.getDS18Count();
}

float NeoBeeTemperature::getCTemperatureInside() {
  if (sensor_count == 0) return 0.0;
  float temp(sensors.getTempC(m_ctx.temperature.addr_inside));
  if (temp == 85.0) return 0.0;
  return temp;
}

float NeoBeeTemperature::getCTemperatureOutside() {
  if (sensor_count < 2) return 0.0;
  float temp(sensors.getTempC(m_ctx.temperature.addr_outside));
  if (temp == 85.0) return 0.0;
  return temp;
}

float NeoBeeTemperature::getCTemperatureByIndex(const uint8_t index) {
    begin();
    if (sensors.getDeviceCount() == 0) {
      #ifdef DEBUG
      Serial.println("No temperature sensors present. Ignoring call and returning 0.");
      #endif
      return 0.0f;
    }

    if (index >= sensors.getDeviceCount()) {
      #ifdef DEBUG
      Serial.println("Requested temperature: Index to high. No such sensor.");
      #endif
      return 0.0f;
    }

    sensors.requestTemperatures();
    
    #ifdef DEBUG
    Serial.print("Temperature for index ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(sensors.getTempCByIndex(index));
    #endif

    return sensors.getTempCByIndex(index);
}