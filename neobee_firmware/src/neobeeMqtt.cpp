#include "neobeeMqtt.h"

#define buffer_size 32

NeoBeeMqtt::NeoBeeMqtt(Context& ctx): 
m_buffer(nullptr), m_ctx(ctx) 
{};

NeoBeeMqtt::~NeoBeeMqtt() {
    delete m_buffer;
};

void NeoBeeMqtt::setMqttHost(const uint8_t* host) {
    memcpy(m_ctx.mqttServer.host_name, host, sizeof(m_ctx.mqttServer.host_name));
};

void NeoBeeMqtt::sendMessage() {};

uint8_t* NeoBeeMqtt::getBuffer() {
    if (m_buffer == nullptr) {
        m_buffer = new uint8_t(buffer_size);
    };
    return m_buffer;
};

