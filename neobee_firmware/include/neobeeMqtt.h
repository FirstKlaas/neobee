#ifndef neobeemqtt_h
#define neobeemqtt_h

#include <Arduino.h>
#include "neobeeTypes.h"
#include "neobeeContext.h"

class NeoBeeMqtt
{

  public:
    NeoBeeMqtt(Context& ctx);
    ~NeoBeeMqtt();

    void sendMessage();
    void setHost(const uint8_t* host);

    /**
     * Prints the mqtt host to dest.
     * It just copies the bytes from the configuration
     * context to the destination. Size is the size of
     * the dest buffer. The number of bytes copied is
     * the minimum of the size parameter and the length
     * of the mqtt host in the configuration context.
     * Returns true, if the full mqtt hostname could be 
     * copied, false else.
     **/  
    bool printHost(uint8_t dest, uint8_t size);
    
    void setPort(uint16_t port);

    uint16_t getPort(); 

    uint8_t operator[](const uint8_t index);

    /**
     * Returns the buffer size for the message
     * to be broadcasted.
     **/
    size_t bufferSize() const;

  private:
    uint8_t* m_buffer;
    Context& m_ctx;

    uint8_t* getBuffer();

};

#endif