#include "sleep_manager_stm.hpp"

#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(sleep_mgr_stm, LOG_LEVEL_INF);

/* Use the real global semaphore defined elsewhere */
extern struct k_sem wakeup_sem;

namespace {

/*
 * ISR/thread shared state:
 * - Use atomics (volatile is NOT sufficient for concurrency).
 * - Keep ISR extremely small: no LOG_* calls here.
 */
static atomic_t wkup_count;
static atomic_t last_wkup_time;

static void wkup_isr(const struct device* dev, struct gpio_callback* cb, uint32_t pins) {
  ARG_UNUSED(dev);
  ARG_UNUSED(cb);
  ARG_UNUSED(pins);

  const uint32_t now = k_uptime_get_32();
  const uint32_t last = (uint32_t)atomic_get(&last_wkup_time);

  if ((uint32_t)(now - last) > CONFIG_GPIO_WAKEUP_DEBOUNCE_MS) {
    atomic_set(&last_wkup_time, now);
    atomic_inc(&wkup_count);
  }
}

}  // namespace

#define WAKEUP_PIN_CFG(node_id, prop, idx) GPIO_DT_SPEC_GET_BY_IDX(node_id, prop, idx),

SleepManagerStm::SleepManagerStm()
    : initialized(false), sleep_timeout_ms(DEFAULT_SLEEP_DURATION_MS) {
#if defined(CONFIG_SOC_SERIES_STM32WLX)
  #if DT_NODE_HAS_PROP(DT_PATH(zephyr_user), wakeup_gpios)
  static const struct gpio_dt_spec specs[] = {
    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), wakeup_gpios, WAKEUP_PIN_CFG)};

  for (size_t i = 0; i < ARRAY_SIZE(specs); i++) {
    if (!wakeup_pins.full()) {
      WakeupPin pin;
      pin.spec = specs[i];
      wakeup_pins.push_back(pin);
    } else {
      LOG_WRN("Max wakeup pins reached.");
      break;
    }
  }
  #else
  LOG_WRN("No wakeup-gpios defined in zephyr,user node.");
  #endif
#endif
}

Peripheral::Status SleepManagerStm::init() {
#if defined(CONFIG_SOC_SERIES_STM32WLX)
  if (wakeup_pins.empty()) {
    LOG_INF("No wakeup pins to configure.");
  }

  for (auto& pin : wakeup_pins) {
    if (!device_is_ready(pin.spec.port)) {
      LOG_ERR("GPIO port %s not ready", pin.spec.port->name);
      continue;
    }

    if (gpio_pin_configure_dt(&pin.spec, GPIO_INPUT) != 0) {
      LOG_ERR("Pin %d cfg failed", pin.spec.pin);
      continue;
    }

    (void)gpio_pin_interrupt_configure_dt(&pin.spec, GPIO_INT_EDGE_TO_ACTIVE);

    gpio_init_callback(&pin.cb_data, wkup_isr, BIT(pin.spec.pin));
    (void)gpio_add_callback(pin.spec.port, &pin.cb_data);

    LOG_INF("Wakeup source initialized: Pin %d on %s", pin.spec.pin, pin.spec.port->name);
  }

  const Peripheral::Status rtc_rc = rtc.init();
  if (rtc_rc != Peripheral::Status::OK && rtc_rc != Peripheral::Status::ERROR_ALREADY_INITIALIZED) {
    return rtc_rc;
  }
#endif

  initialized = true;
  return Peripheral::Status::OK;
}

uint32_t SleepManagerStm::get_and_clear_wakeup_count() {
  /* atomically get-and-clear */
  return (uint32_t)atomic_set(&wkup_count, 0);
}

bool SleepManagerStm::is_ready() const {
#if defined(CONFIG_SOC_SERIES_STM32WLX)
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
    return;
  }

#if defined(CONFIG_SOC_SERIES_STM32WLX)
  if (!rtc.is_ready() || !rtc.ensure_time_valid()) {
    /*
     * Wait until something wakes us (wkup_isr gives wakeup_sem).
     * This is thread context; safe to block here.
     */
    (void)k_sem_take(&wakeup_sem, K_FOREVER);
    return;
  }

  rtc_time now;
  if (rtc.get_time(now)) {
    int64_t epoch_alarm = rtc.to_epoch(now) + (sleep_timeout_ms / 1000);
    rtc_time alarm_time;
    if (rtc.epoch_to_rtc_time(epoch_alarm, alarm_time)) {
      rtc.set_alarm(alarm_time);
    }
  }

  (void)k_sem_take(&wakeup_sem, K_FOREVER);
#else
  k_msleep(sleep_timeout_ms);
#endif
}

void SleepManagerStm::set_sleep_duration(int duration_ms) {
  if (duration_ms > 0) {
    sleep_timeout_ms = duration_ms;
  }
}

void SleepManagerStm::timed_sleep() {
  if (initialized) {
    k_msleep(sleep_timeout_ms);
  }
}