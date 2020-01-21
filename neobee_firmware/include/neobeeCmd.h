#ifndef neobeeCmd_h
#define neobeeCmd_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "neobee.h"

#define BUFFER_SIZE 32

enum class CmdCode : uint8_t {
    NOP = 0,
    GET_NAME = 1,
    SET_NAME = 2,
    GET_SCALE_OFFSET = 10,
    SET_SCALE_OFFSET = 11,
    GET_MAC_ADDRESS = 80,
    ECHO = 255
};

enum class StatusCode : uint8_t {
    OK = 20,
    BAD_REQUEST = 40,
    NOT_FOUND = 44
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
        uint8_t* m_data_space;
        uint8_t m_data_space_size;

        void sendResponse(WiFiClient& client, bool flush = true);
        void handleCommand(WiFiClient& client);

        /**
         * Handles the GET_NAME command.
         * 
         * Sends back the name of the device.
         * 
         * If no name is set, a 404 status is returned
         * and no assertions for the data are made.
         * 
         * If a name is set, a status of 200 is returned.
         * The data block contains the name of the device.
         * All unused data block bytes have a value of 0.
         **/ 
        void handleGetNameCmd(WiFiClient& client);

        void handleSetNameCmd(WiFiClient& client);

        void handleGetScaleOffsetCmd(WiFiClient& client);

        void handleGetMACAddress(WiFiClient& client);

        /** 
         * Handles the ECHO command.
         * 
         * The ECHO command just sends back the incoming data
         * without any change. This command is more for testing
         * purpose.
         **/ 
        inline void handleEchoCmd(WiFiClient& client)
        {
            #ifdef DEBUG
            Serial.println("CMD ECHO");
            #endif 
            sendResponse(client); 
        };
        
        inline void setStatus(StatusCode status) {
            m_buffer[1] = static_cast<uint8_t>(status);
        }

        inline void clearBuffer() { memset(m_buffer, 0, BUFFER_SIZE); };
};

#endif