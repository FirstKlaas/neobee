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

bool NeoBeeScale::begin() {

  if (_has_started) return false;
  #ifdef DEBUG
    Serial.println("Starting scale");
  #endif

  _has_started = true;

  _scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);  
  delay(200);
  _scale.set_offset(m_ctx.scale.offset);
  _scale.set_scale(m_ctx.scale.factor);
  return true;
}

float NeoBeeScale::getWeight(uint8_t ntimes, WeightMethod method) {
    begin();
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
}

long NeoBeeScale::getRaw(uint8_t ntimes) {
  begin();
  return _scale.read_average(ntimes);   
}

long NeoBeeScale::readMedian() {
  begin();
  for (uint8_t i=0; i<15; i++) {
    _values[i] = _scale.read();
  };
  sortArray(_values,15);
  return _values[7];
}

long NeoBeeScale::readMedAvg() {
  begin();
  for (uint8_t i=0; i<15; i++) {
    _values[i] = _scale.read();
  };
  sortArray(_values,15);
  return (_values[5] + _values[6] + _values[7] + _values[8] + _values[9]) / 5.;
}

double NeoBeeScale::readPrecise(uint8_t ntimes) {
  begin();
  long raw = 0;
  for (uint8_t i=0; i<ntimes; i++) {
      raw += readMedian();
  };
  return raw / ntimes;
}

bool NeoBeeScale::calibrate(uint16_t reference_weight, uint8_t ntimes) {
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
  double raw = ntimes > 1 ? readPrecise(ntimes) : readMedian();
  #ifdef DEBUG
  Serial.println("___TARE___");
  Serial.print("Raw         : ");Serial.println(raw);
  Serial.print("Offset      : ");Serial.println(getOffset());
  Serial.print("Factor      : ");Serial.println(getFactor());
  #endif
  setOffset(raw);
}

double NeoBeeScale::getOffset() {
  return m_ctx.scale.getOffset();
}

void NeoBeeScale::setOffset(const double offset) {
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
