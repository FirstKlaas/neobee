#ifndef neobeeTemperature_h
#define neobeeTemperature_h

#include "neobeeTypes.h"
#include "neobeeContext.h"

class NeoBeeTemperature
{
  public:
    NeoBeeTemperature(Context& ctx);
    virtual ~NeoBeeTemperature();

    void begin();
    float getCTemperatureByIndex(const uint8_t index=0);
    float getCTemperatureInside();
    float getCTemperatureOutside();
    uint8_t getDeviceCount();
    bool hasInsideSensorAddress();
    

  private:
    Context& m_ctx;
    bool m_has_started;
    uint8_t sensor_count;
    
};

#endif
