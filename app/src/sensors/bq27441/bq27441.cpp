#include "bq27441.hpp"

#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "protobufs/buzzverse/bq27441.pb.h"

LOG_MODULE_REGISTER(bq27441, LOG_LEVEL_DBG);

BQ27441::BQ27441(const device* dev) : bq27441_dev(dev) {}

using Status = Sensor<buzzverse_v1_BQ27441Data>::Status;

Peripheral::Status BQ27441::init() {
  if (!device_is_ready(bq27441_dev)) {
    LOG_WRN("BQ27441 device not ready");
    return Peripheral::Status::NOT_READY;
  }

  LOG_INF("BQ27441 device ready");
  ready = true;
  return Peripheral::Status::OK;
}

Status BQ27441::read_data(buzzverse_v1_BQ27441Data* data) const {
  struct sensor_value voltage, current, state_of_charge, remaining_charge_capacity;

  if (sensor_sample_fetch(bq27441_dev) != 0) {
    LOG_ERR("Failed to fetch BQ27441 data");
    return Status::READ_ERR;
  }

  // Read values from sensor channels
  sensor_channel_get(bq27441_dev, SENSOR_CHAN_GAUGE_VOLTAGE, &voltage);
  sensor_channel_get(bq27441_dev, SENSOR_CHAN_GAUGE_AVG_CURRENT, &current);
  sensor_channel_get(bq27441_dev, SENSOR_CHAN_GAUGE_STATE_OF_CHARGE, &state_of_charge);
  sensor_channel_get(bq27441_dev, SENSOR_CHAN_GAUGE_REMAINING_CHARGE_CAPACITY,
                     &remaining_charge_capacity);

  // Convert sensor_value to protobuf-compatible format (e.g., millivolts, milliamps)
  data->voltage_mv = (voltage.val1 * 1000) + (voltage.val2 / 1000);  // Convert V to mV
  data->current_ma = (current.val1 * 1000) + (current.val2 / 1000);  // Convert A to mA
  data->state_of_charge = state_of_charge.val1;                      // Percentage
  data->remaining_capacity_mah = (remaining_charge_capacity.val1 * 1000) +
                                 (remaining_charge_capacity.val2 / 1000);  // Convert Ah to mAh

  LOG_DBG("Voltage: %d mV", data->voltage_mv);
  LOG_DBG("Current: %d mA", data->current_ma);
  LOG_DBG("State of charge: %d%%", data->state_of_charge);
  LOG_DBG("Remaining capacity: %d mAh", data->remaining_capacity_mah);

  return Status::OK;
}