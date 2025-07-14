// filepath: app/src/sensors/sen0308/sen0308.hpp
#ifndef SEN0308_HPP
#define SEN0308_HPP

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include "sensor.hpp"

struct SoilMoistureData {
    uint16_t raw_adc;
    float voltage;
    float percent; // 0-100%
};

class SEN0308 : public Sensor<SoilMoistureData> {
public:
    explicit SEN0308(const device* adc_dev, uint8_t channel);

    Peripheral::Status init() override;
    bool is_ready() const override;
    etl::string<PERIPHERAL_NAME_SIZE> get_name() const override;
    Status read_data(SoilMoistureData* data) const override;

private:
    const device* m_adc_dev;
    uint8_t m_channel;
    bool m_ready{false};
};

#endif // SEN0308_HPP