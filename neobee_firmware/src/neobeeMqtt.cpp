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
        m_buffer = new uint8_t(bufferSize());
    };
    return m_buffer;
};

uint8_t NeoBeeMqtt::operator[](const uint8_t index) {
    if (index >= buffer_size) {
        return 0;
    };
    uint8_t* buffer = getBuffer();
    return buffer[index];
};

size_t NeoBeeMqtt::bufferSize() const {
    return buffer_size;
};


