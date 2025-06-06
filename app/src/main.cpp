#include <etl/vector.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "buzzverse/bme280.pb.h"
#include "buzzverse/packet.pb.h"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "utils/packet_handler.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DELAY_10_SEC K_SECONDS(10)
#define DELAY_20_SEC K_SECONDS(20)
#define DELAY_30_SEC K_SECONDS(30)

int main(void) {
  const device* const bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);
  const device* const bq27441_dev = DEVICE_DT_GET_ANY(ti_bq274xx);
  constexpr uint8_t DEVICE_AMOUNT = 3;

  BME280 bme280(bme280_dev);
  BQ27441 bq27441(bq27441_dev);

  LoRaWANHandler lorawan(bq27441);

  etl::vector<Peripheral*, DEVICE_AMOUNT> peripherals = {&bme280, &bq27441, &lorawan};

  for (auto* peripheral : peripherals) {
    if (peripheral->init() != Peripheral::Status::OK) {
      LOG_ERR("%s initialization failed", peripheral->get_name().c_str());
    }
  }

  while (true) {
    buzzverse_v1_BME280Data bme280_data = buzzverse_v1_BME280Data_init_zero;
    bme280.read_data(&bme280_data);

    // Create and populate a Packet protobuf message
    buzzverse_v1_Packet packet = buzzverse_v1_Packet_init_zero;
    packet.which_data = buzzverse_v1_Packet_bme280_tag;
    packet.data.bme280 = bme280_data;

    // Send the packet using LoRaWANHandler
    if (!lorawan.is_ready() || (lorawan.send_packet(packet) != LoRaWANHandler::Status::OK)) {
      LOG_ERR("Failed to send packet");
    }

    k_sleep(DELAY_10_SEC);
  }

  return 0;
}
