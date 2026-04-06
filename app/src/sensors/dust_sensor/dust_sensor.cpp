#include "dust_sensor.hpp"
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(DustSensor, LOG_LEVEL_DBG);

DustSensor::DustSensor(const struct adc_dt_spec* adc_spec, const struct gpio_dt_spec* iled_spec)
    : m_adc_spec(adc_spec), m_iled_spec(iled_spec) {}

using Status = Sensor::Status;

Peripheral::Status DustSensor::init() {
    if (!m_adc_spec || !device_is_ready(m_adc_spec->dev)) {
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        LOG_WRN("DustSensor: ADC device not ready or spec is invalid");
        return Peripheral::Status::NOT_READY;
    }

    if (!m_iled_spec || !gpio_is_ready_dt(m_iled_spec)) {
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        LOG_WRN("DustSensor: GPIO device not ready or spec is invalid");
        return Peripheral::Status::NOT_READY;
    }

    int err = adc_channel_setup_dt(m_adc_spec);
    if (err != 0) {
        LOG_ERR("DustSensor: Failed to setup ADC channel %d (err %d)", m_adc_spec->channel_id, err);
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        return Peripheral::Status::INIT_ERR;
    }

    err = gpio_pin_configure_dt(m_iled_spec, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("DustSensor: Failed to configure ILED GPIO (err %d)", err);
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        return Peripheral::Status::INIT_ERR;
    }

    LOG_INF("DustSensor: Ready, channel %d and ILED pin %d configured", m_adc_spec->channel_id, m_iled_spec->pin);
    status = buzzverse_v1_Status_ComponentState_NORMAL;
    ready = true;
    return Peripheral::Status::OK;
}

Status DustSensor::read_data(buzzverse_v1_Analog& data) const {
    uint16_t sample_buffer = 0;
    struct adc_sequence sequence = {0};
    int ret = adc_sequence_init_dt(m_adc_spec, &sequence);
    if (ret != 0) {
        LOG_ERR("DustSensor: Failed to initialize ADC sequence: %d", ret);
        return Status::READ_ERR;
    }
    sequence.buffer = &sample_buffer;
    sequence.buffer_size = sizeof(sample_buffer);

    gpio_pin_set_dt(m_iled_spec, 1);
    
    k_busy_wait(280);

    ret = adc_read_dt(m_adc_spec, &sequence);
    if (ret) {
        LOG_ERR("DustSensor: ADC read failed: %d", ret);
        gpio_pin_set_dt(m_iled_spec, 0);
        return Status::READ_ERR;
    }

    k_busy_wait(40);

    gpio_pin_set_dt(m_iled_spec, 0);

    int32_t millivolts = sample_buffer;
    ret = adc_raw_to_millivolts_dt(m_adc_spec, &millivolts);
    if (ret) {
        LOG_ERR("DustSensor: Failed to convert raw ADC to millivolts: %d", ret);
        return Status::READ_ERR;
    }

    data.millivolts = static_cast<uint16_t>(millivolts);
    data.type = buzzverse_v1_Analog_SensorType_DUST_SENSOR;

    LOG_DBG("DustSensor: Voltage: %d mV", millivolts);

    return Status::OK;
}

Status DustSensor::get_packet(buzzverse_v1_Packet& packet) const {
    buzzverse_v1_Analog analog_data = buzzverse_v1_Analog_init_zero;

    if(read_data(analog_data) != Sensor::Status::OK) {
        return Status::READ_ERR;
    }

    packet = buzzverse_v1_Packet_init_default;
    packet.which_data = buzzverse_v1_Packet_analog_tag;
    packet.data.analog = analog_data;
    LOG_DBG("Packet constructed with dust sensor data (as analog).");
    return Status::OK;
}

void DustSensor::get_status(buzzverse_v1_Status& status_message) const {
    status_message.analog_status = status;
}
