#pragma once
#include <Arduino.h>

namespace urf {
namespace Encoder {
class Manchester {
   public:
    Manchester(Print& p) : _p(p) {}

    size_t packetSize(uint16_t size) {
        return size * 2;
    }

    inline void begin() {}

    void encode(uint8_t b) {
        _p.write((~(b & 0xf) << 4) | (b & 0xf));
        _p.write((~(b >> 4) << 4) | (b >> 4));
    }

    inline void end() {}

    inline void write(uint8_t b) {
        _p.write(b);
    }

   private:
    Print& _p;
};
}  // namespace Encoder
}  // namespace urf
