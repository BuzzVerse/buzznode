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

Application::Application(etl::array<Sensor*, NUMBER_OF_SENSORS> sensors, LoRaWANHandler& lorawan,
                         SleepManager* sleep_manager)
		: m_sensors(sensors), m_lorawan(lorawan), m_sleep_manager(sleep_manager) {}

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

	// Initialize sensors
	for(auto sensor: m_sensors) {
		LOG_DBG("Initializing: %s", sensor->get_name().c_str());
		if (sensor->init() != Peripheral::Status::OK) {
			LOG_ERR("%s initialization failed.", sensor->get_name().c_str());
		} else {
			LOG_INF("%s initialized.", sensor->get_name().c_str());
		}
	}

	// Initialize the LoRaWAN manager
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
  for(auto sensor: m_sensors) {
	  sensor->set_status(status_msg, buzzverse_v1_Status_ComponentState_NORMAL);
  }
  status_msg.lorawan_status = buzzverse_v1_Status_ComponentState_NORMAL;

	for(auto sensor: m_sensors) {
		if (!sensor->is_ready()) {
			LOG_WRN("%s failed initialization.", sensor->get_name().c_str());
			sensor->set_status(status_msg, buzzverse_v1_Status_ComponentState_INITIALIZATION_FAILED);
		}
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

	for(auto sensor: m_sensors) {
		if (sensor->is_ready()) {
			if (sensor->read_data(&bme_data) != Sensor::Status::OK) {
				LOG_ERR("Failed to read %s data.", sensor->get_name().c_str());
			}
		} else {
			LOG_ERR("%s not ready for reading.", sensor->get_name().c_str());
		}
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
