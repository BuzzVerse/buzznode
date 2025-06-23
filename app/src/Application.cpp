#include "Application.hpp"

#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

#include "buzzverse/bme280.pb.h"
#include "buzzverse/bq27441.pb.h"
#include "buzzverse/packet.pb.h"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "utils/sleep-manager.hpp"

LOG_MODULE_REGISTER(application, CONFIG_APP_LOG_LEVEL);

Application::Application(BME280& bme280, BQ27441& bq27441, LoRaWANHandler& lorawan,
                         SleepManager* sleep_manager)
    : m_bme280(bme280), m_bq27441(bq27441), m_lorawan(lorawan), m_sleep_manager(sleep_manager) {
  // Constructor: Initializes members with injected dependencies.
}

bool Application::init() {
  LOG_INF("Application core initializing...");

  if (!initialize_peripherals()) {
    LOG_ERR("Critical peripheral initialization failed.");
    return false;
  }

#if defined(CONFIG_SOC_ESP32S3)
  if (m_sleep_manager && m_sleep_manager->is_ready()) {
    esp_sleep_wakeup_cause_t cause = m_sleep_manager->get_wakeup_cause();
    LOG_INF("ESP32S3 Wakeup cause: %d", cause);
  }
#endif
  LOG_INF("Application core initialization complete.");
  return true;
}

bool Application::initialize_peripherals() {
  LOG_DBG("Initializing all application peripherals...");
  bool all_essential_ready = true;

  // --- Initialize each peripheral directly ---
  LOG_DBG("Initializing: %s", m_bme280.get_name().c_str());
  if (m_bme280.init() != Peripheral::Status::OK) {
    LOG_ERR("%s initialization failed.", m_bme280.get_name().c_str());
    all_essential_ready = false;  // BME280 is essential
  } else {
    LOG_INF("%s initialized.", m_bme280.get_name().c_str());
  }

  LOG_DBG("Initializing: %s", m_bq27441.get_name().c_str());
  if (m_bq27441.init() != Peripheral::Status::OK) {
    LOG_ERR("%s initialization failed.", m_bq27441.get_name().c_str());
    // BQ27441 is not considered essential for the main application logic to run
  } else {
    LOG_INF("%s initialized.", m_bq27441.get_name().c_str());
  }

  LOG_DBG("Initializing: %s", m_lorawan.get_name().c_str());
  if (m_lorawan.init() != Peripheral::Status::OK) {
    LOG_ERR("%s initialization failed.", m_lorawan.get_name().c_str());
    all_essential_ready = false;  // LoRaWAN is essential
  } else {
    LOG_INF("%s initialized.", m_lorawan.get_name().c_str());
  }

  // Initialize SleepManager only if it was provided
  if (m_sleep_manager) {
    LOG_DBG("Initializing: %s", m_sleep_manager->get_name().c_str());
    if (m_sleep_manager->init() != Peripheral::Status::OK) {
      LOG_ERR("%s initialization failed.", m_sleep_manager->get_name().c_str());
      // Not essential, will fallback to k_sleep
    } else {
      LOG_INF("%s initialized.", m_sleep_manager->get_name().c_str());
    }
  }

  return all_essential_ready;
}

void Application::generate_init_failure_report(buzzverse_v1_Packet& packet) {
  LOG_INF("Generating initialization failure report...");

  packet.which_data = buzzverse_v1_Packet_status_tag;
  auto& status_msg = packet.data.status;

  // Assume OK by default, then mark failures.
  status_msg.bme280_status = buzzverse_v1_Status_ComponentState_NORMAL;
  status_msg.bq27441_status = buzzverse_v1_Status_ComponentState_NORMAL;
  status_msg.lorawan_status = buzzverse_v1_Status_ComponentState_NORMAL;

  // Check the readiness of each peripheral managed by the Application class
  if (!m_bme280.is_ready()) {
    LOG_WRN("BME280 failed initialization.");
    status_msg.bme280_status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
  }

  if (!m_bq27441.is_ready()) {
    LOG_WRN("BQ27441 failed initialization.");
    status_msg.bq27441_status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
  }

  if (!m_lorawan.is_ready()) {
    LOG_WRN("LoRaWAN handler failed initialization.");
    status_msg.lorawan_status = buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED;
  }
}

void Application::run_cycle() {
  LOG_INF("--- Starting Application Cycle ---");

  buzzverse_v1_BME280Data bme280_data = buzzverse_v1_BME280Data_init_zero;

  read_sensor_data(bme280_data);
  send_lora_packet(bme280_data);

  LOG_INF("--- Application Cycle Complete ---");
}

void Application::enter_low_power_mode(int sleep_duration_ms) {
  // Check if SleepManager is available and ready
  if (!m_sleep_manager || !m_sleep_manager->is_ready()) {
    LOG_WRN("SleepManager not available/ready. Defaulting to k_sleep for %d ms.",
            sleep_duration_ms);
    k_sleep(K_MSEC(sleep_duration_ms));
    sys_reboot(SYS_REBOOT_COLD);
  }

  LOG_INF("Preparing system for deep sleep (duration: %d ms)...", sleep_duration_ms);

  m_sleep_manager->set_sleep_duration(sleep_duration_ms);
  m_sleep_manager->enter_sleep(SleepManager::SleepMode::DEEP_SLEEP);

  // This code should not be reached on platforms where deep sleep causes a reset.
  LOG_ERR("!!! CRITICAL: Execution continued after deep sleep call. Deep sleep failed. !!!");
  sys_reboot(SYS_REBOOT_COLD);
}
void Application::read_sensor_data(buzzverse_v1_BME280Data& bme_data) {
  LOG_INF("Reading sensor data...");
  bool bme_read_ok = false;

  if (m_bme280.is_ready()) {
    if (m_bme280.read_data(&bme_data) == Sensor<buzzverse_v1_BME280Data>::Status::OK) {
      LOG_INF("BME280: Temp: %d, Press: %u, Hum: %u", bme_data.temperature, bme_data.pressure,
              bme_data.humidity);
      bme_read_ok = true;
    } else {
      LOG_ERR("Failed to read BME280 data.");
    }
  } else {
    LOG_ERR("BME280 not ready for reading.");
  }

  if (m_bq27441.is_ready()) {
    buzzverse_v1_BQ27441Data local_bq_data = buzzverse_v1_BQ27441Data_init_zero;
    if (m_bq27441.read_data(&local_bq_data) == Sensor<buzzverse_v1_BQ27441Data>::Status::OK) {
      LOG_INF("BQ27441 (local log): V(mV):%u, I(mA):%d, SoC:%u%%", local_bq_data.voltage_mv,
              local_bq_data.current_ma, local_bq_data.state_of_charge);
    } else {
      LOG_WRN("Failed to read BQ27441 data for local logging.");
    }
  } else {
    LOG_WRN("BQ27441 not ready for local logging.");
  }

  if (!bme_read_ok) {
    LOG_WRN("Primary sensor data (BME280) could not be read for the application packet.");
  }
}

void Application::send_lora_packet(const buzzverse_v1_BME280Data& bme_data) {
  LOG_INF("Preparing LoRaWAN application packet...");

  bool bme_has_valid_data =
    !(bme_data.temperature == 0 && bme_data.pressure == 0 && bme_data.humidity == 0);

  if (!bme_has_valid_data) {
    LOG_WRN("No valid BME280 data to construct an application packet.");
    return;
  }

  buzzverse_v1_Packet packet = buzzverse_v1_Packet_init_default;
  packet.which_data = buzzverse_v1_Packet_bme280_tag;
  packet.data.bme280 = bme_data;
  LOG_DBG("Application packet constructed with BME280 data.");

  if (m_lorawan.is_ready()) {
    LOG_INF("Attempting to send application packet via LoRaWAN...");
    if (m_lorawan.send_packet(packet) != LoRaWANHandler::Status::OK) {
      LOG_ERR("Failed to send LoRaWAN application packet.");
    } else {
      LOG_INF("LoRaWAN application packet successfully sent or queued.");
    }
  } else {
    LOG_ERR("LoRaWAN handler not ready; cannot send packet.");
  }
}