// filepath: app/src/sensors/sen0308/sen0308.cpp
#include "sen0308.hpp"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sen0308, LOG_LEVEL_DBG);

SEN0308::SEN0308(const device* adc_dev, uint8_t channel)
    : m_adc_dev(adc_dev), m_channel(channel) {}

Peripheral::Status SEN0308::init() {
    if (!device_is_ready(m_adc_dev)) {
        LOG_ERR("ADC device not ready");
        return Peripheral::Status::NOT_READY;
    }
    m_ready = true;
    return Peripheral::Status::OK;
}

bool SEN0308::is_ready() const { return m_ready; }

etl::string<PERIPHERAL_NAME_SIZE> SEN0308::get_name() const { return "SEN0308"; }

Sensor<SoilMoistureData>::Status SEN0308::read_data(SoilMoistureData* data) const {
    if (!m_ready || !data) return Status::READ_ERR;

    struct adc_channel_cfg channel_cfg = {};
    channel_cfg.gain = ADC_GAIN_1;
    channel_cfg.reference = ADC_REF_INTERNAL;
    channel_cfg.acquisition_time = ADC_ACQ_TIME_DEFAULT;
    channel_cfg.channel_id = m_channel;
    adc_channel_setup(m_adc_dev, &channel_cfg);

    struct adc_sequence sequence = {};
    uint16_t sample_buffer;
    sequence.channels = BIT(m_channel);
    sequence.buffer = &sample_buffer;
    sequence.buffer_size = sizeof(sample_buffer);
    sequence.resolution = 12;

    int ret = adc_read(m_adc_dev, &sequence);
    if (ret) {
        LOG_ERR("ADC read failed: %d", ret);
        return Status::READ_ERR;
    }

    data->raw_adc = sample_buffer;
    data->voltage = (sample_buffer / 4095.0f) * 3.3f; // Adjust Vref as needed
    data->percent = (1.0f - (data->voltage / 3.3f)) * 100.0f; // Example mapping

    LOG_DBG("Soil Moisture ADC: %u, V: %.2f, %%: %.1f", data->raw_adc, data->voltage, data->percent);
    return Status::OK;
}