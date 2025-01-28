#pragma once

#include <cstdint>

namespace utils {

inline uint8_t hex2byte(const char* hex) {
  uint8_t byte = 0;
  for (int i = 0; i < 2; i++) {
    char c = hex[i];
    byte <<= 4;
    if (c >= '0' && c <= '9') {
      byte |= c - '0';
    } else if (c >= 'A' && c <= 'F') {
      byte |= c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
      byte |= c - 'a' + 10;
    }
  }
  return byte;
}

}
