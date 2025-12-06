#ifndef RTC_PERIPHERAL_HPP
#define RTC_PERIPHERAL_HPP

#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/timeutil.h>

#include "peripheral.hpp"

class RtcPeripheral : public Peripheral {
 public:
  RtcPeripheral();

  Status init() override;
  bool is_ready() const override;
  etl::string<PERIPHERAL_NAME_SIZE> get_name() const override;

  bool get_time(rtc_time& out_time) const;
  bool ensure_time_valid();
  bool set_alarm(const rtc_time& t);
  int64_t to_epoch(const rtc_time& t) const;
  bool epoch_to_rtc_time(int64_t epoch, rtc_time& out) const;

 private:
  bool set_default_time();

  const device* rtc_dev;
  bool initialized;
};

#endif