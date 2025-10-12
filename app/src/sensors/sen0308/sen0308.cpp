#include "sen0308.hpp"

#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

#include "buzzverse/sen0308.pb.h"

LOG_MODULE_REGISTER(sen0308, LOG_LEVEL_DBG);

SEN0308::SEN0308(const struct adc_dt_spec* adc_spec)
    : m_adc_spec(adc_spec) {}
    
using Status = Sensor::Status;

Peripheral::Status SEN0308::init() {
    if (!m_adc_spec || !device_is_ready(m_adc_spec->dev)) {
	    status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        LOG_WRN("SEN0308: ADC device not ready or spec is invalid");
        return Peripheral::Status::NOT_READY;
    }

    int err = adc_channel_setup_dt(m_adc_spec);
    if (err != 0) {
        LOG_ERR("SEN0308: Failed to setup ADC channel %d (err %d)", m_adc_spec->channel_id, err);
        status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
        return Peripheral::Status::INIT_ERR;
    }

    LOG_INF("SEN0308: ADC device ready, channel %d configured via DT", m_adc_spec->channel_id);
	status = buzzverse_v1_Status_ComponentState_NORMAL;

    ready = true;
    return Peripheral::Status::OK;
}

Status SEN0308::read_data(buzzverse_v1_SEN0308Data& data) const {
    uint16_t sample_buffer = 0;
    struct adc_sequence sequence = {0};
    int ret = adc_sequence_init_dt(m_adc_spec, &sequence);
    if (ret != 0) {
        LOG_ERR("SEN0308: Failed to initialize ADC sequence: %d", ret);
        return Status::READ_ERR;
    }
    sequence.buffer = &sample_buffer;
    sequence.buffer_size = sizeof(sample_buffer);

    ret = adc_read_dt(m_adc_spec, &sequence);
    if (ret) {
        LOG_ERR("SEN0308: ADC read failed: %d", ret);
        return Status::READ_ERR;
    }

    int32_t millivolts = sample_buffer;
    ret = adc_raw_to_millivolts_dt(m_adc_spec, &millivolts);
    if (ret) {
        LOG_ERR("SEN0308: Failed to convert raw ADC to millivolts: %d", ret);
        return Status::READ_ERR;
    }
    data.millivolts = static_cast<uint16_t>(millivolts);

    LOG_DBG("SEN0308: Raw ADC: %u, Voltage: %u mV", sample_buffer, data.millivolts);

    return Status::OK;
}

Status SEN0308::get_packet(buzzverse_v1_Packet& packet) const {
    buzzverse_v1_SEN0308Data sen0308_data = buzzverse_v1_SEN0308Data_init_zero;

    if(read_data(sen0308_data) != Sensor::Status::OK) {
        return Status::READ_ERR;
    }

    packet = buzzverse_v1_Packet_init_default;
    packet.which_data = buzzverse_v1_Packet_sen0308_tag;
    packet.data.sen0308 = sen0308_data;
    LOG_DBG("Packet constructed with SEN0308 data.");
    return Status::OK;
}

void SEN0308::get_status(buzzverse_v1_Status& status_message) const {
	status_message.sen0308_status = status;
}