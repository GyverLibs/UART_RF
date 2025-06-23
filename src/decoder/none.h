#pragma once
#include <Arduino.h>

namespace urf {
namespace Decoder {
class None {
   public:
    static inline bool validLen(uint16_t len) {
        return len;
    }
    static inline uint16_t getLen(uint16_t len) {
        return len;
    }
    static inline void decode(Stream& s, uint8_t* buf, uint16_t len) {
        s.readBytes(buf, len);
    }
};
}  // namespace Decoder
}  // namespace urf