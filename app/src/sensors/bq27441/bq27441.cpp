#include "bq27441.hpp"

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "buzzverse/bq27441.pb.h"

LOG_MODULE_REGISTER(bq27441, LOG_LEVEL_DBG);

BQ27441::BQ27441(const device* dev) : bq27441_dev(dev) {}

using Status = Sensor::Status;

Peripheral::Status BQ27441::init() {
  if (!device_is_ready(bq27441_dev)) {
    LOG_WRN("BQ27441 device not ready");
    return Peripheral::Status::NOT_READY;
  }

  LOG_INF("BQ27441 device ready");
  ready = true;
  return Peripheral::Status::OK;
}

Status BQ27441::read_data(void* data_pointer) const {
  struct sensor_value voltage, current, state_of_charge;

	buzzverse_v1_BQ27441Data* data = static_cast<buzzverse_v1_BQ27441Data*>(data_pointer);

  if (sensor_sample_fetch(bq27441_dev) != 0) {
    LOG_ERR("Failed to fetch BQ27441 data");
    return Status::READ_ERR;
  }

  // Read values from sensor channels
  sensor_channel_get(bq27441_dev, SENSOR_CHAN_GAUGE_VOLTAGE, &voltage);
  sensor_channel_get(bq27441_dev, SENSOR_CHAN_GAUGE_AVG_CURRENT, &current);
  sensor_channel_get(bq27441_dev, SENSOR_CHAN_GAUGE_STATE_OF_CHARGE, &state_of_charge);

  // Convert sensor_value to protobuf-compatible format (e.g., millivolts, milliamps)
  data->voltage_mv = (voltage.val1 * 1000) + (voltage.val2 / 1000);  // Convert V to mV
  data->current_ma = (current.val1 * 1000) + (current.val2 / 1000);  // Convert A to mA
  data->state_of_charge = state_of_charge.val1;                      // Percentage

  LOG_INF("Voltage: %d mV", data->voltage_mv);
  LOG_INF("Current: %d mA", data->current_ma);
  LOG_INF("State of charge: %d%%", data->state_of_charge);

  return Status::OK;
}

Status BQ27441::get_packet(buzzverse_v1_Packet& packet) const {
	// Currently no need to have a BQ27441 packet, as battery data is sent in the Status packet instead.
	return Status::OK;
}

void BQ27441::get_status(buzzverse_v1_Status& status_message) const {
	if(is_ready())
		status_message.bq27441_status = buzzverse_v1_Status_ComponentState_NORMAL;
	else {
		status_message.bq27441_status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
		LOG_WRN("%s failed initialization.", get_name().c_str());
	}
}
