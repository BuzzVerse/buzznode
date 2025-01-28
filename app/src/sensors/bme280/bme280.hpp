#ifndef BME280_HPP
#define BME280_HPP

#include <etl/string.h>
#include <zephyr/device.h>

#include "protobufs/buzzverse/bme280.pb.h"
#include "sensor.hpp"

class BME280 : public Sensor<buzzverse_v1_BME280Data> {
 public:
  explicit BME280(const device* dev);

  bool init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "BME280";
  }

  void read_data(buzzverse_v1_BME280Data* data) const override;

 private:
  const device* bme280_dev;
  bool ready{false};
};

#endif  // BME280_HPP