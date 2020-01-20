#ifndef neobeeCmd_h
#define neobeeCmd_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "neobee.h"

enum class CmdCode  : uint8_t {
    NOP = 0,
    GET_NAME = 1,
    SET_NAME = 2,
    GET_SCALE_OFFSET = 10,
    SET_SCALE_OFFSET = 11
};

class NeoBeeCmd
{
    public:
        NeoBeeCmd(Context& ctx, uint8_t buffer_size, uint16_t port=8888);
        virtual ~NeoBeeCmd();

        inline CmdCode getCommand() {
            return static_cast<CmdCode>(m_buffer[0]);
        }

        inline void setCommand(CmdCode cmd) {
            m_buffer[0] = static_cast<uint8_t>(cmd);
        }

    private:
        Context& m_ctx;
        uint8_t m_buffer_size;
        uint16_t m_port;
        uint8_t* m_buffer;

        void setNameFromCmd();
};

#endif