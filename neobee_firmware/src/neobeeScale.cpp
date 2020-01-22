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
  #ifdef DEBUG
    Serial.println("Starting scale");
  #endif

  if (_has_started) return false;

  _scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);  
  delay(200);
  _scale.set_offset(m_ctx.scale.offset);
  _scale.set_scale(m_ctx.scale.factor);
  return true;
}

float NeoBeeScale::getWeight(uint8_t ntimes) {
  return _scale.get_units(ntimes);
}

long NeoBeeScale::getRaw(uint8_t ntimes) {
  return _scale.read_average(ntimes);   
}

long NeoBeeScale::readMedian() {
  for (uint8_t i=0; i<15; i++) {
    _values[i] = _scale.read();
  };
  sortArray(_values,15);
  return _values[7];
}

double NeoBeeScale::readPrecise(uint8_t ntimes) {
  long raw = 0;
  for (uint8_t i=0; i<ntimes; i++) {
      raw += readMedian();
  };
  return raw / ntimes;
}

void NeoBeeScale::calibrate(uint16_t reference_weight, uint8_t ntimes) {
  _scale.set_scale(1.f);
  double raw = readPrecise(ntimes);
  setFactor((raw - m_ctx.scale.offset) / reference_weight);
  _scale.set_scale(getFactor());
}

void NeoBeeScale::tare(uint8_t ntimes) {
  setOffset(readPrecise(ntimes));
  _scale.set_offset(getOffset());
}

double NeoBeeScale::getOffset() {
  return m_ctx.scale.offset;
}

void NeoBeeScale::setOffset(const double offset) {
  m_ctx.scale.offset = offset;
  bitSet(m_ctx.flags, FLAG_OFFSET_SET);
}

float NeoBeeScale::getFactor() {
  return m_ctx.scale.factor;
}

void NeoBeeScale::setFactor(const float factor) {
  m_ctx.scale.factor = factor;
  bitSet(m_ctx.flags, FLAG_FACTOR_SET);
}

uint8_t NeoBeeScale::getGain() {
  return m_ctx.scale.gain;
}

void NeoBeeScale::setGain(const uint8_t gain) {
  m_ctx.scale.gain = gain;
  bitSet(m_ctx.flags, FLAG_GAIN_SET);
}
