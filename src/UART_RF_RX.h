#pragma once
#include <Arduino.h>
#include <Hamming.h>

#include "./decoder/dc4_6.h"
#include "./decoder/manchester.h"
#include "./decoder/none.h"
#include "./utils.h"

template <class decoder_t = urf::Decoder::DC4_6, urf::Hamming hamming = urf::Hamming::None>
class UART_RF_RX {
    typedef void (*PacketCallback)(uint8_t type, void* data, size_t len);
    typedef void (*RawCallback)(void* data, size_t len);

    enum class State : uint8_t {
        Idle,
        Train,
        Reading,
    };

   public:
    UART_RF_RX(Stream& s, uint32_t baud) : _s(s), _tout((10000 * 2 + baud) / baud + 1) {}

    // подключить обработчик пакета вида f(uint8_t type, void* data, size_t len)
    void onPacket(PacketCallback cb) {
        _pack_cb = cb;
    }

    // подключить обработчик сырых данных вида f(void* data, size_t len)
    void onRaw(RawCallback cb) {
        _raw_cb = cb;
    }

    // получить качество приёма в процентах (только для пакетов Packet)
    uint8_t getRSSI() {
        uint8_t x = _rssi;
        x -= (x >> 1) & 0x55;
        x = (x & 0x33) + ((x >> 2) & 0x33);
        x = (x + (x >> 4)) & 0x0F;
        return (x * 100) >> 3;
    }

    // тикер, вызывать в loop
    void tick() {
        do {
            switch (_state) {
                case State::Idle:
                    if (_s.read() == _URF_TRAIN_BYTE) {
                        _state = State::Train;
                        _tmr = millis();
                    }
                    break;

                case State::Train:
                    switch (_s.read()) {
                        case _URF_TRAIN_BYTE:
                            break;

                        case _URF_START_BYTE:
                            _state = State::Reading;
                            _rssi <<= 1;
                            break;

                        default:
                            if (_timeout()) _state = State::Idle;
                            return;
                    }
                    _tmr = millis();
                    break;

                case State::Reading: {
                    uint8_t av = _s.available();
                    if (_pAv != av) {
                        _pAv = av;
                        _tmr = millis();
                    } else if (_timeout()) {
                        _state = State::Idle;
                        if (!decoder_t::validLen(av)) break;
                        uint16_t len = decoder_t::getLen(av);
                        uint8_t buf[len];
                        decoder_t::decode(_s, buf, len);

                        switch (hamming) {
                            case urf::Hamming::None:
                                break;

                            case urf::Hamming::Simple:
                                if (!Hamming3::decode(buf, len)) return;
                                len = Hamming3::decodedSize(len);
                                break;

                            case urf::Hamming::Mixed:
                                if (!Hamming3::unmix8(buf, len) || !Hamming3::decode(buf, len)) return;
                                len = Hamming3::decodedSize(len);
                                break;
                        }

                        if (_raw_cb) _raw_cb(buf, len);
                        if (_pack_cb) {
                            if (len > 3 && !urf::crc8(buf, len)) {
                                uint16_t plen = ((buf[len - 3] << 8) | buf[len - 2]) >> _URF_TYPE_SIZE;
                                if (plen == len - 3 || plen == len - 4) {
                                    _rssi |= 1;
                                    _pack_cb(buf[len - 2] & _URF_TYPE_MASK, buf, plen);
                                }
                            }
                        }
                    }
                } break;
            }
        } while (_s.available());
    }

   private:
    Stream& _s;
    PacketCallback _pack_cb = nullptr;
    RawCallback _raw_cb = nullptr;

    uint16_t _tmr = 0;
    uint8_t _rssi = 0xff;
    uint8_t _tout;
    uint8_t _pAv = 0;
    State _state = State::Idle;

    inline bool _timeout() {
        return uint16_t(uint16_t(millis()) - _tmr) >= _tout;
    }
};