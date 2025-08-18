#ifndef SENSOR_HPP
#define SENSOR_HPP

#include "peripheral.hpp"
#include "buzzverse/packet.pb.h"

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
  virtual Status read_data(void* data_pointer) const = 0;

	virtual void set_status(buzzverse_v1_Status& status_message, buzzverse_v1_Status_ComponentState status_component_state) const = 0;
};

#endif  // SENSOR_HPP
