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
    Serial.println("LED initialized");
};

NeoBeeLED::~NeoBeeLED() {
    switchOff();
};

void NeoBeeLED::pulse(uint16_t duration_ms, uint8_t count, uint16_t dark_time) {
    for (uint8_t i=0; i<count; i++) {
        switchOn();
        delay(duration_ms);
        switchOff();
        if (dark_time > 0) {
            delay(dark_time);
        }
    }
};

void NeoBeeLED::switchOn() {
    digitalWrite(m_pin, HIGH);
};

void NeoBeeLED::switchOff() {
    digitalWrite(m_pin, LOW);
};

void NeoBeeLED::toggle() {
    digitalWrite(m_pin, !digitalRead(m_pin));
};