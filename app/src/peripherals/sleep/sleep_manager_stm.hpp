#ifndef SLEEP_MANAGER_STM_HPP
#define SLEEP_MANAGER_STM_HPP

#include <etl/vector.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "peripherals/rtc/rtc_peripheral.hpp"
#include "sleep_manager_base.hpp"

struct WakeupPin {
  struct gpio_dt_spec spec;
  struct gpio_callback cb_data;
};

class SleepManagerStm : public SleepManagerBase {
 public:
  SleepManagerStm();

  Peripheral::Status init() override;
  bool is_ready() const override;
  etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const override;

  uint32_t get_and_clear_wakeup_count() override;

  void enter_sleep(SleepMode mode) override;
  void set_sleep_duration(int duration_ms) override;
  void timed_sleep() override;

 private:
  static constexpr size_t MAX_WAKEUP_PINS = 4;
  etl::vector<WakeupPin, MAX_WAKEUP_PINS> wakeup_pins;

  bool initialized;
  int sleep_timeout_ms;

#if defined(CONFIG_SOC_SERIES_STM32WLX)
  RtcPeripheral rtc;
#endif
};

#endif
