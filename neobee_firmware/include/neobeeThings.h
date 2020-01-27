#ifndef neobeeButtons_h
#define neobeeButtons_h

#include <Arduino.h>

class NeoBeeButton 
{
    public:
        NeoBeeButton(const uint8_t pin);
        ~NeoBeeButton();

        bool isPressed();

    private:
        const uint8_t m_pin;
};


class NeoBeeLED
{
    public:
        NeoBeeLED(const uint8_t pin);
        ~NeoBeeLED();

        void pulse(uint16_t duration_ms = 500, uint8_t count = 1, uint16_t dark_time = 0);

    private:
        const uint8_t m_pin;
};

#endif
