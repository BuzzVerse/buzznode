#pragma once

#include <etl/string.h>
#include <zephyr/device.h>

class BME280 {
 public:
  explicit BME280(const device* dev);
  bool init();
  void read_data(etl::string<64>& msg, uint32_t counter) const;
  bool is_ready() const {
    return ready;
  }

 private:
  const device* bme280_dev;
  bool ready{false};
};