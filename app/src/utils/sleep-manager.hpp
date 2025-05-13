#ifndef SLEEPMANAGER_HPP
#define SLEEPMANAGER_HPP

#include <etl/string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/pm.h>

#ifdef CONFIG_SOC_ESP32S3
  #include <esp_sleep.h>
#endif

#include "Peripheral.hpp"

constexpr size_t SLEEP_MANAGER_NAME_SIZE = 32;
constexpr int DEFAULT_SLEEP_DURATION_MS = 5000;

class SleepManager : public Peripheral {
 public:
  enum class SleepMode { LIGHT_SLEEP, DEEP_SLEEP };

  SleepManager();
  ~SleepManager() override = default;

  Status init() override;
  bool is_ready() const override;
  etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const override;

  void enter_sleep(SleepMode mode);
  void set_sleep_duration(int duration_ms);
  void timed_sleep();

#ifdef CONFIG_SOC_ESP32S3
  esp_sleep_wakeup_cause_t get_wakeup_cause();
#endif

 private:
  // --- Member Variables ---
  bool initialized = false;
  int sleep_duration_ms = DEFAULT_SLEEP_DURATION_MS;
};

#endif  // SLEEPMANAGER_HPP