#ifndef DUST_SENSOR_HPP
#define DUST_SENSOR_HPP

#include <etl/string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include "buzzverse/analog.pb.h"

#include "../peripherals/peripheral.hpp"
#include "../sensor.hpp"

class DustSensor : public Sensor {
public:
    DustSensor(const struct adc_dt_spec* adc_spec, const struct gpio_dt_spec* iled_spec);

    Peripheral::Status init() override;

    bool is_ready() const override {
        return ready;
    };

    etl::string<PERIPHERAL_NAME_SIZE> get_name() const override {
        return "DustSensor";
    }

    Status get_packet(buzzverse_v1_Packet& packet) const override;

    void get_status(buzzverse_v1_Status& status_message) const override;

private:
    buzzverse_v1_Status_ComponentState status{buzzverse_v1_Status_ComponentState_STATE_UNSPECIFIED};

    Sensor::Status read_data(buzzverse_v1_Analog& data) const;

    const struct adc_dt_spec* m_adc_spec;
    const struct gpio_dt_spec* m_iled_spec;

    bool ready{false};
};

#endif // DUST_SENSOR_HPP
