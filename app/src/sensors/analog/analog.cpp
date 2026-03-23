#include "analog.hpp"

#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

#include "buzzverse/analog.pb.h"

LOG_MODULE_REGISTER(Analog, LOG_LEVEL_DBG);
Analog::Analog(const struct adc_dt_spec* adc_spec)
    : m_adc_spec(adc_spec) {}
    
using Status = Sensor::Status;

Peripheral::Status Analog::init() {
    if (!m_adc_spec || !device_is_ready(m_adc_spec->dev)) {
	    status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        LOG_WRN("Analog: Analog device not ready or spec is invalid");
        return Peripheral::Status::NOT_READY;
    }

    int err = adc_channel_setup_dt(m_adc_spec);
    if (err != 0) {
        LOG_ERR("Analog: Failed to setup Analog channel %d (err %d)", m_adc_spec->channel_id, err);
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        return Peripheral::Status::INIT_ERR;
    }

    LOG_INF("Analog: Analog device ready, channel %d configured via DT", m_adc_spec->channel_id);
	status = buzzverse_v1_Status_ComponentState_NORMAL;

    ready = true;
    return Peripheral::Status::OK;
}

Status Analog::read_data(buzzverse_v1_Analog& data) const {
    uint16_t sample_buffer = 0;
    struct adc_sequence sequence = {0};
    int ret = adc_sequence_init_dt(m_adc_spec, &sequence);
    if (ret != 0) {
        LOG_ERR("Analog: Failed to initialize Analog sequence: %d", ret);
        return Status::READ_ERR;
    }
    sequence.buffer = &sample_buffer;
    sequence.buffer_size = sizeof(sample_buffer);

    ret = adc_read_dt(m_adc_spec, &sequence);
    if (ret) {
        LOG_ERR("Analog: Analog read failed: %d", ret);
        return Status::READ_ERR;
    }

    int32_t millivolts = sample_buffer;
    ret = adc_raw_to_millivolts_dt(m_adc_spec, &millivolts);
    if (ret) {
        LOG_ERR("Analog: Failed to convert raw Analog to millivolts: %d", ret);
        return Status::READ_ERR;
    }
    data.millivolts = static_cast<uint16_t>(millivolts);
    data.type = buzzverse_v1_Analog_SensorType_SOIL_MOISTURE;

    LOG_DBG("Analog: Raw Analog: %u, Voltage: %u mV", sample_buffer, data.millivolts);

    return Status::OK;
}

Status Analog::get_packet(buzzverse_v1_Packet& packet) const {
    buzzverse_v1_Analog analog_data = buzzverse_v1_Analog_init_zero;

    if(read_data(analog_data) != Sensor::Status::OK) {
        return Status::READ_ERR;
    }

    packet = buzzverse_v1_Packet_init_default;
    packet.which_data = buzzverse_v1_Packet_analog_tag;
    packet.data.analog = analog_data;
    LOG_DBG("Packet constructed with analog data.");
    return Status::OK;
}

void Analog::get_status(buzzverse_v1_Status& status_message) const {
	status_message.analog_status = status;
}