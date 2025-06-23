#pragma once
#include <Arduino.h>

namespace urf {
namespace Encoder {
class DC4_6 {
   public:
    DC4_6(Print& p) : _p(p) {}

    size_t packetSize(uint16_t size) {
        return size * 3 / 2;
    }

    inline void begin() {
        _first = true;
    }

    void encode(uint8_t b) {
        uint8_t l = _encodeNibble(b & 0xf);
        uint8_t h = _encodeNibble(b >> 4);

        if (_first) {
            _p.write((l << 2) | (h >> 4));
            _buf = h;
        } else {
            _p.write((_buf << 4) | (l >> 2));
            _p.write((l << 6) | h);
        }
        _first ^= 1;
    }

    void end() {
        if (!_first) encode(0);
    }

    inline void write(uint8_t b) {
        _p.write(b);
    }

   private:
    Print& _p;
    bool _first = true;
    uint8_t _buf;

    uint8_t _encodeNibble(uint8_t n) {
        switch (n) {
            case 0: return 0b110100;
            case 1: return 0b110010;
            case 2: return 0b110001;
            case 3: return 0b101100;
            case 4: return 0b101010;
            case 5: return 0b101001;
            case 6: return 0b100110;
            case 7: return 0b100101;
            case 8: return 0b100011;
            case 9: return 0b011010;
            case 10: return 0b011001;
            case 11: return 0b010110;
            case 12: return 0b010101;
            case 13: return 0b010011;
            case 14: return 0b001101;
            case 15: return 0b001011;
        }
        return 0;
    }
};
}  // namespace Encoder
}  // namespace urf