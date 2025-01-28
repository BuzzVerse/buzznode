#pragma once

#include <etl/array.h>
#include <etl/string.h>

class LoRaWANHandler {
 public:
  LoRaWANHandler();
  bool init();
  void send_message(const char* msg) const;
  bool is_connected() const {
    return connected;
  }

 private:
  static constexpr size_t MAX_MSG_SIZE = 64;
  bool connected{false};
  etl::array<uint8_t, 8> dev_eui;
  etl::array<uint8_t, 8> join_eui;
  etl::array<uint8_t, 16> app_key;
};