#include <etl/vector.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "sensors/bme280/bme280.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DELAY K_SECONDS(10)

int main() {
  const device* const bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);

  BME280 bme280(bme280_dev);
  LoRaWANHandler lorawan;

  etl::vector<Peripheral*, 2> peripherals = {&bme280, &lorawan};

  // Initialize all peripherals
  for (auto* peripheral : peripherals) {
    if (!peripheral->init()) {
      LOG_ERR("%s initialization failed", peripheral->get_name().c_str());
    }
  }

  uint32_t counter = 0;

  while (1) {
    etl::string<64> msg;

    if (bme280.is_ready()) {
      bme280.read_data(msg, counter);
    } else {
      msg = "BME280 not ready";
    }

    if (lorawan.is_ready()) {
      lorawan.send_message(msg.c_str());
    } else {
      LOG_INF("Message not sent: %s", msg.c_str());
    }

    counter++;
    k_sleep(DELAY);
  }

  return 0;
}