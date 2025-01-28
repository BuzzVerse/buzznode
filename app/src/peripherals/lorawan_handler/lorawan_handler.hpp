#ifndef LORAWAN_HANDLER_HPP
#define LORAWAN_HANDLER_HPP

#include <etl/array.h>
#include <etl/string.h>

#include "peripheral.hpp"

class LoRaWANHandler : public Peripheral {
 public:
  LoRaWANHandler();

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
  bool connected{false};
  etl::array<uint8_t, 8> dev_eui;
  etl::array<uint8_t, 8> join_eui;
  etl::array<uint8_t, 16> app_key;
};

#endif  // LORAWAN_HANDLER_HPP