// отправка
#include <Arduino.h>
#include <UART_RF_TX.h>

void setup() {
    Serial.begin(4800);
}

UART_RF_TX<> tx(Serial, 4800);

void loop() {
    String s;
    s += "hello! ";
    static uint8_t i;
    s += ++i;

    // Serial.begin(4800);
    tx.sendPacket(s.c_str(), s.length());
    // Serial.end();

    delay(1000);
}