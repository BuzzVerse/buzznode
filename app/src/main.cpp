#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

#include "Application.hpp"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "peripherals/sleep/sleep_manager.hpp"
#include "sensors/analog/analog.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"

LOG_MODULE_REGISTER(main_entry, LOG_LEVEL_DBG);

#ifdef CONFIG_SLEEP_TIME_MS
  #define APP_SLEEP_DURATION_MS CONFIG_SLEEP_TIME_MS
#else
  #define APP_SLEEP_DURATION_MS 10000
#endif

static const struct adc_dt_spec soil_sensor_adc_spec =
  ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

K_SEM_DEFINE(wakeup_sem, 0, 1);

int main(void) {
  LOG_INF("===== Buzzverse Node System Booting (Zephyr Log) =====");

  BME280 bme280(DEVICE_DT_GET_ANY(bosch_bme280));
  Analog analog(&soil_sensor_adc_spec);

  // Array of available sensors
  etl::array<etl::unique_ptr<Sensor>, NUMBER_OF_SENSORS> sensors{
    etl::unique_ptr<BME280>(etl::move(&bme280)),
#ifdef CONFIG_ENABLE_ANALOG
    etl::unique_ptr<Analog>(etl::move(&analog)),
#endif
  };

  BQ27441 bq27441(DEVICE_DT_GET_ANY(ti_bq274xx));
  LoRaWANHandler lorawan(bq27441);

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  SleepManager sleep_manager;
  Application app(sensors, lorawan, &sleep_manager);
#else
  Application app(sensors, lorawan, nullptr);
#endif

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

  while (true) {
    app.run_cycle();

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
    app.enter_low_power_mode(APP_SLEEP_DURATION_MS);
#else
    LOG_INF("Device sleep not enabled. Entering polling loop.");
    k_msleep(APP_SLEEP_DURATION_MS);
#endif
  }

  return 0;  // Should not be reached
}
