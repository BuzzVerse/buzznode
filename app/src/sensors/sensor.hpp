#ifndef SENSOR_HPP
#define SENSOR_HPP

#include "peripheral.hpp"

template <typename DataType>
class Sensor : public Peripheral {
 public:
  virtual ~Sensor() = default;

  virtual void read_data(DataType* data) const = 0;
};

#endif  // SENSOR_HPP