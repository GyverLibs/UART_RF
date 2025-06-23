#pragma once
#include <Arduino.h>

namespace urf {
namespace Decoder {
class DC4_6 {
   public:
    static inline bool validLen(uint16_t len) {
        return len && !(len % 3);
    }
    static inline uint16_t getLen(uint16_t len) {
        return len * 2 / 3;
    }
    static inline void decode(Stream& s, uint8_t* buf, uint16_t len) {
        while (len) {
            uint8_t b0 = s.read();
            uint8_t b1 = s.read();
            uint8_t b2 = s.read();
            *buf++ = _decodeNibble(b0 >> 2) | (_decodeNibble((b0 << 4) | (b1 >> 4)) << 4);
            *buf++ = _decodeNibble((b1 << 2) | (b2 >> 6)) | (_decodeNibble(b2) << 4);
            len -= 2;
        }
    }

   private:
    static uint8_t _decodeNibble(uint8_t n6) {
        switch (n6 & 0b111111) {
            case 0b110100: return 0;
            case 0b110010: return 1;
            case 0b110001: return 2;
            case 0b101100: return 3;
            case 0b101010: return 4;
            case 0b101001: return 5;
            case 0b100110: return 6;
            case 0b100101: return 7;
            case 0b100011: return 8;
            case 0b011010: return 9;
            case 0b011001: return 10;
            case 0b010110: return 11;
            case 0b010101: return 12;
            case 0b010011: return 13;
            case 0b001101: return 14;
            case 0b001011: return 15;
        }
        return 0xff;
    }
};
}  // namespace Decoder
}  // namespace urf