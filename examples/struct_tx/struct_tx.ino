// отправка
#include <Arduino.h>
#include <UART_RF_TX.h>

struct Data {
    int i;
    float f;
};

void setup() {
    Serial.begin(4800);
}

UART_RF_TX<> tx(Serial, 4800);

void loop() {
    static int i;
    Data data{i++, 3.14};

    // Serial.begin(4800);
    tx.sendPacket(data);
    // Serial.end();

    delay(1000);
}