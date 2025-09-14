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
   * @param data_pointer Pointer to the data structure to populate
   * @return Status
   * @retval OK if the data is successfully read
   * @retval READ_ERR if the sensor read fails
   */
  virtual Status read_data(void* data_pointer) const = 0;

  /**
   * @brief Read data from the sensor and get a packet containing the readout
   *
   * @param packet Reference to a packet to populate
   * @return Status
   * @retval OK if the data is successfully read and packet is constructed
   * @retval READ_ERR if the sensor read fails
   */
  virtual Status get_packet(buzzverse_v1_Packet& packet) const = 0;

  /**
   * @brief Sets value of a field in the status message struct corresponding to the correct sensor
   *
   * @param status_message Reference to the status message struct
   * @param status_component_state Status state to be set for a particular sensor
   */
  virtual void get_status(buzzverse_v1_Status& status_message) const = 0;
};

#endif  // SENSOR_HPP
