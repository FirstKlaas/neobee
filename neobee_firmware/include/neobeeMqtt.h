#ifndef neobeemqtt_h
#define neobeemqtt_h

#include <Arduino.h>
#include "neobeeContext.h"

enum class MqttFlags : uint8_t {
  FLAG_SSID_SET     = 0,
  FLAG_PASSWORD_SET = 1,
  FLAG_AUTH         = 2,
  FLAG_HOST_SET     = 3,
  FLAG_PORT_SET     = 4
};

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
     * context to the destination.
     **/  
    bool printHost(uint8_t* dest);
    
    void setPort(uint16_t port);

    uint16_t getPort(); 

    void publishData(float weight = 0.f, float tempInside = 0.f, float tempOutside = 0.f);

    void publishDeviceUp();

    bool connect(uint8_t number_of_tries = 10);


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