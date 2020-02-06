#ifndef neobeeCmd_h
#define neobeeCmd_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "neobee.h"

#define BUFFER_SIZE 32


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
        bool m_started;

        void sendResponse(WiFiClient& client, bool flush = true);
        void handleCommand(WiFiClient& client);
        
        inline RequestMethod getMethod() {
            return static_cast<RequestMethod>(m_buffer[1] & REQUEST_METHOD_MASK);
        }

        inline void setMethod(RequestMethod method) {
            m_buffer[1] &= ~REQUEST_METHOD_MASK;
            m_buffer[1] |= static_cast<uint8_t>(method); 
        }

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