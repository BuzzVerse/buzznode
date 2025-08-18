#ifndef BME280_HPP
#define BME280_HPP

#include <etl/string.h>
#include <zephyr/device.h>

#include "buzzverse/bme280.pb.h"
#include "sensor.hpp"

class BME280 : public Sensor {
 public:
  explicit BME280(const device* dev);

  Peripheral::Status init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "BME280";
  }

  Status read_data(void* data_pointer) const override;

	void set_status(buzzverse_v1_Status& status_message, buzzverse_v1_Status_ComponentState status_component_state) const override;

 private:
  const device* bme280_dev;
  bool ready{false};
};

#endif  // BME280_HPP
