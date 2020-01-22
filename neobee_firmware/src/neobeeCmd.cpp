#include "neobeeCmd.h"

const char *ssid = "NeoBee";
const char *password = "sumsum";

WiFiServer wifiServer(8888);

#define HEADSIZE 2

NeoBeeCmd::NeoBeeCmd(
    Context& ctx, 
    NeoBeeScale& scale, 
    NeoBeeTemperature& temp, 
    uint16_t port): 

    m_ctx(ctx), 
    m_port(port),
    m_scale(scale),
    m_temperature(temp)
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

inline void writeInt32(uint32_t value, uint8_t* dst) {
    dst[0] = (value >> 24) & 0xff;
    dst[1] = (value >> 16) & 0xff;
    dst[2] = (value >> 8) & 0xff;
    dst[3] = value & 0xff;
}

inline uint32_t readInt32(uint8_t* dst)
{
    return (dst[0] << 24) + (dst[1] << 16) + (dst[2] << 8) + (dst[3]); 
}

void NeoBeeCmd::handleCommand(WiFiClient& client) {
    CmdCode cmd = getCommand();
    switch (cmd) {
        case CmdCode::NOP:
            #ifdef DEBUG 
            Serial.println("NOP Request");
            #endif
            clearBuffer(CmdCode::NOP, StatusCode::OK); 
            break;

        case CmdCode::GET_NAME:
            #ifdef DEBUG 
            Serial.println("GET_NAME Request");
            #endif
            clearBuffer(CmdCode::GET_NAME, StatusCode::OK);
            if (m_ctx.hasName()) {
                memcpy(m_data_space, m_ctx.name, sizeof(m_ctx.name));
                setStatus(StatusCode::OK);
            } else {
                setStatus(StatusCode::NOT_FOUND);
            }
            break;

        case CmdCode::SET_NAME:
            #ifdef DEBUG 
            Serial.println("SET_NAME_ADDRESS Request");
            #endif
            /**
             * Wie verhaelt es sich mit 0 Terminierungen
             * und Laengen? Das array für den Namen des device
             * im Context muss nicht 0 terminiert sein.
             **/
            clearBuffer(CmdCode::SET_NAME, StatusCode::OK);
            strncpy(m_ctx.name, (char*) (m_buffer+1), sizeof(m_ctx.name));
            break;

        case CmdCode::GET_SCALE_OFFSET:
            #ifdef DEBUG 
            Serial.println("GET_SCALE_OFFSET Request");
            Serial.print("Scale offset is: ");
            Serial.println(m_ctx.scale.offset);
            #endif
            clearBuffer(CmdCode::GET_SCALE_OFFSET, StatusCode::OK);
            if (m_ctx.hasOffset()) {
                writeInt32(int(m_ctx.scale.offset * 100), m_data_space);
            } else {
                setStatus(StatusCode::NOT_FOUND);
            }
            break;

        case CmdCode::SET_SCALE_OFFSET:
            #ifdef DEBUG 
            Serial.println("SET_SCALE_OFFSET Request");
            #endif
            clearBuffer(CmdCode::SET_SCALE_OFFSET, StatusCode::OK);
            m_ctx.scale.offset = readInt32(m_data_space) / 100.f;
            #ifdef DEBUG 
            Serial.print("New offset is:");
            Serial.println(m_ctx.scale.offset);            
            #endif
            break;

        /* ------------------------------------
         * GET_MAC_ADDRESS
         */    
        case CmdCode::GET_MAC_ADDRESS:
            #ifdef DEBUG 
            Serial.println("GET_MAC_ADDRESS Request");
            #endif
            clearBuffer(CmdCode::GET_MAC_ADDRESS, StatusCode::OK);
            WiFi.macAddress(m_data_space);
            break;

        case CmdCode::GET_FLAGS:
            clearBuffer(CmdCode::GET_FLAGS, StatusCode::OK);
            m_data_space[0] = m_ctx.flags;
            break;

        case CmdCode::GET_SCALE_FACTOR:
            clearBuffer(CmdCode::GET_SCALE_FACTOR, StatusCode::OK);
            writeInt32(int(m_ctx.scale.factor * 100), m_data_space);
            break;

        case CmdCode::SET_SCALE_FACTOR:
            clearBuffer(CmdCode::SET_SCALE_FACTOR, StatusCode::OK);
            m_ctx.scale.factor = readInt32(m_data_space) / 100.f;
            break;

        case CmdCode::TARE:
            #ifdef DEBUG 
            Serial.println("TARE Request");
            #endif
            clearBuffer(CmdCode::TARE, StatusCode::OK);
            m_scale.begin();
            m_scale.tare(m_data_space[1]);
            writeInt32(int(m_ctx.scale.offset * 100), m_data_space);
            break;

        case CmdCode::CALIBRATE:
            clearBuffer(CmdCode::CALIBRATE, StatusCode::OK);
            m_scale.begin();
            uint16_t reference_weight = (m_data_space[0] << 0xff) | m_data_space[1];
            m_scale.calibrate(reference_weight, m_data_space[2]);
            writeInt32(int(m_ctx.scale.offset * 100), m_data_space);
            writeInt32(int(m_ctx.scale.factor * 100), m_data_space+4);
            break;

        default:
            #ifdef DEBUG 
            Serial.println("Unknown Command");
            #endif
            clearBuffer(CmdCode::NOP, StatusCode::BAD_REQUEST);
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