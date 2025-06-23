#pragma once
#include <Arduino.h>

namespace urf {
namespace Encoder {
class None {
   public:
    None(Print& p) : _p(p) {}

    size_t packetSize(uint16_t size) {
        return size;
    }

    inline void begin() {}

    inline void encode(uint8_t b) {
        _p.write(b);
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