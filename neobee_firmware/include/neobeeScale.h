#ifndef neobeeScale_h
#define neobeeScale_h

#include "neobeeTypes.h"
#include "neobeeContext.h"

enum class WeightMethod : uint8_t {
  MedianAverage = 0,
  Median        = 1,
  Units         = 2,
  Precise       = 3,
  None          = 255 // This is only to make shure, the static cast is within range.
};

class NeoBeeScale
{
  public:

    NeoBeeScale(Context& ctx);
    virtual ~NeoBeeScale();

    bool begin();
    bool calibrate(uint16_t reference_weight, uint8_t ntimes = 10);
    float getWeight(uint8_t ntimes = 20, WeightMethod method = WeightMethod::MedianAverage);

    double getOffset();
    void setOffset(const double offset);
    inline bool hasOffset() { return m_ctx.scale.hasOffset(); };

    float getFactor();
    void setFactor(const float factor);
    inline bool hasFactor() { return m_ctx.scale.hasFactor(); };

    uint8_t getGain();
    void setGain(const uint8_t gain);
    inline bool hasGain() { return m_ctx.scale.hasGain(); };

    void tare(uint8_t ntimes = 10);
    
  private:  
    Context& m_ctx;
    bool _has_started;
    long readMedian();
    double readPrecise(uint8_t ntimes = 10); 
    long getRaw(uint8_t ntimes = 20);
    long readMedAvg();
};

#endif
