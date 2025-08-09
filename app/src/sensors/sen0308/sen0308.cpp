#include "sen0308.hpp"

#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

#include "buzzverse/sen0308.pb.h"

LOG_MODULE_REGISTER(sen0308, LOG_LEVEL_DBG);

static const struct adc_dt_spec soil_adc_spec = {
    .dev = DEVICE_DT_GET(DT_PARENT(DT_NODELABEL(soil_sensor))),
    .channel_id = DT_REG_ADDR(DT_NODELABEL(soil_sensor)),
    .channel_cfg_dt_node_exists = true,
    .channel_cfg = ADC_CHANNEL_CFG_DT(DT_NODELABEL(soil_sensor)),
    .vref_mv = DT_PROP_OR(DT_NODELABEL(soil_sensor), zephyr_vref_mv, 0),
    .resolution = DT_PROP(DT_NODELABEL(soil_sensor), zephyr_resolution),
    .oversampling = DT_PROP_OR(DT_NODELABEL(soil_sensor), zephyr_oversampling, 0),
};

SEN0308::SEN0308()
    : m_adc_spec(&soil_adc_spec) {}

using Status = Sensor<buzzverse_v1_SEN0308Data>::Status;

Peripheral::Status SEN0308::init() {
    if (!m_adc_spec || !device_is_ready(m_adc_spec->dev)) {
        LOG_WRN("SEN0308: ADC device not ready or spec is invalid");
        return Peripheral::Status::NOT_READY;
    }

    int err = adc_channel_setup_dt(m_adc_spec);
    if (err != 0) {
        LOG_ERR("SEN0308: Failed to setup ADC channel %d (err %d)", m_adc_spec->channel_id, err);
        return Peripheral::Status::INIT_ERR;
    }

    LOG_INF("SEN0308: ADC device ready, channel %d configured via DT", m_adc_spec->channel_id);
    ready = true;
    return Peripheral::Status::OK;
}

Status SEN0308::read_data(buzzverse_v1_SEN0308Data* data) const {
    if (nullptr == data) {
        LOG_ERR("SEN0308: Invalid data pointer");
        return Status::READ_ERR;
    }

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

    // Calculate percent (0-100%) databased on voltage range
    const float VOLTAGE_DRY = 2.97f; // air
    const float VOLTAGE_WET = 0.33f; // water
    float voltage = static_cast<float>(millivolts) / 1000.0f;
    float range = VOLTAGE_DRY - VOLTAGE_WET;
    float percent = (range > 0) ? 100.0f * (VOLTAGE_DRY - voltage) / range : 0.0f;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    data->percent = percent;

    LOG_DBG("SEN0308: Raw ADC: %u, Voltage: %.2f V, Percent: %.1f%%", sample_buffer, voltage, percent);

    return Status::OK;
}