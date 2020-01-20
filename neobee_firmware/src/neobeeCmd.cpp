#include "neobeeCmd.h"


NeoBeeCmd::NeoBeeCmd(Context& ctx, uint8_t buffer_size, uint16_t port): 
m_ctx(ctx), m_buffer_size(buffer_size), m_port(port)
{
    m_buffer = (uint8_t*) malloc(m_buffer_size);
    if (getCommand() == CmdCode::NOP) {
        setCommand(CmdCode::GET_NAME);
    }
};

NeoBeeCmd::~NeoBeeCmd() {
    free(m_buffer);
};

void NeoBeeCmd::setNameFromCmd() {
    memcpy(m_ctx.name, m_buffer+1, 20);
}
