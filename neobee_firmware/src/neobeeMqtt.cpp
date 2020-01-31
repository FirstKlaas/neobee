#include "neobeeMqtt.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

#define buffer_size 32 // The buffer size for messages to send


NeoBeeMqtt::NeoBeeMqtt(Context& ctx): 
m_buffer(nullptr), m_ctx(ctx) 
{
    memset(m_ctx.mqttServer.host_name,0,30);
    memset(m_ctx.mqttServer.login,0,30);
    memset(m_ctx.mqttServer.password,0,30);
    m_ctx.mqttServer.port = 0;
    m_ctx.mqttServer.flags = 0;
    client.setCallback(this->callback);
};

NeoBeeMqtt::~NeoBeeMqtt() {
    delete m_buffer;
};

void NeoBeeMqtt::setHost(const uint8_t* host) {
    memcpy(m_ctx.mqttServer.host_name, host, 30);
};

bool NeoBeeMqtt::printHost(uint8_t* dest) {
    memcpy(dest, m_ctx.mqttServer.host_name, 30);
};

void NeoBeeMqtt::setPort(uint16_t port) {
    m_ctx.mqttServer.port = port;
};

uint16_t NeoBeeMqtt::getPort() {
    return m_ctx.mqttServer.port;
}; 

bool NeoBeeMqtt::connect(uint8_t number_of_tries) {
    if (client.connected) return;

    uint8_t i = 0;
    String clientId = "NeoBeeHive-";
    clientId += WiFi.macAddress();

    #ifdef DEBUG
    Serial.println("Connecting to mqtt server");
    Serial.print("client: ");
    Serial.println(clientId);
    #endif

    while (!client.connected() && i < number_of_tries) {
        i++;
        Serial.print(".");
    };

    #ifdef DEBUG
    if (client.connected()) {
        Serial.println("Could not connect to mqtt server");
    } else {
        Serial.println("Connected successfully");
    };
    #endif

    return client.connected();
}

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


