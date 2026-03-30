#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/drivers/adc.h>

#include "Application.hpp"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/analog/analog.hpp"
#include "sensors/dust_sensor/dust_sensor.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "utils/banner.hpp"
#include "utils/sleep-manager.hpp"

LOG_MODULE_REGISTER(main_entry, LOG_LEVEL_DBG);

#ifdef CONFIG_SLEEP_TIME_MS
  #define APP_SLEEP_DURATION_MS CONFIG_SLEEP_TIME_MS
#else
  #define APP_SLEEP_DURATION_MS 10000
#endif

static const struct adc_dt_spec analog_input_adc_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

#ifdef CONFIG_ENABLE_DUST_SENSOR
static const struct gpio_dt_spec iled_gpio_spec =
    GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), iled_gpios);
#endif


int main(void) {
  printk("%s\n", APP_ASCII_BANNER);
  LOG_INF("===== Buzzverse Node System Booting (Zephyr Log) =====");

  BME280 bme280(DEVICE_DT_GET_ANY(bosch_bme280));
#ifdef CONFIG_ENABLE_ANALOG
  Analog analog(&analog_input_adc_spec);
#endif

#ifdef CONFIG_ENABLE_DUST_SENSOR
  DustSensor dust_sensor(&analog_input_adc_spec, &iled_gpio_spec);
#endif

  // Array of available sensors
  etl::array<etl::unique_ptr<Sensor>, NUMBER_OF_SENSORS> sensors {
	etl::unique_ptr<BME280>(etl::move(&bme280)),
#ifdef CONFIG_ENABLE_ANALOG
  etl::unique_ptr<Analog>(etl::move(&analog)),
#endif
#ifdef CONFIG_ENABLE_DUST_SENSOR
  etl::unique_ptr<DustSensor>(etl::move(&dust_sensor)),
#endif
  };

  BQ27441 bq27441(DEVICE_DT_GET_ANY(ti_bq274xx));
  LoRaWANHandler lorawan(bq27441);

  etl::unique_ptr<SleepManager> p_sleep_manager(nullptr);
#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  p_sleep_manager = etl::unique_ptr<SleepManager>(new SleepManager);
#endif

  Application app(sensors, lorawan, etl::move(p_sleep_manager));

  if (!app.init()) {
    LOG_ERR("Critical application initialization failed!");

    buzzverse_v1_Packet packet = buzzverse_v1_Packet_init_default;
    app.generate_init_failure_report(packet);

    if (lorawan.is_ready()) {
      LOG_ERR("Attempting to send failure status packet via LoRaWAN...");
      lorawan.send_packet(packet);
    }

    LOG_ERR("Rebooting device due to critical initialization failure.");
    sys_reboot(SYS_REBOOT_COLD);
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
