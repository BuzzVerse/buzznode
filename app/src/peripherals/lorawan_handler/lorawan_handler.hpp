#ifndef LORAWAN_HANDLER_HPP
#define LORAWAN_HANDLER_HPP

#include <etl/array.h>
#include <etl/string.h>

#include "peripheral.hpp"
#include "protobufs/buzzverse/bq27441.pb.h"
#include "protobufs/buzzverse/packet.pb.h"
#include "sensors/sensor.hpp"

class LoRaWANHandler : public Peripheral {
 public:
  explicit LoRaWANHandler(Sensor<buzzverse_v1_BQ27441Data>& battery_sensor);

  static constexpr size_t MAX_MSG_SIZE = 64;

  /**
   * @brief LoRaWAN-specific status codes
   */
  enum class Status {
    OK = 0,         /**< Operation successful */
    JOIN_ERR = -3,  /**< Failed to join the LoRaWAN network */
    SEND_ERR = -4,  /**< Failed to send a packet */
    ENCODE_ERR = -5 /**< Failed to encode the message */
  };

  Peripheral::Status init() override;

  bool is_ready() const override {
    return connected;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "LoRaWAN";
  }

  Status send_packet(const buzzverse_v1_Packet& packet) const;

 private:
  bool connected{false};
  etl::array<uint8_t, 8> dev_eui;
  etl::array<uint8_t, 8> join_eui;
  etl::array<uint8_t, 16> app_key;
  static Sensor<buzzverse_v1_BQ27441Data>* battery_sensor;
  static uint8_t battery_level_callback();
};

#endif  // LORAWAN_HANDLER_HPP