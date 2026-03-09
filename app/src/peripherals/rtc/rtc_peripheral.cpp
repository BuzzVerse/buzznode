#include "rtc_peripheral.hpp"

LOG_MODULE_REGISTER(rtc_periph, LOG_LEVEL_INF);

RtcPeripheral::RtcPeripheral() : rtc_dev(DEVICE_DT_GET(DT_NODELABEL(rtc))), initialized(false) {}

static void rtc_alarm_handler(const struct device* dev, uint16_t id, void* user_data) {
  printk("RTC ALARM FIRED! id=%u\n", id);
}

Peripheral::Status RtcPeripheral::init() {
  if (initialized) {
    return Peripheral::Status::ERROR_ALREADY_INITIALIZED;
  }

  if (!device_is_ready(rtc_dev)) {
    LOG_ERR("rtc not ready");
    return Peripheral::Status::NOT_READY;
  }

  if (!ensure_time_valid()) {
    LOG_WRN("rtc no valid time, init default");
    if (!set_default_time()) {
      LOG_ERR("rtc time set failed");
      return Peripheral::Status::ERROR_HW_CONFIG_FAILED;
    }
  }

  rtc_alarm_set_callback(rtc_dev, 0, rtc_alarm_handler, nullptr);

  initialized = true;
  return Peripheral::Status::OK;
}

bool RtcPeripheral::is_ready() const {
  return initialized && device_is_ready(rtc_dev);
}

etl::string<PERIPHERAL_NAME_SIZE> RtcPeripheral::get_name() const {
  return etl::string<PERIPHERAL_NAME_SIZE>("RtcPeripheral");
}

bool RtcPeripheral::get_time(rtc_time& out_time) const {
  return rtc_get_time(rtc_dev, &out_time) == 0;
}

bool RtcPeripheral::ensure_time_valid() {
  rtc_time tmp;
  return rtc_get_time(rtc_dev, &tmp) == 0;
}

bool RtcPeripheral::set_alarm(const rtc_time& t) {
  uint16_t supported = 0;
  if (rtc_alarm_get_supported_fields(rtc_dev, 0, &supported) != 0) {
    return false;
  }

  uint16_t mask = 0;
  if (supported & RTC_ALARM_TIME_MASK_SECOND) mask |= RTC_ALARM_TIME_MASK_SECOND;
  if (supported & RTC_ALARM_TIME_MASK_MINUTE) mask |= RTC_ALARM_TIME_MASK_MINUTE;
  if (supported & RTC_ALARM_TIME_MASK_HOUR) mask |= RTC_ALARM_TIME_MASK_HOUR;
  if (supported & RTC_ALARM_TIME_MASK_MONTHDAY) mask |= RTC_ALARM_TIME_MASK_MONTHDAY;
  if (supported & RTC_ALARM_TIME_MASK_MONTH) mask |= RTC_ALARM_TIME_MASK_MONTH;
  if (supported & RTC_ALARM_TIME_MASK_YEAR) mask |= RTC_ALARM_TIME_MASK_YEAR;

  (void)rtc_alarm_is_pending(rtc_dev, 0);

  return rtc_alarm_set_time(rtc_dev, 0, mask, &t) == 0;
}

int64_t RtcPeripheral::to_epoch(const rtc_time& t) const {
  struct rtc_time tmp = t;
  struct tm tm_now = *rtc_time_to_tm(&tmp);
  return timeutil_timegm64(&tm_now);
}

bool RtcPeripheral::epoch_to_rtc_time(int64_t epoch, rtc_time& out) const {
  time_t epoch32 = static_cast<time_t>(epoch);

  struct tm tm_alarm;
  if (gmtime_r(&epoch32, &tm_alarm) == nullptr) {
    return false;
  }

  out.tm_sec = tm_alarm.tm_sec;
  out.tm_min = tm_alarm.tm_min;
  out.tm_hour = tm_alarm.tm_hour;
  out.tm_mday = tm_alarm.tm_mday;
  out.tm_mon = tm_alarm.tm_mon;
  out.tm_year = tm_alarm.tm_year;
  out.tm_wday = tm_alarm.tm_wday;
  out.tm_yday = tm_alarm.tm_yday;
  out.tm_isdst = -1;

  return true;
}

bool RtcPeripheral::set_default_time() {
  rtc_time def{};
  def.tm_sec = 0;
  def.tm_min = 0;
  def.tm_hour = 0;
  def.tm_mday = 1;
  def.tm_mon = 0;
  def.tm_year = 2025 - 1900;
  def.tm_wday = 0;
  def.tm_yday = 0;
  def.tm_isdst = 0;

  return rtc_set_time(rtc_dev, &def) == 0;
}