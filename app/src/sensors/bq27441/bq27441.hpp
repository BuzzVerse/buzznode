#ifndef BQ27441_HPP
#define BQ27441_HPP

#include <etl/string.h>
#include <zephyr/device.h>

#include "buzzverse/bq27441.pb.h"
#include "sensor.hpp"

class BQ27441 : public Sensor {
 public:
  explicit BQ27441();

  Peripheral::Status init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "BQ27441";
  }

  Status get_packet(buzzverse_v1_Packet& packet) const override;

  void get_status(buzzverse_v1_Status& status_message) const override;
 private:
  const device* bq27441_dev;
  bool ready{false};
  buzzverse_v1_Status_ComponentState status{buzzverse_v1_Status_ComponentState_STATE_UNSPECIFIED};

  /**
   * @brief Read data from the sensor
   *
   * @param data BQ27441 data structure to populate
   * @return Status
   * @retval OK if the data is successfully read
   * @retval READ_ERR if the sensor read fails
   */
  Status read_data(buzzverse_v1_BQ27441Data& data) const;
};

#endif  // BQ27441_HPP
