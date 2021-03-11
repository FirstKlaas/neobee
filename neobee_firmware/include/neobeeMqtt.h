#ifndef neobeemqtt_h
#define neobeemqtt_h

#include <Arduino.h>
#include "neobeeContext.h"

class NeoBeeMqtt
{

  public:
    NeoBeeMqtt(Context& ctx);
    ~NeoBeeMqtt();

    void setHost(const uint8_t* host);

    /**
     * Prints the mqtt host to dest.
     * It just copies the bytes from the configuration
     * context to the destination.
     **/  
    void printHost(uint8_t* dest);
    
    void setPort(uint16_t port);

    uint16_t getPort(); 

    void publishData(float weight = 0.f, float tempInside = 0.f, float tempOutside = 0.f);

    bool connect(uint8_t number_of_tries = 20);

    bool isConnected();

    void loop();


  private:
    uint8_t* m_buffer;
    Context& m_ctx;
    uint8_t operator[](const uint8_t index);

    /**
     * Returns the buffer size for the message
     * to be broadcasted.
     **/
    size_t bufferSize() const;

    uint8_t* getBuffer();
    void callback(char* topic, byte* payload, unsigned int length);

};

#endif