#include "sleep_manager.hpp"

#if defined(CONFIG_SOC_STM32WL55XX)
  #include "sleep_manager_stm.hpp"
#elif defined(CONFIG_SOC_ESP32S3)
  #include "sleep_manager_esp.hpp"
#endif

SleepManager::SleepManager() {
#if defined(CONFIG_SOC_STM32WL55XX)
  impl = etl::unique_ptr<SleepManagerBase>(new SleepManagerStm());
#elif defined(CONFIG_SOC_ESP32S3)
  impl = etl::unique_ptr<SleepManagerBase>(new SleepManagerEsp());
#else
  impl = nullptr;
#endif
}

Peripheral::Status SleepManager::init() {
  if (!impl) {
    return Peripheral::Status::INIT_ERR;
  }
  return impl->init();
}

bool SleepManager::is_ready() const {
  return impl && impl->is_ready();
}

etl::string<SLEEP_MANAGER_NAME_SIZE> SleepManager::get_name() const {
  return impl ? impl->get_name() : etl::string<SLEEP_MANAGER_NAME_SIZE>("SleepManagerNone");
}

void SleepManager::enter_sleep(SleepMode mode) {
  if (impl) {
    impl->enter_sleep(mode);
  }
}

void SleepManager::set_sleep_duration(int ms) {
  if (impl) {
    impl->set_sleep_duration(ms);
  }
}

void SleepManager::timed_sleep() {
  if (impl) {
    impl->timed_sleep();
  }
}

SleepManager::WakeCause SleepManager::get_wakeup_cause() const {
  if (!impl) {
    return SleepManagerBase::WakeCause::UNKNOWN;
  }
  return impl->get_wakeup_cause();
}