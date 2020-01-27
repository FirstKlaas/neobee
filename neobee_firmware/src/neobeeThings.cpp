#include "neobeeThings.h"

NeoBeeButton::NeoBeeButton(uint8_t pin): m_pin(pin) {
    pinMode(m_pin, INPUT);
};

NeoBeeButton::~NeoBeeButton() {};

bool NeoBeeButton::isPressed() {
    return (digitalRead(m_pin) == HIGH );
};


NeoBeeLED::NeoBeeLED(const uint8_t pin): m_pin(pin) {
    pinMode(m_pin, OUTPUT);
};

NeoBeeLED::~NeoBeeLED() {

};

void NeoBeeLED::pulse(uint16_t duration_ms, uint8_t count, uint16_t dark_time) {
    for (uint8_t i=0; i++; i<count) {
        digitalWrite(m_pin, HIGH);
        delay(duration_ms);
        digitalWrite(m_pin, LOW);
        if (dark_time > 0) {
            delay(dark_time);
        }
        
    }
};
