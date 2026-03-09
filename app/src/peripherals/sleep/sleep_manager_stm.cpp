#include "sleep_manager_stm.hpp"

LOG_MODULE_REGISTER(sleep_mgr_stm, LOG_LEVEL_INF);

SleepManagerStm::SleepManagerStm()
    : initialized(false),
      sleep_timeout_ms(DEFAULT_SLEEP_DURATION_MS)
#ifdef CONFIG_SOC_STM32WL55XX
      ,
      wkup_gpio(GPIO_DT_SPEC_GET(DT_ALIAS(wkup_src), gpios)),
      rtc()
#endif
{
}

static struct gpio_callback btn_cb_data;

int count = 0;

void button_isr(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
  count++;
  printk("Button pressed! Total count: %d\n", count);
}

Peripheral::Status SleepManagerStm::init() {
#ifdef CONFIG_SOC_STM32WL55XX
  if (!device_is_ready(wkup_gpio.port)) {
    LOG_ERR("wkup gpio not ready");
    return Peripheral::Status::NOT_READY;
  }

  const int pin_rc = gpio_pin_configure_dt(&wkup_gpio, GPIO_INPUT);
  if (pin_rc != 0) {
    LOG_ERR("wkup gpio cfg failed (%d)", pin_rc);
    return Peripheral::Status::ERROR_HW_CONFIG_FAILED;
  }

  gpio_pin_interrupt_configure_dt(&wkup_gpio, GPIO_INT_EDGE_TO_ACTIVE);

  gpio_init_callback(&btn_cb_data, button_isr, BIT(wkup_gpio.pin));
  gpio_add_callback(wkup_gpio.port, &btn_cb_data);

  const Peripheral::Status rtc_rc = rtc.init();
  if (rtc_rc != Peripheral::Status::OK && rtc_rc != Peripheral::Status::ERROR_ALREADY_INITIALIZED) {
    LOG_ERR("rtc init failed (%d)", static_cast<int>(rtc_rc));
    return rtc_rc;
  }

  if (!rtc.is_ready()) {
    LOG_ERR("rtc not ready after init");
    return Peripheral::Status::NOT_READY;
  }

#endif

  initialized = true;
  return Peripheral::Status::OK;
}

bool SleepManagerStm::is_ready() const {
#ifdef CONFIG_SOC_STM32WL55XX
  return initialized && rtc.is_ready();
#else
  return initialized;
#endif
}

etl::string<SLEEP_MANAGER_NAME_SIZE> SleepManagerStm::get_name() const {
  return etl::string<SLEEP_MANAGER_NAME_SIZE>("SleepManagerStm");
}

void SleepManagerStm::enter_sleep(SleepMode mode) {
  ARG_UNUSED(mode);

  if (!initialized) {
    LOG_ERR("sleep_mgr not initialized");
    return;
  }

#ifdef CONFIG_SOC_STM32WL55XX
  if (!rtc.is_ready()) {
    LOG_ERR("rtc unavailable");
    k_msleep(sleep_timeout_ms);
    return;
  }

  if (!rtc.ensure_time_valid()) {
    LOG_WRN("rtc time invalid, sleep without alarm");
    sys_poweroff();
    return;
  }

  rtc_time now;
  if (!rtc.get_time(now)) {
    LOG_WRN("rtc read failed, sleep without alarm");
    sys_poweroff();
    return;
  }

  const int64_t epoch_now = rtc.to_epoch(now);
  const int64_t epoch_alarm = epoch_now + (sleep_timeout_ms / 1000);

  rtc_time alarm_time;
  if (!rtc.epoch_to_rtc_time(epoch_alarm, alarm_time)) {
    LOG_WRN("rtc convert failed, sleep without alarm");
    sys_poweroff();
    return;
  }

  if (!rtc.set_alarm(alarm_time)) {
    LOG_WRN("rtc alarm set failed, sleep without alarm");
    return;
  }

  //sys_poweroff();
  k_sleep(K_FOREVER);

#else
  k_msleep(sleep_timeout_ms);
#endif
}

void SleepManagerStm::set_sleep_duration(int duration_ms) {
  if (duration_ms <= 0) {
    LOG_WRN("invalid sleep timeout %d", duration_ms);
    return;
  }
  sleep_timeout_ms = duration_ms;
}

void SleepManagerStm::timed_sleep() {
  if (!initialized) {
    LOG_ERR("sleep_mgr not initialized");
    return;
  }

  k_msleep(sleep_timeout_ms);
}
