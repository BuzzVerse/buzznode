#ifndef SEN0308_HPP
#define SEN0308_HPP

#include <etl/string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h> // Zephyr's ADC driver header

#include "../peripherals/peripheral.hpp"
#include "../sensor.hpp"

// Data structure for the soil moisture sensor readings
struct SoilMoistureData {
    uint16_t raw_adc;
    float voltage;
    float percent; // 0-100%
};

class SEN0308 : public Sensor<SoilMoistureData> {
public:
    /**
     * @brief Constructor for the SEN0308 sensor.
     * @param adc_spec A pointer to the ADC device tree specification struct.
     * This is obtained using ADC_DT_SPEC_GET() in your main application.
     */
    explicit SEN0308(const struct adc_dt_spec* adc_spec);

    // Initializes the sensor hardware (ADC channel setup).
    Peripheral::Status init() override;

    // Checks if the sensor is ready for reading.
    bool is_ready() const override;

    // Returns the name of the sensor.
    etl::string<PERIPHERAL_NAME_SIZE> get_name() const override;

    // Reads data from the sensor and populates the SoilMoistureData struct.
    Status read_data(SoilMoistureData* data) const override;

private:
    // A pointer to the ADC specification struct from the device tree.
    const struct adc_dt_spec* m_adc_spec;

    bool m_ready{false}; // Flag to indicate if the sensor is initialized and ready
};

#endif // SEN0308_HPP
