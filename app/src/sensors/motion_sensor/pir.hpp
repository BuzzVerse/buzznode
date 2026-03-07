#ifndef PIR_HPP
#define PIR_HPP

#include <etl/string.h>
#include <zephyr/drivers/gpio.h>

#include "buzzverse/motion_sensor.pb.h"
#include "sensors/sensor.hpp"

static struct gpio_callback cb_data;
void motion_detected_cb(const struct device *dev, struct gpio_callback *cb_data, uint32_t pins);

class MotionSensor : public Sensor {
 public:
  explicit MotionSensor(const struct gpio_dt_spec* gpio_spec);

  Peripheral::Status init() override;

  bool is_ready() const override {
    return ready;
  }

  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
    return "MotionSensor";
  }

  Status get_packet(buzzverse_v1_Packet& packet) const override;

  void get_status(buzzverse_v1_Status& status_message) const override;

 private:
  const struct gpio_dt_spec* m_gpio_spec;
  bool ready{false};
  buzzverse_v1_Status_ComponentState status{buzzverse_v1_Status_ComponentState_STATE_UNSPECIFIED};

  /**
   * @brief Read data from the sensor
   *
   * @param data MotionSensor data structure to populate
   * @return Status
   * @retval OK if the data is successfully read
   * @retval READ_ERR if the sensor read fails
   */
  Status read_data(buzzverse_v1_MotionSensorData& data) const;
};

#endif  // PIR_HPP
