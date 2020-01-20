#ifndef neobeeTemperature_h
#define neobeeTemperature_h

#include "neobeeTypes.h"

class NeoBeeTemperature
{
  public:
    NeoBeeTemperature(Context& ctx);
    virtual ~NeoBeeTemperature();

    void begin();
    bool hasInsideSensorAddress();

  private:
    Context& m_ctx;
    
};

#endif
