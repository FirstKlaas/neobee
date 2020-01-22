#ifndef neobeeCmd_h
#define neobeeCmd_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "neobee.h"

#define BUFFER_SIZE 32

enum class CmdCode : uint8_t {
    NOP              =   0,
    GET_NAME         =   1,
    SET_NAME         =   2,
    GET_FLAGS        =   3,
    GET_SCALE_OFFSET =  10,
    SET_SCALE_OFFSET =  11,
    GET_SCALE_FACTOR =  12,
    SET_SCALE_FACTOR =  13,

    GET_MAC_ADDRESS  =  80,
    GET_VERSION      =  81,
    SET_IDLE_TIME    =  82,
    GET_IDLE_TIME    =  83, 
    TARE             = 200,
    CALIBRATE        = 201,
    GET_WEIGHT       = 202
};

enum class StatusCode : uint8_t {
    NONE         =  0,
    OK           = 20,
    BAD_REQUEST  = 40,
    NOT_FOUND    = 44
};

class NeoBeeCmd
{
    public:
        NeoBeeCmd(Context& ctx, NeoBeeScale& scale, NeoBeeTemperature& temp, uint16_t port=8888);
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
        NeoBeeScale& m_scale;
        NeoBeeTemperature& m_temperature;

        void sendResponse(WiFiClient& client, bool flush = true);
        void handleCommand(WiFiClient& client);
        
        inline void setStatus(StatusCode status) {
            m_buffer[1] = static_cast<uint8_t>(status);
        };

        inline void clearBuffer(
            CmdCode cmd = CmdCode::NOP,
            StatusCode status = StatusCode::NONE 
        ) {
            memset(m_buffer, 0, BUFFER_SIZE);
            setCommand(cmd);
            setStatus(status); 
        };
};

#endif