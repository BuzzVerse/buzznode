#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/policy.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

#include "Application.hpp"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "peripherals/sleep/sleep_manager.hpp"
#include "sensors/analog/analog.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "utils/banner.hpp"

LOG_MODULE_REGISTER(main_entry, LOG_LEVEL_DBG);

#ifdef CONFIG_SLEEP_TIME_MS
  #define APP_SLEEP_DURATION_MS CONFIG_SLEEP_TIME_MS
#else
  #define APP_SLEEP_DURATION_MS 10000
#endif

static const struct adc_dt_spec soil_sensor_adc_spec =
  ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

K_SEM_DEFINE(wakeup_sem, 0, 1);

static const struct device* i2c_dev;
static const struct device* adc_dev;
static const struct device* spi1_dev;
static const struct device* subghzspi_dev;

static void suspend_peripherals(void) {
  pm_device_action_run(i2c_dev, PM_DEVICE_ACTION_SUSPEND);
  pm_device_action_run(adc_dev, PM_DEVICE_ACTION_SUSPEND);
  pm_device_action_run(spi1_dev, PM_DEVICE_ACTION_SUSPEND);
  pm_device_action_run(subghzspi_dev, PM_DEVICE_ACTION_SUSPEND);
}

static void resume_peripherals(void) {
  pm_device_action_run(subghzspi_dev, PM_DEVICE_ACTION_RESUME);
  pm_device_action_run(spi1_dev, PM_DEVICE_ACTION_RESUME);
  pm_device_action_run(adc_dev, PM_DEVICE_ACTION_RESUME);
  pm_device_action_run(i2c_dev, PM_DEVICE_ACTION_RESUME);
}

int main(void) {
  // Lock all PM states during init
  pm_policy_state_all_lock_get();

  printk("%s\n", APP_ASCII_BANNER);
  LOG_INF("===== Buzzverse Node System Booting (Zephyr Log) =====");

  // Get device references for PM control
  i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c2));
  adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc1));
  spi1_dev = DEVICE_DT_GET(DT_NODELABEL(spi1));
  subghzspi_dev = DEVICE_DT_GET(DT_NODELABEL(subghzspi));

  BME280 bme280(DEVICE_DT_GET_ANY(bosch_bme280));
  Analog analog(&soil_sensor_adc_spec);

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

  // Release PM lock after init
  pm_policy_state_all_lock_put();

  while (true) {
    // Resume all peripherals before active cycle
    resume_peripherals();

    app.run_cycle();

    // Suspend all peripherals before sleep
    suspend_peripherals();

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
    app.enter_low_power_mode(APP_SLEEP_DURATION_MS);
#else
    LOG_INF("Device sleep not enabled. Entering polling loop.");
    k_msleep(APP_SLEEP_DURATION_MS);
#endif
  }

  return 0;
}