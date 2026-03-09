#ifndef SLEEP_MANAGER_HPP
#define SLEEP_MANAGER_HPP

#include <etl/memory.h>
#include <etl/string.h>

#include "peripheral.hpp"
#include "sleep_manager_base.hpp"

class SleepManager : public Peripheral {
 public:
  using SleepMode = SleepManagerBase::SleepMode;
  using WakeCause = SleepManagerBase::WakeCause;

  SleepManager();

  Peripheral::Status init() override;
  bool is_ready() const override;
  etl::string<SLEEP_MANAGER_NAME_SIZE> get_name() const override;

  void enter_sleep(SleepMode mode);
  void set_sleep_duration(int ms);
  void timed_sleep();

  WakeCause get_wakeup_cause() const;

 private:
  etl::unique_ptr<SleepManagerBase> impl;
};

#endif