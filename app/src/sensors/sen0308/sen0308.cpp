#include "sen0308.hpp"

#include <zephyr/sys/printk.h> // Using printk for output
#include <zephyr/kernel.h>     // For k_msleep

// Constructor: Now only takes the ADC spec.
SEN0308::SEN0308(const struct adc_dt_spec* adc_spec)
    : m_adc_spec(adc_spec){}

Peripheral::Status SEN0308::init() {
    if (!m_adc_spec || !device_is_ready(m_adc_spec->dev)) {
        printk("ERROR: SEN0308: ADC device not ready or spec is invalid.\n");
        m_ready = false;
        return Peripheral::Status::NOT_READY;
    }

    // Setup the ADC channel.
    int err = adc_channel_setup_dt(m_adc_spec);
    if (err != 0) {
        printk("ERROR: SEN0308: Failed to setup ADC channel %d (err %d)\n", m_adc_spec->channel_id, err);
        m_ready = false;
        return Peripheral::Status::INIT_ERR;
    }

    printk("SEN0308: ADC device ready, channel %d configured via DT\n", m_adc_spec->channel_id);
    m_ready = true;
    return Peripheral::Status::OK;
}

// Checks if the sensor is ready.
bool SEN0308::is_ready() const { return m_ready; }

// Returns the name of the peripheral.
etl::string<PERIPHERAL_NAME_SIZE> SEN0308::get_name() const { return "SEN0308"; }

// Reads data from the sensor.
// Reads data from the sensor.
Sensor<SoilMoistureData>::Status SEN0308::read_data(SoilMoistureData* data) const {
    if (!m_ready || !data) {
        printk("ERROR: SEN0308: Sensor not ready or invalid data pointer.\n");
        return Status::READ_ERR;
    }

    uint16_t sample_buffer;
    struct adc_sequence sequence = {0};
    int ret = adc_sequence_init_dt(m_adc_spec, &sequence);
    if (ret != 0) {
        printk("ERROR: SEN0308: Failed to initialize ADC sequence: %d\n", ret);
        return Status::READ_ERR;
    }
    sequence.buffer = &sample_buffer;
    sequence.buffer_size = sizeof(sample_buffer);

    ret = adc_read_dt(m_adc_spec, &sequence);

    if (ret) {
        printk("ERROR: SEN0308: ADC read failed: %d\n", ret);
        return Status::READ_ERR;
    }

    data->raw_adc = sample_buffer;
    int32_t millivolts = sample_buffer;
    ret = adc_raw_to_millivolts_dt(m_adc_spec, &millivolts);
    if (ret) {
        printk("ERROR: SEN0308: Failed to convert raw ADC to millivolts: %d\n", ret);
        return Status::READ_ERR;
    }
    data->voltage = static_cast<float>(millivolts) / 1000.0f;

    const float VOLTAGE_DRY = 2.97f; // From your measurement in air.
    const float VOLTAGE_WET = 0.40f; // From your measurement in water.

    float range = VOLTAGE_DRY - VOLTAGE_WET;
    if (range <= 0) {
        data->percent = 0; // Avoid division by zero or negative numbers.
    } else {
        float percent = 100.0f * (VOLTAGE_DRY - data->voltage) / range;
        data->percent = (percent < 0.0f) ? 0.0f : (percent > 100.0f) ? 100.0f : percent;
    }

    return Status::OK;
}
