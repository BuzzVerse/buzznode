#include "sleep-manager.hpp"

#include <zephyr/logging/log.h>
#include <zephyr/sys/poweroff.h>

#ifdef CONFIG_SOC_ESP32S3
  #include <driver/rtc_io.h>
#endif

LOG_MODULE_REGISTER(sleep_manager, LOG_LEVEL_DBG);

SleepManager::SleepManager() {}

Peripheral::Status SleepManager::init() {
  initialized = true;
  LOG_INF("SleepManager initialized.");
  return Status::OK;
}

bool SleepManager::is_ready() const {
  return initialized;
}

etl::string<SLEEP_MANAGER_NAME_SIZE> SleepManager::get_name() const {
  return "SleepManager";
}

void SleepManager::enter_sleep(SleepMode mode) {
  if (!initialized) {
    LOG_WRN("Sleep Manager not initialized, cannot sleep.");
    return;
  }

#ifdef CONFIG_SOC_SERIES_STM32
  LOG_WRN("STM32 Sleep not implemented yet.");
  timed_sleep();
#elif CONFIG_SOC_ESP32S3

  if (mode == SleepMode::LIGHT_SLEEP) {
    LOG_INF("Entering light sleep (using k_sleep)...");
    k_sleep(K_MSEC(DEFAULT_SLEEP_DURATION_MS));

  } else if (mode == SleepMode::DEEP_SLEEP) {
    LOG_INF("Configuring wake-up sources for TIMED deep sleep...");

    // --- Enable Timer Wakeup ---
    uint64_t sleep_time_us = (uint64_t)sleep_duration_ms * 1000ULL;
    LOG_INF("Enabling timer wakeup for %llu us (%d ms)", sleep_time_us, sleep_duration_ms);
    esp_err_t err = esp_sleep_enable_timer_wakeup(sleep_time_us);
    if (err != ESP_OK) {
      LOG_ERR("Failed to enable timer wakeup (err %d)", err);
      LOG_WRN("Falling back to k_sleep instead of deep sleep.");
      k_sleep(K_MSEC(sleep_duration_ms));
      return;
    }

    LOG_INF("Entering ESP32 deep sleep mode (timer wake)...");
    k_sleep(K_MSEC(50));  // Allow logs to propagate before deep sleep

    esp_deep_sleep_start();

    LOG_ERR("Should not reach here after deep sleep start!");
  }
#else
  LOG_WRN("Deep sleep not implemented for this SOC.");
  timed_sleep();
#endif
}

void SleepManager::timed_sleep() {
  if (!initialized) {
    LOG_WRN("Sleep Manager not initialized, cannot sleep.");
    return;
  }
  LOG_INF("Entering timed sleep (k_sleep) for %dms...", sleep_duration_ms);
  k_sleep(K_MSEC(sleep_duration_ms));
}

void SleepManager::set_sleep_duration(int duration_ms) {
  if (duration_ms > 0) {
    sleep_duration_ms = duration_ms;
    LOG_INF("Sleep duration set to %d ms", sleep_duration_ms);
  } else {
    LOG_WRN("Invalid sleep duration: %d ms. Keeping previous value: %d ms", duration_ms,
            sleep_duration_ms);
  }
}

#ifdef CONFIG_SOC_ESP32S3
esp_sleep_wakeup_cause_t SleepManager::get_wakeup_cause() {
  return esp_sleep_get_wakeup_cause();
}
#endif