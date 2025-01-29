#ifndef BQ27441_HPP
#define BQ27441_HPP

#include <etl/string.h>
#include <zephyr/device.h>

#include "protobufs/buzzverse/bq27441.pb.h"
#include "sensor.hpp"

class BQ27441 : public Sensor<buzzverse_v1_BQ27441Data> {
 public:
  explicit BQ27441(const device* dev);

  bool init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "BQ27441";
  }

  void read_data(buzzverse_v1_BQ27441Data* data) const override;

 private:
  const device* bq27441_dev;
  bool ready{false};
};

#endif  // BQ27441_HPP