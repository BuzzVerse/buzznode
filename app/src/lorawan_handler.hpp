#pragma once

#include <etl/array.h>
#include <etl/string.h>

class LoRaWANHandler {
 public:
  LoRaWANHandler() = default;
  bool init();
  void send_message(const char* msg) const;
  bool is_connected() const {
    return connected;
  }

 private:
  static constexpr size_t MAX_MSG_SIZE = 64;
  bool connected{false};
  etl::array<uint8_t, 8> dev_eui = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  etl::array<uint8_t, 8> join_eui = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  etl::array<uint8_t, 16> app_key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
};