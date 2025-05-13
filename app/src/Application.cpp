#include "Application.hpp"

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include "buzzverse/bme280.pb.h"
#include "buzzverse/packet.pb.h"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "peripherals/peripheral.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "sensors/sensor.hpp"
#include "utils/sleep-manager.hpp"

// Register a logging module for this application logic file.
LOG_MODULE_REGISTER(application, CONFIG_APP_LOG_LEVEL);

// Define global static instances of the primary hardware peripherals.
static BME280 g_bme280(DEVICE_DT_GET_ANY(bosch_bme280));
static BQ27441 g_bq27441(DEVICE_DT_GET_ANY(ti_bq274xx));

// LoRaWANHandler takes a reference to BQ27441 for its battery level callback.
static LoRaWANHandler g_lorawan(g_bq27441);
#ifdef CONFIG_ENABLE_DEVICE_SLEEP
static SleepManager g_sleep_manager;
#endif

Application::Application()
    : m_bme280(g_bme280),
      m_bq27441(g_bq27441),
      m_lorawan(g_lorawan)
#ifdef CONFIG_ENABLE_DEVICE_SLEEP
      ,
      m_sleep_manager(g_sleep_manager)
#endif
{
  // Constructor: Initializes member references to the global peripheral instances.
}

bool Application::init() {
  LOG_INF("Application core initializing...");

  if (!initialize_peripherals()) {
    LOG_ERR("Critical peripheral initialization failed.");
    return false;  // Indicate failure to main
  }

  // Log ESP32S3 wake-up cause if sleep is enabled and on the correct SoC.
#if defined(CONFIG_SOC_ESP32S3) && defined(CONFIG_ENABLE_DEVICE_SLEEP)
  if (m_sleep_manager.is_ready()) {
    esp_sleep_wakeup_cause_t cause = m_sleep_manager.get_wakeup_cause();

    LOG_INF("ESP32S3 Wakeup cause: %d", cause);
  } else {
    LOG_WRN("SleepManager not ready, cannot get wakeup cause.");
  }
#endif
  LOG_INF("Application core initialization complete.");
  return true;
}

bool Application::initialize_peripherals() {
  LOG_DBG("Initializing all application peripherals...");
  m_peripherals.clear();  // Ensure the vector is empty before populating

  // Populate the list of peripherals for batch initialization.
  m_peripherals.push_back(&m_bme280);
  m_peripherals.push_back(&m_bq27441);  // BQ27441 is needed for LoRaWAN battery cb
  m_peripherals.push_back(&m_lorawan);
#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  m_peripherals.push_back(&m_sleep_manager);
#endif

  bool all_essential_ready = true;
  for (Peripheral* peripheral_ptr : m_peripherals) {
    // This check should ideally not be necessary if m_peripherals is populated correctly.
    if (!peripheral_ptr) {
      LOG_ERR("Encountered a null peripheral pointer during initialization!");
      all_essential_ready = false;  // This is a critical internal logic error.
      continue;
    }

    LOG_DBG("Initializing: %s", peripheral_ptr->get_name().c_str());
    if (peripheral_ptr->init() != Peripheral::Status::OK) {
      LOG_ERR("%s initialization failed.", peripheral_ptr->get_name().c_str());
      // Determine if the application can function without this peripheral.
      // For this application, BME280 (primary sensor) and LoRaWAN are essential.
      if (peripheral_ptr == &m_bme280 || peripheral_ptr == &m_lorawan) {
        all_essential_ready = false;
      }
    } else {
      LOG_INF("%s initialized.", peripheral_ptr->get_name().c_str());
    }
  }
  return all_essential_ready;
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

  // BQ27441 data is read by LoRaWANHandler's battery_level_callback when needed by the stack.
  // However, we can read it here for local logging or other application logic if desired.
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

  // Determine if the BME280 data is valid enough to send.
  // TODO: This is a simple check; we may want to refine this based on something else
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
      // Battery level is reported separately by the LoRaWAN stack
    }
  } else {
    LOG_ERR("LoRaWAN handler not ready; cannot send packet.");
  }
}

void Application::run_cycle() {
  LOG_INF("--- Starting Application Cycle ---");

  buzzverse_v1_BME280Data bme280_data = buzzverse_v1_BME280Data_init_zero;

  read_sensor_data(bme280_data);  // Read BME280 for the main application packet.
  send_lora_packet(bme280_data);  // Send the BME280 data.

  LOG_INF("--- Application Cycle Complete ---");
}

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
void Application::enter_low_power_mode() {
  if (!m_sleep_manager.is_ready()) {
    LOG_ERR("SleepManager not ready. Defaulting to k_sleep for %d ms.", m_app_sleep_duration_ms);
    k_sleep(K_MSEC(m_app_sleep_duration_ms));
    // This loop is a fallback if SleepManager fails or deep sleep does not occur as expected.
    LOG_WRN("Reached code after k_sleep fallback. Looping indefinitely.");
    while (true) {
      k_sleep(K_SECONDS(m_app_sleep_duration_ms / 1000));
      LOG_INF("Fallback k_sleep loop iteration (SleepManager was not ready).");
    }
    return;  // Should not be reached if while(true) is active
  }

  LOG_INF("Preparing system for deep sleep (duration: %d ms)...", m_app_sleep_duration_ms);

  m_sleep_manager.set_sleep_duration(m_app_sleep_duration_ms);
  m_sleep_manager.enter_sleep(SleepManager::SleepMode::DEEP_SLEEP);

  // On ESP32, esp_deep_sleep_start() causes a reset. This log indicates an issue.
  LOG_ERR(
    "!!! CRITICAL: Execution continued after esp_deep_sleep_start() call. Deep sleep failed. !!!");
  // If deep sleep fails, enter a k_sleep loop to prevent high power consumption.
  LOG_WRN("Deep sleep did not take effect. Entering k_sleep fallback loop.");
  while (true) {
    k_sleep(K_SECONDS(m_app_sleep_duration_ms / 1000));
    LOG_INF("Fallback k_sleep loop iteration (deep sleep failed to engage).");
  }
}
#endif  // CONFIG_ENABLE_DEVICE_SLEEP