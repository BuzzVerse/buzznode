#ifndef LORAWAN_HANDLER_HPP
#define LORAWAN_HANDLER_HPP

#include <etl/array.h>
#include <etl/string.h>

#include "buzzverse/packet.pb.h"
#include "peripheral.hpp"

class LoRaWANHandler : public Peripheral {
 public:
  LoRaWANHandler();

  static constexpr uint8_t LORAWAN_PORT = 2;

  static constexpr size_t MAX_MSG_SIZE = 64;
  static constexpr size_t EUI_SIZE = 8;
  static constexpr size_t KEY_SIZE = 16;

  bool init() override;

  bool is_ready() const override {
    return connected;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "LoRaWAN";
  }

  bool send_packet(const buzzverse_v1_Packet& packet) const;

 private:
  bool connected{false};
  etl::array<uint8_t, EUI_SIZE> dev_eui;
  etl::array<uint8_t, EUI_SIZE> join_eui;
  etl::array<uint8_t, KEY_SIZE> app_key;
};

#endif  // LORAWAN_HANDLER_HPP