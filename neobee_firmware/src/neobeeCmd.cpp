#include "neobeeCmd.h"

const char *ssid = "NeoBee";
const char *password = "sumsum";

WiFiServer wifiServer(8888);

#define HEADSIZE 2

NeoBeeCmd::NeoBeeCmd(Context& ctx, uint16_t port): 
m_ctx(ctx), m_port(port)
{
    m_buffer = (uint8_t*) malloc(BUFFER_SIZE);
    m_data_space = m_buffer + HEADSIZE;
    m_data_space_size = BUFFER_SIZE - HEADSIZE;
};

NeoBeeCmd::~NeoBeeCmd() {
    free(m_buffer);
};

void NeoBeeCmd::begin()
{   
    WiFi.softAP(ssid);

    IPAddress myIP = WiFi.softAPIP();
    #ifdef DEBUG
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    #endif 
    wifiServer = new WiFiServer(m_port);
    wifiServer->begin();
}

void NeoBeeCmd::sendResponse(WiFiClient& client, bool flush) {
    client.write(m_buffer, BUFFER_SIZE);
    if (flush) client.flush();
    clearBuffer();
}

/**
 * Return the name (and status 200) if set, or a status of 404,
 * if no name is set.
 **/
void NeoBeeCmd::handleGetNameCmd(WiFiClient& client) {
    clearBuffer();
    setCommand(CmdCode::GET_NAME);
    if (m_ctx.hasName()) {
        /**
         * Die min Betrachtung ist nur eine Sicherheitsmassnahme.
         * Eigentlich wird per Spezifikation sicher gestellt, dass
         * der Name immer kleiner, als der data space ist.
         **/
        memcpy(m_data_space, m_ctx.name, std::min((uint8_t)sizeof(m_ctx.name), m_data_space_size));
        setStatus(StatusCode::OK);
    } else {
        setStatus(StatusCode::NOT_FOUND);
    }
}

void NeoBeeCmd::handleSetNameCmd(WiFiClient& client) {
    /**
     * Wie verhaelt es sich mit 0 Terminierungen
     * und Laengen? Das array für den Namen des device
     * im Context muss nicht 0 terminiert sein.
     **/
    strncpy(m_ctx.name, (char*) (m_buffer+1), sizeof(m_ctx.name));
    clearBuffer();
    setStatus(StatusCode::OK);
    setCommand(CmdCode::SET_NAME);
}

/**
 * Sending back the offset of the scale and a status of OK,
 * if a offset is set. A status of NOT_FOUND, if not set.
 * 
 * The offset is send back as a four byte integer. The
 * actual value is multiplied by 100 before sending it back.
 * 
 * So the value of 55.23f would be sendback as 5523. But
 * also keep in mind, that an actual value of 55.2345f
 * would also be send back as 5523.
 */
void NeoBeeCmd::handleGetScaleOffsetCmd(WiFiClient& client) {
    clearBuffer();
    setCommand(CmdCode::GET_SCALE_OFFSET);
    setStatus(StatusCode::OK);
    #ifdef DEBUG
    m_ctx.scale.offset = 1234.55f;
    Serial.print("Scale is ");
    Serial.println(m_ctx.scale.offset);
    #endif
    uint32_t offset = int(m_ctx.scale.offset * 100);
    m_data_space[0] = (offset >> 24) & 0xff;
    m_data_space[1] = (offset >> 16) & 0xff;
    m_data_space[2] = (offset >> 8) & 0xff;
    m_data_space[3] = offset & 0xff;
}

void NeoBeeCmd::handleGetMACAddress(WiFiClient& client) {
    clearBuffer();
    setCommand(CmdCode::GET_MAC_ADDRESS);
    setStatus(StatusCode::OK);
    #ifdef DEBUG
    Serial.println(WiFi.macAddress());
    #endif
    WiFi.macAddress(m_data_space);
}

void NeoBeeCmd::handleCommand(WiFiClient& client) {
    CmdCode cmd = getCommand();
    switch (cmd) {
        case CmdCode::NOP:
            #ifdef DEBUG 
            Serial.println("NOP Request");
            #endif
            clearBuffer();
            setCommand(CmdCode::NOP);
            setStatus(StatusCode::OK); 
            break;
        case CmdCode::GET_NAME:
            #ifdef DEBUG 
            Serial.println("GET_NAME Request");
            #endif
            handleGetNameCmd(client);
            break;
        case CmdCode::SET_NAME:
            #ifdef DEBUG 
            Serial.println("SET_NAME_ADDRESS Request");
            #endif
            handleSetNameCmd(client);
            break;
        case CmdCode::GET_SCALE_OFFSET:
            #ifdef DEBUG 
            Serial.println("GET_SCALE_OFFSET Request");
            #endif
            handleGetScaleOffsetCmd(client);
            break;
        case CmdCode::ECHO:
            #ifdef DEBUG 
            Serial.println("ECHO Request");
            #endif
            handleEchoCmd(client);
            break;
        case CmdCode::GET_MAC_ADDRESS:
            #ifdef DEBUG 
            Serial.println("GET_MAC_ADDRESS Request");
            #endif
            handleGetMACAddress(client);
            break;
        default:
            #ifdef DEBUG 
            Serial.println("Unknown Command");
            #endif
            clearBuffer();
            setCommand(CmdCode::NOP);
            setStatus(StatusCode::BAD_REQUEST);
            break;
    }
    sendResponse(client);
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