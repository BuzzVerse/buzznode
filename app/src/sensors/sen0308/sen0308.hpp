#ifndef SEN0308_HPP
#define SEN0308_HPP

#include <etl/string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>

#include "buzzverse/sen0308.pb.h"

#include "../peripherals/peripheral.hpp"
#include "../sensor.hpp"

class SEN0308 : public Sensor {
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
    bool is_ready() const override {
        return ready;
    };

    // Returns the name of the sensor.
    etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
        return "SEN0308";
    }

    Status get_packet(buzzverse_v1_Packet& packet) const override;

    void get_status(buzzverse_v1_Status& status_message) const override;

private:
    buzzverse_v1_Status_ComponentState status{buzzverse_v1_Status_ComponentState_STATE_UNSPECIFIED};

    // Reads data from the sensor and populates the SoilMoistureData struct.
    Sensor::Status read_data(buzzverse_v1_SEN0308Data& data) const;

    // A pointer to the ADC specification struct from the device tree.
    const struct adc_dt_spec* m_adc_spec;

    bool ready{false}; // Flag to indicate if the sensor is initialized and ready
};

#endif // SEN0308_HPP