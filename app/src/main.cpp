#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h> // Using printk for output
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/adc.h> // Include for adc_dt_spec

#include "Application.hpp"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "utils/banner.hpp"
#include "utils/sleep-manager.hpp"
#include "sensors/sen0308/sen0308.hpp"

#ifdef CONFIG_SLEEP_TIME_MS
  #define APP_SLEEP_DURATION_MS CONFIG_SLEEP_TIME_MS
#else
  #define APP_SLEEP_DURATION_MS 10000
#endif

// ADC specification for the soil sensor.
static const struct adc_dt_spec soil_adc_spec = {
    .dev = DEVICE_DT_GET(DT_PARENT(DT_NODELABEL(soil_sensor))),
    .channel_id = DT_REG_ADDR(DT_NODELABEL(soil_sensor)),
    .channel_cfg_dt_node_exists = true,
    .channel_cfg = ADC_CHANNEL_CFG_DT(DT_NODELABEL(soil_sensor)),
    .vref_mv = DT_PROP_OR(DT_NODELABEL(soil_sensor), zephyr_vref_mv, 0),
    .resolution = DT_PROP(DT_NODELABEL(soil_sensor), zephyr_resolution),
    .oversampling = DT_PROP_OR(DT_NODELABEL(soil_sensor), zephyr_oversampling, 0),
};

// The GPIO spec for the enable pin is no longer needed.

int main(void) {
  // Add a long delay at the very start of main.
  // This gives the continuously-powered sensor plenty of time to stabilize
  // after the whole board powers on, fulfilling the "bigger wait" request.
  printk("Waiting for sensor to stabilize after power-on...\n");
  k_msleep(5000); // Wait 5 seconds

  printk("%s\n", APP_ASCII_BANNER);
  printk("===== Buzzverse Node System Booting =====\n");

  BME280 bme280(DEVICE_DT_GET_ANY(bosch_bme280));
  // Array of available sensors
  etl::array<etl::unique_ptr<Sensor>, NUMBER_OF_SENSORS> sensors {
	etl::unique_ptr<BME280>(etl::move(&bme280)),
  };

  BQ27441 bq27441(DEVICE_DT_GET_ANY(ti_bq274xx));
  LoRaWANHandler lorawan(bq27441);
  // Construct the sensor object, passing only the ADC spec.
  SEN0308 soil_sensor(&soil_adc_spec);
  const device* adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
  soil_sensor.init();

  etl::unique_ptr<SleepManager> p_sleep_manager(nullptr);
#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  p_sleep_manager = etl::unique_ptr<SleepManager>(new SleepManager);
#endif

  Application app(sensors, lorawan, etl::move(p_sleep_manager));
  
  if (soil_sensor.init() != Peripheral::Status::OK) {
      printk("ERROR: SEN0308 initialization failed. Halting.\n");
      while (true) { k_sleep(K_SECONDS(1)); }
  } else {
      printk("SEN0308 initialized successfully.\n");
  }

  while (true)
  {
    printk("Reading soil moisture data...\n");
    SoilMoistureData soil_data;
    if (soil_sensor.read_data(&soil_data) == Sensor<SoilMoistureData>::Status::OK) {
        printk("Soil Moisture Data: Raw ADC: %d, Voltage: %.2f V, Percent: %.2f %%\n",
                soil_data.raw_adc,
                (double)soil_data.voltage,
                (double)soil_data.percent);
    } else {
        printk("ERROR: Failed to read soil moisture data.\n");
    }
    k_msleep(APP_SLEEP_DURATION_MS);
  }

  app.run_cycle();

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  app.enter_low_power_mode(APP_SLEEP_DURATION_MS);
  LOG_WRN("Execution continued after enter_low_power_mode - this is unexpected for deep sleep.");
#else
  LOG_INF("Device sleep not enabled. Entering polling loop.");
  while (true) {
    k_sleep(K_MSEC(APP_SLEEP_DURATION_MS));
    app.run_cycle();
  }
#endif

  return 0;  // Should not be reached
}
