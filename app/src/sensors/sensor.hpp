#ifndef SENSOR_HPP
#define SENSOR_HPP

#include "peripheral.hpp"

template <typename DataType>
class Sensor : public Peripheral {
 public:
  virtual ~Sensor() = default;

  /**
   * @brief Sensor-specific status codes
   */
  enum class Status {
    OK = 0,        /**< Read successful */
    READ_ERR = -3, /**< Sensor read failure */
  };

  /**
   * @brief Read data from the sensor
   *
   * @param data Pointer to the data structure to populate
   * @return Status
   * @retval OK if the data is successfully read
   * @retval READ_ERR if the sensor read fails
   */
  virtual Status read_data(DataType* data) const = 0;
};

#endif  // SENSOR_HPP