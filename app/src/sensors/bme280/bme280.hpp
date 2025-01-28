#ifndef BME280_HPP
#define BME280_HPP

#include <etl/string.h>
#include <zephyr/device.h>

#include "sensor.hpp"

class BME280 : public Sensor {
 public:
  explicit BME280(const device* dev);

  bool init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<64> get_name() const override {
    return "BME280";
  }

  void read_data(etl::string<64>& msg, uint32_t counter) const override;

 private:
  const device* bme280_dev;
  bool ready{false};
};

#endif  // BME280_HPP