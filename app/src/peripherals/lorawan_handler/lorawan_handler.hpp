#ifndef LORAWAN_HANDLER_HPP
#define LORAWAN_HANDLER_HPP

#include <etl/array.h>
#include <etl/string.h>

#include "peripheral.hpp"
#include "protobufs/buzzverse/packet.pb.h"

class LoRaWANHandler : public Peripheral {
 public:
  LoRaWANHandler();

  static constexpr size_t MAX_MSG_SIZE = 64;

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
  etl::array<uint8_t, 8> dev_eui;
  etl::array<uint8_t, 8> join_eui;
  etl::array<uint8_t, 16> app_key;
};

#endif  // LORAWAN_HANDLER_HPP