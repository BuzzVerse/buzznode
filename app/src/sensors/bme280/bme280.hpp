#ifndef BME280_HPP
#define BME280_HPP

#include <etl/string.h>
#include <zephyr/device.h>

#include "buzzverse/bme280.pb.h"
#include "sensor.hpp"

class BME280 : public Sensor {
 public:
  explicit BME280(const device* dev);

  Peripheral::Status init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "BME280";
  }

  Status get_packet(buzzverse_v1_Packet& packet) const override;

  void get_status(buzzverse_v1_Status& status_message) const override;

 private:
  const device* bme280_dev;
  bool ready{false};
  buzzverse_v1_Status_ComponentState status{buzzverse_v1_Status_ComponentState_STATE_UNSPECIFIED};

  /**
   * @brief Read data from the sensor
   *
   * @param data Pointer to the BME280 data structure to populate
   * @return Status
   * @retval OK if the data is successfully read
   * @retval READ_ERR if the sensor read fails
   */
  Status read_data(buzzverse_v1_BME280Data* data) const;

};

#endif  // BME280_HPP
