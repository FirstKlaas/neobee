#include "neobeeMqtt.h"
#include "neobeeUtil.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

#define buffer_size 32 // The buffer size for messages to send
#define NbMqtt m_ctx.mqttServer

NeoBeeMqtt::NeoBeeMqtt(Context& ctx): 
m_buffer(nullptr), m_ctx(ctx) 
{
    memset(m_ctx.mqttServer.host_name,0,30);
    memset(m_ctx.mqttServer.login,0,30);
    memset(m_ctx.mqttServer.password,0,30);
    m_ctx.mqttServer.port = 0;
    client.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); }); 
};

NeoBeeMqtt::~NeoBeeMqtt() {
    delete m_buffer;
};

void NeoBeeMqtt::setHost(const uint8_t* host) {
    memcpy(m_ctx.mqttServer.host_name, host, 30);
};

void NeoBeeMqtt::printHost(uint8_t* dest) {
    memcpy(dest, m_ctx.mqttServer.host_name, 30);
};

void NeoBeeMqtt::setPort(uint16_t port) {
    m_ctx.mqttServer.port = port;
};

uint16_t NeoBeeMqtt::getPort() {
    return m_ctx.mqttServer.port;
}; 

void NeoBeeMqtt::callback(char* topic, byte* payload, unsigned int length) {

};

bool NeoBeeMqtt::connect(uint8_t number_of_tries) {
    if (isConnected()) return true;

    // Checking the prerequisites to connect to mqtt.
    if (!m_ctx.mqttServer.hostnameSet()) {
        #ifdef DEBUG
        Serial.println("No hostname set.");
        #endif
        return false;
    };

    if (!m_ctx.mqttServer.portSet()) {
        #ifdef DEBUG
        Serial.println("No port set.");
        #endif
        return false;
    };

    uint8_t i = 0;

    String clientId = stringFromByteAray(m_ctx.name, sizeof(m_ctx.name));
    clientId += "-";
    clientId += WiFi.macAddress();

    #ifdef DEBUG
    Serial.println("Connecting to mqtt server");
    Serial.print("client: ");
    Serial.println(clientId);
    Serial.print("Server: ");
    Serial.print(NbMqtt.getHostName());
    Serial.print(":");
    Serial.println(NbMqtt.port);
    #endif

    String tmpHostName = NbMqtt.getHostName();
    String tmpLogin = NbMqtt.getLogin();
    String tmpPassword = NbMqtt.getPassword();

    client.setServer(tmpHostName.c_str(), NbMqtt.port);
    //client.setServer("192.168.178.77", 1883);
    #ifdef DEBUG
    if (!NbMqtt.loginSet()) Serial.println("No Login");
    if (!NbMqtt.passwordSet()) Serial.println("No Password");
    
    if (NbMqtt.credentialsSet()) {
        Serial.println("Connecting to mqtt server with credentials.");
    } else {
        Serial.println("Connecting to mqtt server without credentials.");
    };
    #endif

    while (!client.connected() && i < number_of_tries) {
        i++;
        #ifdef DEBUG
        Serial.print(".");
        #endif
        // If credentials are set, use those.
        if (NbMqtt.credentialsSet()) {
            client.connect(
                clientId.c_str(), 
                tmpLogin.c_str(), 
                tmpPassword.c_str());
        } else {
            client.connect(clientId.c_str());
        };
    };
    #ifdef DEBUG
    Serial.println();
    #endif

    if (client.connected()) {
        uint8_t* bufferPtr(getBuffer());
        uint32_t ip = uint32_t(WiFi.localIP());

        WiFi.macAddress(bufferPtr);
        bufferPtr += 6;
        memcpy(bufferPtr, &ip, 4);
        bufferPtr += 4;
        writeDouble100(m_ctx.scale.offset, bufferPtr);
        bufferPtr += 4;
        writeFloat100(m_ctx.scale.factor, bufferPtr);
        bufferPtr += 4;

        client.publish("/neobee/hive/connect", getBuffer(), bufferPtr - getBuffer());
    } else {
        #ifdef DEBUG
        Serial.println("Could not connect to mqtt server");
        Serial.println(NbMqtt.getHostName());
        Serial.println(NbMqtt.port);
        
        #endif
    };
    
    return client.connected();
}

bool NeoBeeMqtt::isConnected() 
{
    return client.connected();
};

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

void NeoBeeMqtt::publishData(float weight, float tempInside, float tempOutside) {
    uint8_t* bufferPtr(getBuffer());

    WiFi.macAddress(bufferPtr);
    bufferPtr += 6;
    writeInt32(int(weight * 100.f + 0.5f), bufferPtr);
    bufferPtr += 4;
    writeInt32(int(tempInside * 100.f + 0.5f), bufferPtr);
    bufferPtr += 4;
    writeInt32(int(tempOutside * 100.f + 0.5f), bufferPtr);
    bufferPtr += 4;
    client.publish("/neobee/hive/rawdata", getBuffer(), 18);
    memset(getBuffer(),0,18);
}
