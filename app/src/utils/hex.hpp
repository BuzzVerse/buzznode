#pragma once

namespace utils {

template <typename T>
T hex2val(const char* hex) {
  T result = 0;
  constexpr size_t DIGITS = sizeof(T) * 2;

  for (size_t i = 0; i < DIGITS; i++) {
    result <<= 4;
    char c = hex[i];
    if (c >= '0' && c <= '9') {
      result |= (c - '0');
    } else if (c >= 'A' && c <= 'F') {
      result |= (c - 'A' + 10);
    } else if (c >= 'a' && c <= 'f') {
      result |= (c - 'a' + 10);
    } else {
      return 0;
    }
  }

  return result;
}

}  // namespace utils
