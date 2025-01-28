#include <etl/vector.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "protobufs/buzzverse/bme280.pb.h"
#include "protobufs/buzzverse/packet.pb.h"
#include "sensors/bme280/bme280.hpp"
#include "utils/packet_handler.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DELAY K_SECONDS(10)

// TODO: Remove this when ready
void test_encode_decode();

int main(void) {
  const device* const bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);

  BME280 bme280(bme280_dev);
  LoRaWANHandler lorawan;

  etl::vector<Peripheral*, 2> peripherals = {&bme280, &lorawan};

  for (auto* peripheral : peripherals) {
    if (!peripheral->init()) {
      LOG_ERR("%s initialization failed", peripheral->get_name().c_str());
    }
  }

  while (true) {
    // Create and populate a BME280Data protobuf message
    buzzverse_v1_BME280Data bme280_data = buzzverse_v1_BME280Data_init_zero;
    if (bme280.is_ready()) {
      bme280.read_data(&bme280_data);
    } else {
      LOG_ERR("BME280 not ready");
      k_sleep(DELAY);
      continue;
    }

    // Create and populate a Packet protobuf message
    buzzverse_v1_Packet packet = buzzverse_v1_Packet_init_zero;
    packet.which_data = buzzverse_v1_Packet_bme280_tag;
    packet.data.bme280 = bme280_data;

    // TODO: Remove this when ready
    test_encode_decode();

    // Send the packet using LoRaWANHandler
    if (!lorawan.is_ready() || !lorawan.send_packet(packet)) {
      LOG_ERR("Failed to send packet");
    }

    k_sleep(DELAY);
  }

  return 0;
}

// TODO: Remove this when ready
void test_encode_decode() {
  buzzverse_v1_Packet packet = buzzverse_v1_Packet_init_zero;
  packet.which_data = buzzverse_v1_Packet_bme280_tag;
  packet.data.bme280.temperature = 1;
  packet.data.bme280.pressure = 1;
  packet.data.bme280.humidity = 1;

  uint8_t buffer[64];
  size_t size = 0;

  if (!PacketHandler::encode(packet, buffer, size)) {
    LOG_ERR("Failed to encode packet");
    return;
  }

  // print encoded packet bytes
  LOG_INF("Encoded packet: ");
  for (size_t i = 0; i < size; i++) {
    LOG_INF("0x%02x", buffer[i]);
  }

  buzzverse_v1_Packet decoded_packet = buzzverse_v1_Packet_init_zero;
  if (!PacketHandler::decode(buffer, size, decoded_packet)) {
    LOG_ERR("Failed to decode packet");
    return;
  }

  LOG_INF("Decoded packet: temperature=%d, pressure=%d, humidity=%d",
          decoded_packet.data.bme280.temperature, decoded_packet.data.bme280.pressure,
          decoded_packet.data.bme280.humidity);
}