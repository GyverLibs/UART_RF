#pragma once
#include <Arduino.h>
#include <Hamming.h>

#include "./encoder/dc4_6.h"
#include "./encoder/manchester.h"
#include "./encoder/none.h"
#include "./utils.h"

static constexpr uint16_t URF_MAX_LEN = (1 << (16 - _URF_TYPE_SIZE)) - 1;
static constexpr uint16_t URF_MAX_TYPE = (1 << _URF_TYPE_SIZE) - 1;
#define URF_AUTO_TYPE URF_MAX_TYPE

#define _URF_RF_DESYNC 50    // минимальный период отправки по радио, мс
#define _URF_RF_MIN_TRAIN 4  // мин. кол-во импульсов для раскачки радио

template <class encoder_t = urf::Encoder::DC4_6, urf::Hamming hamming = urf::Hamming::None>
class UART_RF_TX {
   public:
    UART_RF_TX(Print& p, uint32_t baud, uint8_t trainMs = 20) : _enc(p), _trains(trainMs * baud / 10000) {}

    // реальный размер передаваемого пакета
    size_t packetSize(uint16_t size) {
        return _enc.packetSize(size * (hamming == urf::Hamming::None ? 1 : 2));
    }

    // ======== PACKET ========

    // отправить пакет авто (тип URF_AUTO_TYPE)
    template <typename Td>
    void sendPacket(const Td& data) {
        _sendPacket(URF_AUTO_TYPE, &data, sizeof(Td));
    }

    // отправить пакет авто (тип URF_AUTO_TYPE)
    void sendPacket(const void* data, size_t len) {
        _sendPacket(URF_AUTO_TYPE, data, len);
    }

    // отправить пакет с типом
    template <typename Tp, typename Td>
    void sendPacketT(Tp type, const Td& data) {
        _sendPacket(uint8_t(type), &data, sizeof(Td));
    }

    // отправить пакет с типом
    template <typename Tp>
    void sendPacketT(Tp type, const void* data, size_t len) {
        _sendPacket(uint8_t(type), data, len);
    }

    // ======== RAW ========

    // отправить сырые данные
    template <typename T>
    void sendRaw(const T& data) {
        sendRaw(&data, sizeof(T));
    }

    // отправить сырые данные
    void sendRaw(const void* data, size_t len) {
        _begin();

        switch (hamming) {
            case urf::Hamming::None: {
                const uint8_t* p = (const uint8_t*)data;
                while (len--) _enc.encode(*p++);
            } break;

            case urf::Hamming::Simple: {
                const uint8_t* p = (const uint8_t*)data;
                while (len--) _sendHam(*p++);
            } break;

            case urf::Hamming::Mixed: {
                size_t elen = Hamming3::encodedSize(len);
                uint8_t* buf = new uint8_t[elen];
                if (!buf) return;

                Hamming3::encode(buf, data, len);
                Hamming3::mix8(buf, elen);

                uint8_t* p = buf;
                while (elen--) _enc.encode(*p++);
                delete[] buf;
            } break;
        }

        _end();
    }

    // прошло времени с конца последней отправки, мс
    uint16_t lastSend() {
        return uint16_t(millis()) - _lastSend;
    }

   private:
    encoder_t _enc;
    uint16_t _lastSend = 0;
    uint8_t _trains;

    void _begin() {
        uint8_t t = lastSend() >= _URF_RF_DESYNC ? _trains : _URF_RF_MIN_TRAIN;
        while (t--) _enc.write(_URF_TRAIN_BYTE);
        _enc.write(_URF_START_BYTE);
        _enc.begin();
    }
    void _end() {
        _enc.end();
        _lastSend = millis();
    }

    void _sendHam(uint8_t b) {
        if (hamming == urf::Hamming::Simple) {
            _enc.encode(Hamming3::encode(b & 0xf));
            _enc.encode(Hamming3::encode(b >> 4));
        }
    }

    void _sendPacket(uint8_t type, const void* data, size_t len) {
        if (len > URF_MAX_LEN || type > URF_MAX_TYPE) return;

        bool odd = len & 1;
        uint8_t lentype[2];
        lentype[0] = (len << _URF_TYPE_SIZE) >> 8;
        lentype[1] = (len << _URF_TYPE_SIZE) | (type & _URF_TYPE_MASK);
        uint8_t crc = urf::crc8(data, len);
        if (!odd) crc = urf::crc8(&odd, 1, crc);
        crc = urf::crc8(lentype, 2, crc);

        _begin();

        switch (hamming) {
            case urf::Hamming::None: {
                const uint8_t* p = (const uint8_t*)data;
                while (len--) _enc.encode(*p++);
                if (!odd) _enc.encode(0);
                _enc.encode(lentype[0]);
                _enc.encode(lentype[1]);
                _enc.encode(crc);
            } break;

            case urf::Hamming::Simple: {
                const uint8_t* p = (const uint8_t*)data;
                while (len--) _sendHam(*p++);
                if (!odd) _sendHam(0);
                _sendHam(lentype[0]);
                _sendHam(lentype[1]);
                _sendHam(crc);
            } break;

            case urf::Hamming::Mixed: {
                size_t elen = Hamming3::encodedSize(len + 3 + !odd);
                uint8_t* buf = new uint8_t[elen];
                if (!buf) return;

                uint8_t* p = buf;
                Hamming3::encode(p, data, len), p += Hamming3::encodedSize(len);
                if (!odd) Hamming3::encode(p, &odd, 1), p += Hamming3::encodedSize(1);
                Hamming3::encode(p, lentype, 2), p += Hamming3::encodedSize(2);
                Hamming3::encode(p, &crc, 1);

                Hamming3::mix8(buf, elen);

                p = buf;
                while (elen--) _enc.encode(*p++);
                delete[] buf;
            } break;
        }

        _end();
    }
};