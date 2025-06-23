#pragma once
#include <Arduino.h>

namespace urf {
namespace Decoder {
class Manchester {
   public:
    static inline bool validLen(uint16_t len) {
        return len && !(len & 1);
    }
    static inline uint16_t getLen(uint16_t len) {
        return len >> 1;
    }
    static inline void decode(Stream& s, uint8_t* buf, uint16_t len) {
        while (len--) {
            *buf = (s.read() & 0xf);
            *buf |= (s.read() & 0xf) << 4;
            ++buf;
        }
    }
};
}  // namespace Decoder
}  // namespace urf