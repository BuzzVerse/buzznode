#ifndef SLEEP_MANAGER_BASE_HPP
#define SLEEP_MANAGER_BASE_HPP

#include <etl/string.h>

#include "peripheral.hpp"

constexpr size_t SLEEP_MANAGER_NAME_SIZE = 32;
constexpr int DEFAULT_SLEEP_DURATION_MS = 5000;

class SleepManagerBase : public Peripheral {
 public:
  enum class SleepMode { LIGHT_SLEEP, DEEP_SLEEP };

  enum class WakeCause { UNKNOWN = 0, RTC_ALARM, WAKEUP_PIN, TIMER, POWER_ON_RESET };

  virtual ~SleepManagerBase() = default;

  virtual Peripheral::Status init() = 0;
  virtual bool is_ready() const = 0;
  virtual etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const = 0;

  virtual void enter_sleep(SleepMode mode) = 0;
  virtual void set_sleep_duration(int duration_ms) = 0;
  virtual void timed_sleep() = 0;

  virtual WakeCause get_wakeup_cause() const {
    return WakeCause::UNKNOWN;
  }
};

#endif