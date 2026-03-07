#include "pir.hpp"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

#include "buzzverse/motion_sensor.pb.h"

LOG_MODULE_REGISTER(MotionSensor, LOG_LEVEL_DBG);

void motion_detected_cb(const struct device *dev, struct gpio_callback *cb_data, uint32_t pins) {
	LOG_DBG("MotionSensor: motion detected");
}

MotionSensor::MotionSensor(const struct gpio_dt_spec* gpio_spec)
    : m_gpio_spec(gpio_spec) {}

using Status = Sensor::Status;

Peripheral::Status MotionSensor::init() {
    if (!m_gpio_spec || !device_is_ready(m_gpio_spec->port)) {
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        LOG_WRN("MotionSensor: GPIO device not ready or spec invalid");
        return Peripheral::Status::NOT_READY;
    }

    int ret = gpio_pin_configure_dt(m_gpio_spec, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("MotionSensor: Failed to configure GPIO pin %d (err %d)", m_gpio_spec->pin, ret);
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        return Peripheral::Status::INIT_ERR;
    }

	ret = gpio_pin_interrupt_configure_dt(m_gpio_spec, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
        LOG_ERR("MotionSensor: Failed to configure interrupt on pin %d (err %d)", m_gpio_spec->pin, ret);
        return Peripheral::Status::INIT_ERR;
	}

	gpio_init_callback(&cb_data, motion_detected_cb, BIT(m_gpio_spec->pin));
	ret = gpio_add_callback(m_gpio_spec->port, &cb_data);
	if (ret != 0) {
        LOG_ERR("MotionSensor: Failed to add callback (err %d)", ret);
        return Peripheral::Status::INIT_ERR;
	}

    LOG_INF("MotionSensor: GPIO ready on pin %d", m_gpio_spec->pin);
    status = buzzverse_v1_Status_ComponentState_NORMAL;
    ready = true;
    return Peripheral::Status::OK;
}

Status MotionSensor::read_data(buzzverse_v1_MotionSensorData& data) const {
	/*
    int val = gpio_pin_get_dt(m_gpio_spec);
    if (val < 0) {
        LOG_ERR("MotionSensor: GPIO read failed: %d", val);
        return Status::READ_ERR;
    }

    data.motion_detected = (val > 0);

    LOG_DBG("MotionSensor: GPIO raw: %d, motion detected: %d", val, data.motion_detected);
	*/
    return Status::OK;
}

Status MotionSensor::get_packet(buzzverse_v1_Packet& packet) const {
	/*
    buzzverse_v1_MotionSensorData motion_data = buzzverse_v1_MotionSensorData_init_zero;

    if (read_data(motion_data) != Sensor::Status::OK) {
        return Status::READ_ERR;
    }

    packet = buzzverse_v1_Packet_init_default;
    packet.which_data = buzzverse_v1_Packet_motion_sensor_tag;
    packet.data.motion_sensor = motion_data;
    LOG_DBG("Packet constructed with motion sensor data.");
	*/
    return Status::OK;
}

void MotionSensor::get_status(buzzverse_v1_Status& status_message) const {
    status_message.motion_status = status;
}
