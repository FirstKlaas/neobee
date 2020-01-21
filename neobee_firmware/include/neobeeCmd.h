#ifndef neobeeCmd_h
#define neobeeCmd_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "neobee.h"

#define BUFFER_SIZE 32

enum class CmdCode  : uint8_t {
    NOP = 0,
    GET_NAME = 1,
    SET_NAME = 2,
    GET_SCALE_OFFSET = 10,
    SET_SCALE_OFFSET = 11,
    ECHO = 255
};

class NeoBeeCmd
{
    public:
        NeoBeeCmd(Context& ctx, uint16_t port=8888);
        virtual ~NeoBeeCmd();

        inline CmdCode getCommand() {
            return static_cast<CmdCode>(m_buffer[0]);
        }

        inline void setCommand(CmdCode cmd) {
            m_buffer[0] = static_cast<uint8_t>(cmd);
        }

        void begin();
        void checkForCommands();

    private:
        Context& m_ctx;
        uint16_t m_port;
        uint8_t* m_buffer;
        WiFiServer* wifiServer;

        void sendResponse(WiFiClient& client, bool flush = true);
        void handleCommand(WiFiClient& client);
        void handleGetNameCmd(WiFiClient& client);
        inline void handleEchoCmd(WiFiClient& client)
        {
            Serial.println("CMD ECHO"); 
            sendResponse(client); 
        };
        
        inline void clearBuffer() { memset(m_buffer, 0, BUFFER_SIZE); };
        inline uint8_t* dataSpace() { return m_buffer+2; };
        inline uint8_t dataSpaceSize() { return BUFFER_SIZE-2; }; 
};

#endif