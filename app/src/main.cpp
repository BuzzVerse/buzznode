#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

#include "Application.hpp"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "utils/banner.hpp"
#include "utils/sleep-manager.hpp"
#include "sensors/sen0308/sen0308.hpp"

LOG_MODULE_REGISTER(main_entry, LOG_LEVEL_DBG);

#ifdef CONFIG_SLEEP_TIME_MS
  #define APP_SLEEP_DURATION_MS CONFIG_SLEEP_TIME_MS
#else
  #define APP_SLEEP_DURATION_MS 10000
#endif

int main(void) {
  printk("%s\n", APP_ASCII_BANNER);
  LOG_INF("===== Buzzverse Node System Booting (Zephyr Log) =====");

  BME280 bme280(DEVICE_DT_GET_ANY(bosch_bme280));
  BQ27441 bq27441(DEVICE_DT_GET_ANY(ti_bq274xx));
  LoRaWANHandler lorawan(bq27441);

  SleepManager* p_sleep_manager = nullptr;

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  SleepManager sleep_manager_instance;
  p_sleep_manager = &sleep_manager_instance;
#endif

  Application app(bq27441, lorawan, p_sleep_manager);
  const device* adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
  SEN0308 soil_sensor(adc_dev, 8); // Channel 8 (GPIO8)
  soil_sensor.init();

  SoilMoistureData soil_data;
  if (soil_sensor.read_data(&soil_data) == Sensor<SoilMoistureData>::Status::OK) {
    // Use soil_data.percent, etc.
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