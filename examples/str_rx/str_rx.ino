// приём
#include <Arduino.h>
#include <UART_RF_RX.h>

UART_RF_RX<> rx(Serial, 4800);

void setup() {
    Serial.begin(4800);

    rx.onPacket([](uint8_t type, void* data, size_t len) {
        Serial.write((uint8_t*)data, len);
        Serial.println();
    });
}

void loop() {
    rx.tick();
}