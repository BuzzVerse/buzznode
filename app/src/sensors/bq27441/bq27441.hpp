#ifndef BQ27441_HPP
#define BQ27441_HPP

#include <etl/string.h>
#include <zephyr/device.h>

#include "buzzverse/bq27441.pb.h"
#include "sensor.hpp"

class BQ27441 : public Sensor {
 public:
  explicit BQ27441(const device* dev);

  Peripheral::Status init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "BQ27441";
  }

  Status read_data(void* data_pointer) const override;

  Status get_packet(buzzverse_v1_Packet& packet) const override;

  void set_status(buzzverse_v1_Status& status_message, buzzverse_v1_Status_ComponentState status_component_state) const override;

 private:
  const device* bq27441_dev;
  bool ready{false};
};

#endif  // BQ27441_HPP
