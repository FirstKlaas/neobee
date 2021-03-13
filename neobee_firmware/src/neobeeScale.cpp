#include "neobeeScale.h"

#include "arduinosort.h"
#include "HX711.h"

// The HX711 scale instance
HX711 _scale;

long _values[15];

NeoBeeScale::NeoBeeScale(Context& ctx): 
m_ctx(ctx), _has_started(false)
{
}

NeoBeeScale::~NeoBeeScale() {}

bool NeoBeeScale::hasStarted() {
  return _has_started;
}
bool NeoBeeScale::begin() {

  if (hasStarted()) return false;

  #ifdef DEBUG
    Serial.println("Starting scale");
  #endif

  _scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);  

  /**
   * New in Version 0.1.5
   * 
   * Test, if scale is present. Using a timeout of 5 seconds and
   * a delay of 200 ms. If the scale is not ready, the scale will
   * be ignored. The prior versions used the "ready" function, which
   * is a blocking version of the function.
   **/

  if (_scale.wait_ready_timeout(5000,200)) {
    _has_started = true;
    _scale.set_offset(m_ctx.scale.offset);
    _scale.set_scale(m_ctx.scale.factor);
    #ifdef DEBUG
    Serial.println("Scale started successfully.");
    #endif
    return true;
  };

  #ifdef DEBUG
    Serial.println("Cannot start scale. Check hardware and wiring.");
  #endif

  _has_started = false;
  return false;
}

float NeoBeeScale::getWeight(uint8_t ntimes, WeightMethod method) {
    if (!hasStarted()) {
      #ifdef DEBUG
      Serial.println("No scale present. Returning 0.0");
      #endif
      return 0.0f;
    };

    return _scale.get_units(ntimes);
    /**
    switch (method) {
        case WeightMethod::Median:
            return std::max((readMedian() - getOffset()) / getFactor(), 0.);

        case WeightMethod::MedianAverage:
            return std::max((readMedAvg() - getOffset()) / getFactor(), 0.);

        case WeightMethod::Precise:
            return std::max((readPrecise(ntimes) - getOffset()) / getFactor(), 0.);

        case WeightMethod::Units:
            return std::max(_scale.get_units(ntimes), 0.f);

        default:
            return std::max((readMedAvg() - getOffset()) / getFactor(), 0.);
    }
    **/
}

long NeoBeeScale::getRaw(uint8_t ntimes) {
  if (!hasStarted()) {
      #ifdef DEBUG
      Serial.println("No scale present. Returning 0");
      #endif
      return 0;
  }; 
    
  return _scale.read_average(ntimes);   
}

long NeoBeeScale::readMedian() {
  if (!hasStarted()) {
    #ifdef DEBUG
    Serial.println("No scale present. Returning 0");
    #endif  
    return 0;
  };
  for (uint8_t i=0; i<15; i++) {
    _values[i] = _scale.read();
  };
  sortArray(_values,15);
  return _values[7];
}

long NeoBeeScale::readMedAvg() {
  if (!hasStarted()) {
    #ifdef DEBUG
      Serial.println("No scale present. Returning 0");
      #endif
      return 0;
  };
  for (uint8_t i=0; i<15; i++) {
    _values[i] = _scale.read();
  };
  sortArray(_values,15);
  return (_values[5] + _values[6] + _values[7] + _values[8] + _values[9]) / 5.;
}

double NeoBeeScale::readPrecise(uint8_t ntimes) {
  if (!hasStarted()) {
    #ifdef DEBUG
    Serial.println("No scale present. Returning 0.0");
    #endif
    return 0.0f;
  };
  long raw = 0;
  for (uint8_t i=0; i<ntimes; i++) {
      raw += readMedian();
  };
  return raw / ntimes;
}

bool NeoBeeScale::calibrate(uint16_t reference_weight, uint8_t ntimes) {
  if (!hasStarted()) {
    #ifdef DEBUG
    Serial.println("No scale present. Returning false");
    #endif
    return false;
  };
  
  if (ntimes == 0) {
    #ifdef DEBUG
    Serial.println("Number of measures is 0. Cannot calibrate.");
    #endif
    return false;
  };
  double raw = ntimes > 1 ? readPrecise(ntimes) : readMedian();
  setFactor((raw - m_ctx.scale.getOffset()) / reference_weight);
  #ifdef DEBUG
  Serial.println("___CALIBRATE___");
  Serial.print("Raw         : ");Serial.println(raw);
  Serial.print("Ref. Weight : ");Serial.println(reference_weight);
  Serial.print("Offset      : ");Serial.println(getOffset());
  Serial.print("Factor      : ");Serial.println(getFactor());
  #endif
  return true;
  
}

void NeoBeeScale::tare(uint8_t ntimes) {
  if (!hasStarted()) {
    #ifdef DEBUG
    Serial.println("No scale present. Taring not possible.");
    #endif
    return;
  };
  
  //double raw = ntimes > 1 ? readPrecise(ntimes) : readMedian();
  _scale.tare(5);
  #ifdef DEBUG
  Serial.println("___TARE___");
  Serial.print("Offset      : ");Serial.println(_scale.get_offset());
  #endif
  setOffset(_scale.get_offset());
}

double NeoBeeScale::getOffset() {
  return m_ctx.scale.getOffset();
}

void NeoBeeScale::setOffset(const long offset) {
  m_ctx.scale.setOffset(offset);
  _scale.set_offset(offset);
}

float NeoBeeScale::getFactor() {
  return m_ctx.scale.getFactor();
}

void NeoBeeScale::setFactor(const float factor) {
  m_ctx.scale.setFactor(factor);
  _scale.set_scale(factor);
}

uint8_t NeoBeeScale::getGain() {
  return m_ctx.scale.getGain();
}

void NeoBeeScale::setGain(const uint8_t gain) {
  m_ctx.scale.setGain(gain);
  _scale.set_gain(gain);
}
