#include "neobeeCmd.h"

const char *ssid = "NeoBee";
const char *password = "sumsum";

WiFiServer wifiServer(8888);

NeoBeeCmd::NeoBeeCmd(Context& ctx, uint16_t port): 
m_ctx(ctx), m_port(port)
{
    m_buffer = (uint8_t*) malloc(BUFFER_SIZE);
};

NeoBeeCmd::~NeoBeeCmd() {
    free(m_buffer);
};

void NeoBeeCmd::begin()
{   
    WiFi.softAP(ssid);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP); 
    wifiServer = new WiFiServer(m_port);
    wifiServer->begin();
}

void NeoBeeCmd::sendResponse(WiFiClient& client, bool flush) {
    client.write(m_buffer, BUFFER_SIZE);
    if (flush) client.flush();
}

void NeoBeeCmd::handleGetNameCmd(WiFiClient& client) {
    clearBuffer();
    setCommand(CmdCode::GET_NAME);
    memcpy(dataSpace(), m_ctx.name, sizeof(m_ctx.name));
    sendResponse(client);
    clearBuffer();
}

void NeoBeeCmd::handleCommand(WiFiClient& client) {
    CmdCode cmd = getCommand();
    switch (cmd) {
        case CmdCode::NOP: Serial.println("NOP"); break;
        case CmdCode::GET_NAME: handleGetNameCmd(client); break;
        case CmdCode::ECHO: handleEchoCmd(client); break;
        default: Serial.println("Unknown Command"); break;
    }
}

void NeoBeeCmd::checkForCommands() {
    WiFiClient client = wifiServer->available();

    if (client) {
        while (client.connected()) {
            uint8_t index = 0;
            clearBuffer();
            while (client.available()>0) {
                m_buffer[index++] = client.read();
                if (index == 32) {
                    handleCommand(client);
                    clearBuffer();
                    index = 0;
                }
            }
            delay(10);
        };
        client.stop();
    }
}