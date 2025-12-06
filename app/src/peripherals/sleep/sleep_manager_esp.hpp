#ifndef SLEEP_MANAGER_ESP_HPP
#define SLEEP_MANAGER_ESP_HPP

#include "sleep_manager_base.hpp"

#ifdef CONFIG_SOC_ESP32S3
  #include <esp_err.h>
  #include <esp_sleep.h>
#endif

class SleepManagerEsp final : public SleepManagerBase {
 public:
  SleepManagerEsp() = default;

  Peripheral::Status init() override;
  bool is_ready() const override;
  etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const override;

  void enter_sleep(SleepMode mode) override;
  void set_sleep_duration(int duration_ms) override;
  void timed_sleep() override;

  WakeCause get_wakeup_cause() const override;

 private:
  bool initialized = false;
  int sleep_duration_ms = DEFAULT_SLEEP_DURATION_MS;
};

#endif