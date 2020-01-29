#ifndef neobeemqtt_h
#define neobeemqtt_h

#include <Arduino.h>
#include "neobeeTypes.h"
#include "neobeeContext.h"

class NeoBeeMqtt
{

  public:
    NeoBeeMqtt(Context& ctx);
    ~NeoBeeMqtt();

    void sendMessage();
    void setMqttHost(const uint8_t* host);

  private:
    uint8_t* m_buffer;
    Context& m_ctx;

    uint8_t* getBuffer();

};

#endif