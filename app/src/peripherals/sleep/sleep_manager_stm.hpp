#ifndef SLEEP_MANAGER_STM_HPP
#define SLEEP_MANAGER_STM_HPP

#include <zephyr/device.h>
#include <zephyr/dt-bindings/gpio/stm32-gpio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/pm/pm.h>

#include "peripherals/rtc/rtc_peripheral.hpp"
#include "sleep_manager_base.hpp"

class SleepManagerStm : public SleepManagerBase {
 public:
  SleepManagerStm();

  Peripheral::Status init() override;
  bool is_ready() const override;
  etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const override;

  void enter_sleep(SleepMode mode) override;
  void set_sleep_duration(int duration_ms) override;
  void timed_sleep() override;

 private:
  bool initialized;
  int sleep_timeout_ms;

#ifdef CONFIG_SOC_STM32WL55XX
  gpio_dt_spec wkup_gpio;
  RtcPeripheral rtc;
#endif
};

#endif