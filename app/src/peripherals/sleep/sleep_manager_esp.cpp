#include "sleep_manager_esp.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_SOC_ESP32S3
  #include <esp_err.h>
  #include <esp_sleep.h>
#endif

LOG_MODULE_REGISTER(sleep_mgr_esp, LOG_LEVEL_INF);

Peripheral::Status SleepManagerEsp::init() {
  initialized = true;
  return Peripheral::Status::OK;
}

bool SleepManagerEsp::is_ready() const {
  return initialized;
}

etl::string<SLEEP_MANAGER_NAME_SIZE> SleepManagerEsp::get_name() const {
  return etl::string<SLEEP_MANAGER_NAME_SIZE>("SleepManagerEsp");
}

void SleepManagerEsp::enter_sleep(SleepMode mode) {
  if (!initialized) {
    LOG_ERR("sleep_mgr not initialized");
    return;
  }

#ifdef CONFIG_SOC_ESP32S3
  if (mode == SleepMode::LIGHT_SLEEP) {
    k_msleep(sleep_duration_ms);
    return;
  }

  const uint64_t us = static_cast<uint64_t>(sleep_duration_ms) * 1000ULL;
  esp_err_t rc = esp_sleep_enable_timer_wakeup(us);
  if (rc != ESP_OK) {
    LOG_WRN("timer_wakeup setup failed (%d), fallback msleep", (int)rc);
    k_msleep(sleep_duration_ms);
    return;
  }

  k_msleep(50);
  esp_deep_sleep_start();
#else
  k_msleep(sleep_duration_ms);
#endif
}

void SleepManagerEsp::timed_sleep() {
  if (!initialized) {
    LOG_ERR("sleep_mgr not initialized");
    return;
  }
  k_msleep(sleep_duration_ms);
}

void SleepManagerEsp::set_sleep_duration(int duration_ms) {
  if (duration_ms <= 0) {
    LOG_WRN("invalid sleep timeout %d", duration_ms);
    return;
  }
  sleep_duration_ms = duration_ms;
}

SleepManagerBase::WakeCause SleepManagerEsp::get_wakeup_cause() const {
  if (!initialized) {
    return WakeCause::UNKNOWN;
  }

#ifdef CONFIG_SOC_ESP32S3
  esp_sleep_source_t src = esp_sleep_get_wakeup_cause();
  switch (src) {
    case ESP_SLEEP_WAKEUP_TIMER:
      return WakeCause::TIMER;

    case ESP_SLEEP_WAKEUP_GPIO:
      return WakeCause::WAKEUP_PIN;

    case ESP_SLEEP_WAKEUP_UNDEFINED:
      return WakeCause::POWER_ON_RESET;

    default:
      return WakeCause::UNKNOWN;
  }
#else
  return WakeCause::UNKNOWN;
#endif
}