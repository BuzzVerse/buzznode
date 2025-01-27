#ifndef LORAWAN_HANDLER_HPP
#define LORAWAN_HANDLER_HPP

#include <etl/array.h>
#include <etl/string.h>

#include "peripheral.hpp"

class LoRaWANHandler : public Peripheral {
 public:
  static constexpr size_t MAX_MSG_SIZE = 64;

  bool init() override;
  bool is_ready() const override {
    return connected;
  }
  etl::string<64> get_name() const override {
    return "LoRaWAN";
  }

  void send_message(const char* msg) const;

 private:
  bool connected = false;
  etl::array<uint8_t, 8> dev_eui = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  etl::array<uint8_t, 8> join_eui = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  etl::array<uint8_t, 16> app_key = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
};

#endif  // LORAWAN_HANDLER_HPP