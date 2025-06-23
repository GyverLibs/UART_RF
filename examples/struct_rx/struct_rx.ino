// приём
#include <Arduino.h>
#include <UART_RF_RX.h>

struct Data {
    int i;
    float f;
};

UART_RF_RX<> rx(Serial, 4800);

void setup() {
    Serial.begin(4800);

    rx.onPacket([](uint8_t type, void* data, size_t len) {
        if (sizeof(Data) != len) return;  // проверка корректности длины
        Data& d = *((Data*)data);
        Serial.print(d.i);
        Serial.print(',');
        Serial.println(d.f);

        // или
        // Serial.println(static_cast<Data*>(data)->f);
    });
}

void loop() {
    rx.tick();
}