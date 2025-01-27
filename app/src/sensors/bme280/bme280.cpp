#include "bme280.hpp"

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

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

void BME280::read_data(etl::string<64>& msg, uint32_t counter) const {
  struct sensor_value temp, press, humidity;

  int rc = sensor_sample_fetch(bme280_dev);
  if (rc != 0) {
    LOG_ERR("sensor_sample_fetch failed: %d", rc);
    return;
  }

  sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press);
  sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &humidity);

  double temp_val = temp.val1 + (temp.val2 / 1000000.0);
  double hum_val = humidity.val1 + (humidity.val2 / 1000000.0);
  double pressure_hpa = (press.val1 * 10) + (press.val2 * 10.0 / 1000000.0);

  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%u. %.1f °C, %.1f %%, %.1f hPa", counter, temp_val, hum_val,
           pressure_hpa);
  msg = buffer;
}
