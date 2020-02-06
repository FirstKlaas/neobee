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

float NeoBeeScale::getWeight(uint8_t ntimes) {
  begin();
  //return std::max(_scale.get_units(ntimes), 0.f);
  return std::max((readMedAvg() - getOffset()) / getFactor(), 0.);
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
  if (ntimes == 0) return false;
  _scale.set_scale(1.f);
  double raw = ntimes > 1 ? readPrecise(ntimes) : _scale.read();
  setFactor((raw - m_ctx.scale.offset) / reference_weight);
  _scale.set_scale(getFactor());
  return true;
  
}

void NeoBeeScale::tare(uint8_t ntimes) {
  setOffset(readPrecise(ntimes));
  _scale.set_offset(getOffset());
}

double NeoBeeScale::getOffset() {
  return m_ctx.scale.offset;
}

void NeoBeeScale::setOffset(const double offset) {
  m_ctx.scale.setOffset(offset);
}

float NeoBeeScale::getFactor() {
  return m_ctx.scale.getFactor();
}

void NeoBeeScale::setFactor(const float factor) {
  m_ctx.scale.setFactor(factor);
}

uint8_t NeoBeeScale::getGain() {
  return m_ctx.scale.getGain();
}

void NeoBeeScale::setGain(const uint8_t gain) {
  m_ctx.scale.setGain(gain);
}
