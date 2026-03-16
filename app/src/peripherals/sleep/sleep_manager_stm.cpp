#include "sleep_manager_stm.hpp"

LOG_MODULE_REGISTER(sleep_mgr_stm, LOG_LEVEL_INF);

extern struct k_sem wakeup_sem;

namespace {
  struct gpio_callback wkup_cb_data_1;
  struct gpio_callback wkup_cb_data_2;
  
  volatile uint32_t wkup_count = 0;
  volatile uint32_t last_wkup_time = 0;

  void wkup_isr(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
    uint32_t current_time = k_uptime_get_32();
    
    // Debouncing (250ms)
    if (current_time - last_wkup_time > 250) {
      last_wkup_time = current_time;
      wkup_count++;
      printk("Wakeup trigger! Total count: %u\n", wkup_count);
    }
  }
}

SleepManagerStm::SleepManagerStm()
    : initialized(false),
      sleep_timeout_ms(DEFAULT_SLEEP_DURATION_MS)
#ifdef CONFIG_SOC_STM32WL55XX
      ,
      wkup_gpio_1(GPIO_DT_SPEC_GET(DT_ALIAS(wkup_src_1), gpios)),
      wkup_gpio_2(GPIO_DT_SPEC_GET(DT_ALIAS(wkup_src_2), gpios)),
      rtc()
#endif
{
}

uint32_t SleepManagerStm::get_and_clear_wakeup_count() {
    uint32_t current_count = wkup_count;
    wkup_count = 0;
    return current_count;
}

Peripheral::Status SleepManagerStm::init() {
#ifdef CONFIG_SOC_STM32WL55XX

  if (!device_is_ready(wkup_gpio_1.port)) {
    LOG_ERR("wkup gpio 1 not ready");
  } else {
    if (gpio_pin_configure_dt(&wkup_gpio_1, GPIO_INPUT) != 0) {
      LOG_ERR("wkup gpio 1 cfg failed");
    } else {
      gpio_pin_interrupt_configure_dt(&wkup_gpio_1, GPIO_INT_EDGE_TO_ACTIVE);
      gpio_init_callback(&wkup_cb_data_1, wkup_isr, BIT(wkup_gpio_1.pin));
      gpio_add_callback(wkup_gpio_1.port, &wkup_cb_data_1);
      LOG_INF("Wakeup source 1 initialized.");
    }
  }

  if (!device_is_ready(wkup_gpio_2.port)) {
    LOG_ERR("wkup gpio 2 not ready");
  } else {
    if (gpio_pin_configure_dt(&wkup_gpio_2, GPIO_INPUT) != 0) {
      LOG_ERR("wkup gpio 2 cfg failed");
    } else {
      gpio_pin_interrupt_configure_dt(&wkup_gpio_2, GPIO_INT_EDGE_TO_ACTIVE);
      gpio_init_callback(&wkup_cb_data_2, wkup_isr, BIT(wkup_gpio_2.pin));
      gpio_add_callback(wkup_gpio_2.port, &wkup_cb_data_2);
      LOG_INF("Wakeup source 2 initialized.");
    }
  }

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
    k_sem_take(&wakeup_sem, K_FOREVER);
    return;
  }

  rtc_time now;
  if (!rtc.get_time(now)) {
    LOG_WRN("rtc read failed, sleep without alarm");
    k_sem_take(&wakeup_sem, K_FOREVER);
    return;
  }

  const int64_t epoch_now = rtc.to_epoch(now);
  const int64_t epoch_alarm = epoch_now + (sleep_timeout_ms / 1000);

  rtc_time alarm_time;
  if (!rtc.epoch_to_rtc_time(epoch_alarm, alarm_time)) {
    LOG_WRN("rtc convert failed, sleep without alarm");
    k_sem_take(&wakeup_sem, K_FOREVER);
    return;
  }

  if (!rtc.set_alarm(alarm_time)) {
    LOG_WRN("rtc alarm set failed, sleep without alarm");
    return;
  }

  k_sem_take(&wakeup_sem, K_FOREVER);

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
