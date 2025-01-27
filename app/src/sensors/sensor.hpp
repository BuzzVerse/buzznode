#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <etl/string.h>

#include "peripheral.hpp"

class Sensor : public Peripheral {
 public:
  virtual ~Sensor() = default;

  // Read data from the sensor
  virtual void read_data(etl::string<64>& msg, uint32_t counter) const = 0;
};

#endif  // SENSOR_HPP