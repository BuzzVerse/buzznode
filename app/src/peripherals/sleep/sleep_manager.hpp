#ifndef SLEEP_MANAGER_HPP
#define SLEEP_MANAGER_HPP

#include <etl/string.h>

#include "peripheral.hpp"
#include "sleep_manager_base.hpp"

#if defined(CONFIG_SOC_SERIES_STM32WLX)
  #include "sleep_manager_stm.hpp"
typedef SleepManagerStm SleepManagerImpl;
#elif defined(CONFIG_SOC_ESP32S3)
  #include "sleep_manager_esp.hpp"
typedef SleepManagerEsp SleepManagerImpl;
#else
class SleepManagerDummy : public SleepManagerBase {
 public:
  Peripheral::Status init() override {
    return Peripheral::Status::OK;
  }
  bool is_ready() const override {
    return true;
  }
  etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const override {
    return "Dummy";
  }
  void enter_sleep(SleepMode mode) override {}
  void set_sleep_duration(int duration_ms) override {}
  void timed_sleep() override {}
};
typedef SleepManagerDummy SleepManagerImpl;
#endif

class SleepManager : public Peripheral {
 public:
  using SleepMode = SleepManagerBase::SleepMode;
  using WakeCause = SleepManagerBase::WakeCause;

  SleepManager() = default;

  Peripheral::Status init() override {
    return impl.init();
  }
  bool is_ready() const override {
    return impl.is_ready();
  }
  etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const override {
    return impl.get_name();
  }

  uint32_t get_and_clear_wakeup_count() {
    return impl.get_and_clear_wakeup_count();
  }

  void enter_sleep(SleepMode mode) {
    impl.enter_sleep(mode);
  }
  void set_sleep_duration(int ms) {
    impl.set_sleep_duration(ms);
  }
  void timed_sleep() {
    impl.timed_sleep();
  }

  WakeCause get_wakeup_cause() const {
    return impl.get_wakeup_cause();
  }

 private:
  SleepManagerImpl impl;
};

#endif
