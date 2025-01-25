#include <etl/string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>

#include "bme280.hpp"
#include "lorawan_handler.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DELAY K_SECONDS(10)

int main(void) {
  const device* const bme280_dev = DEVICE_DT_GET_ANY(bosch_bme280);

  BME280 bme280(bme280_dev);
  LoRaWANHandler lorawan;

  bool bme280_ready = bme280.init();
  bool lorawan_connected = lorawan.init();

  if (!lorawan_connected && !bme280_ready) {
    LOG_ERR("No devices available, stopping application");
    return -ENODEV;
  }

  uint32_t counter = 0;

  while (1) {
    etl::string<64> msg;

    if (bme280_ready) {
      bme280.read_data(msg, counter);
    } else {
      char buffer[64];
      snprintf(buffer, sizeof(buffer), "%u", counter);
      msg = buffer;
    }

    if (lorawan_connected) {
      lorawan.send_message(msg.c_str());
    } else {
      LOG_INF("BME280 (not sent): %s", msg.c_str());
    }

    counter++;
    k_sleep(DELAY);
  }

  return 0;
}
