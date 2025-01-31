#include "bme280.hpp"

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "buzzverse/bme280.pb.h"

LOG_MODULE_REGISTER(bme280, LOG_LEVEL_DBG);

BME280::BME280(const device* dev) : bme280_dev(dev) {}

bool BME280::init() {
  if (!device_is_ready(bme280_dev)) {
    LOG_WRN("BME280 device not ready");
    return false;
  }

  LOG_INF("BME280 device ready");
  ready = true;
  return true;
}

void BME280::read_data(buzzverse_v1_BME280Data* data) const {
  struct sensor_value temp, press, humidity;

  if (sensor_sample_fetch(bme280_dev) != 0) {
    LOG_ERR("Failed to fetch BME280 data");
    return;
  }

  sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press);
  sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &humidity);

  // Convert sensor values to protobuf-compatible integers
  data->temperature =
    static_cast<uint32_t>((temp.val1 * 100) + (temp.val2 / 10000));  // centi-degrees
  data->pressure = static_cast<uint32_t>((press.val1 * 10) + (press.val2 / 100000));        // hPa
  data->humidity = static_cast<uint32_t>((humidity.val1 * 10) + (humidity.val2 / 100000));  // %
}