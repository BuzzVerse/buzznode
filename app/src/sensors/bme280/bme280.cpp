#include "bme280.hpp"

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "buzzverse/bme280.pb.h"

LOG_MODULE_REGISTER(bme280, LOG_LEVEL_DBG);

BME280::BME280(const device* dev) : bme280_dev(dev) {}

using Status = Sensor::Status;

Peripheral::Status BME280::init() {
  if (!device_is_ready(bme280_dev)) {
    LOG_WRN("BME280 device not ready");
	status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
    return Peripheral::Status::NOT_READY;
  }

  LOG_INF("BME280 device ready");
  ready = true;
  status = buzzverse_v1_Status_ComponentState_NORMAL;
  return Peripheral::Status::OK;
}

Status BME280::read_data(buzzverse_v1_BME280Data* data) const {
  struct sensor_value temp, press, humidity;

  if (nullptr == data) {
    LOG_ERR("Invalid data pointer");
    return Status::READ_ERR;
  }

  if (sensor_sample_fetch(bme280_dev) != 0) {
    LOG_ERR("Failed to fetch BME280 data");
    return Status::READ_ERR;
  }

  sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
  sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press);
  sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &humidity);

  // Convert temperature to whole degrees (-128 to 127)
  data->temperature = static_cast<int8_t>(temp.val1);

  double total_pressure_kPa = static_cast<double>(press.val1) + static_cast<double>(press.val2) / 1000000.0;
  double pressure_hPa = total_pressure_kPa * 10.0;

  int32_t rounded_pressure_hPa = static_cast<int32_t>(round(pressure_hPa));

  // Convert pressure as difference from 1000 hPa (-128 to 127)
  data->pressure = static_cast<int8_t>(rounded_pressure_hPa - 1000);

  // Convert humidity to whole percentage (0-100%)
  data->humidity = static_cast<uint8_t>(humidity.val1);

  LOG_INF("Temperature: %d C", data->temperature);
  LOG_INF("Pressure (Difference from 1000 hPa): %d hPa", data->pressure);
  LOG_INF("Humidity: %d %%", data->humidity);

  return Status::OK;
}

Status BME280::get_packet(buzzverse_v1_Packet& packet) const {
	buzzverse_v1_BME280Data bme_data = buzzverse_v1_BME280Data_init_zero;

	if(read_data(&bme_data) != Sensor::Status::OK) {
		return Status::READ_ERR;
	}

	bool bme_has_valid_data =
		!(bme_data.temperature == 0 && bme_data.pressure == 0 && bme_data.humidity == 0);

	if (!bme_has_valid_data) {
		LOG_WRN("No valid BME280 data to construct an application packet.");
		return Status::READ_ERR;
	}

	packet = buzzverse_v1_Packet_init_default;
	packet.which_data = buzzverse_v1_Packet_bme280_tag;
	packet.data.bme280 = bme_data;
	LOG_DBG("Packet constructed with BME280 data.");

	return Status::OK;
}

void BME280::get_status(buzzverse_v1_Status& status_message) const {
	status_message.bme280_status = status;
}
