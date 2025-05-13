// app/src/main.cpp
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

#include "Application.hpp"
#include "utils/banner.hpp"

LOG_MODULE_REGISTER(main_entry, LOG_LEVEL_DBG);

int main(void) {
  printk("%s\n", APP_ASCII_BANNER);

  LOG_INF("===== Buzzverse Node System Booting (Zephyr Log) =====");

  Application app;

  if (!app.init()) {
    LOG_ERR("Application initialization failed. Halting.");
    while (true) {
      k_sleep(K_SECONDS(60));
    }
  }

  app.run_cycle();

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  app.enter_low_power_mode();
  LOG_WRN("Execution continued after enter_low_power_mode - this is unexpected for deep sleep.");
#else
  LOG_INF("Device sleep not enabled. Running in normal mode.");
  // Main loop for normal operation
  while (true) {
    k_sleep(K_SECONDS(5));
    app.run_cycle();
  }
#endif

  return 0;
}