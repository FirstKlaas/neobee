#ifndef neobeeScale_h
#define neobeeScale_h

#include "neobeeTypes.h"
#include "neobeeContext.h"

class NeoBeeScale
{
  public:

    NeoBeeScale(Context& ctx);
    virtual ~NeoBeeScale();

    bool begin();
    bool calibrate(uint16_t reference_weight, uint8_t ntimes = 10);
    float getWeight(uint8_t ntimes = 20);

    double getOffset();
    void setOffset(const double offset);
    inline bool hasOffset() { return bitRead(m_ctx.flags, FLAG_OFFSET_SET); };

    float getFactor();
    void setFactor(const float factor);
    inline bool hasFactor() { return bitRead(m_ctx.flags, FLAG_FACTOR_SET); };

    uint8_t getGain();
    void setGain(const uint8_t gain);
    inline bool hasGain() { return bitRead(m_ctx.flags, FLAG_GAIN_SET); };

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
