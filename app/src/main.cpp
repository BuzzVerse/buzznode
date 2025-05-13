#include <etl/vector.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "buzzverse/bme280.pb.h"
#include "buzzverse/packet.pb.h"
#include "peripherals/lorawan_handler/lorawan_handler.hpp"
#include "sensors/bme280/bme280.hpp"
#include "sensors/bq27441/bq27441.hpp"
#include "utils/sleep-manager.hpp"

LOG_MODULE_REGISTER(main_app, LOG_LEVEL_DBG);

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  #define APP_SLEEP_DURATION_MS CONFIG_SLEEP_TIME_MS
#endif

BME280 bme280(DEVICE_DT_GET_ANY(bosch_bme280));
BQ27441 bq27441(DEVICE_DT_GET_ANY(ti_bq274xx));
LoRaWANHandler lorawan(bq27441);

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
SleepManager sleep_manager;
#endif

constexpr uint8_t PERIPHERAL_COUNT = 4;
etl::vector<Peripheral*, PERIPHERAL_COUNT> peripherals;

void initialize_peripherals() {
  LOG_INF("Initializing peripherals...");
  peripherals.push_back(&bme280);
  peripherals.push_back(&bq27441);
  peripherals.push_back(&lorawan);
#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  peripherals.push_back(&sleep_manager);
#endif

  bool all_essential_ready = true;
  for (auto* peripheral : peripherals) {
    if (!peripheral) {
      LOG_ERR("Null peripheral pointer!");
      all_essential_ready = false;
      continue;
    }
    LOG_DBG("Initializing %s...", peripheral->get_name().c_str());
    if (peripheral->init() != Peripheral::Status::OK) {
      LOG_ERR("%s initialization failed", peripheral->get_name().c_str());

      // Consider BME280 and LoRaWAN essential for core functionality
      if (peripheral == &bme280 || peripheral == &lorawan) {
        all_essential_ready = false;
      }
    } else {
      LOG_INF("%s initialized successfully.", peripheral->get_name().c_str());
    }
  }

  // TODO: Uncomment this block if you want to halt the system on essential peripheral failure
  // if (!all_essential_ready) {
  //   LOG_ERR("Essential peripheral initialization failed. Halting.");
  //   while (true) {
  //     k_sleep(K_SECONDS(10));
  //   }
  // }
}

void read_sensor_data(buzzverse_v1_BME280Data* bme_data, buzzverse_v1_BQ27441Data* bq_data) {
  LOG_INF("Reading sensor data...");
  bool bme_read_ok = false;
  bool bq_read_ok = false;

  if (bme280.is_ready()) {
    if (bme280.read_data(bme_data) == Sensor<buzzverse_v1_BME280Data>::Status::OK) {
      LOG_INF("BME280: Temp:%d, Press:%u, Hum:%u", bme_data->temperature, bme_data->pressure,
              bme_data->humidity);
      bme_read_ok = true;
    } else {
      LOG_ERR("Failed to read BME280 data.");
    }
  } else {
    LOG_ERR("BME280 not ready for reading.");
  }

  if (bq27441.is_ready()) {
    if (bq27441.read_data(bq_data) == Sensor<buzzverse_v1_BQ27441Data>::Status::OK) {
      LOG_INF("BQ27441: V(mV):%u, I(mA):%d, SoC:%u", bq_data->voltage_mv, bq_data->current_ma,
              bq_data->state_of_charge);
      bq_read_ok = true;
    } else {
      LOG_ERR("Failed to read BQ27441 data.");
    }
  } else {
    LOG_ERR("BQ27441 not ready for reading.");
  }

  if (!bme_read_ok && !bq_read_ok) {
    LOG_WRN("Failed to read data from any primary sensor.");
  }
}

void send_lora_packet(const buzzverse_v1_BME280Data* bme_data,
                      const buzzverse_v1_BQ27441Data* bq_data) {
  LOG_INF("Preparing LoRaWAN packet...");
  buzzverse_v1_Packet packet = buzzverse_v1_Packet_init_default;

  // Prioritize BME280 data in the packet
  if (!bme_data->temperature || !bme_data->pressure || !bme_data->humidity) {
    packet.which_data = buzzverse_v1_Packet_bme280_tag;
    packet.data.bme280 = *bme_data;
  } else {
    LOG_WRN("No valid sensor data to send in LoRaWAN packet.");
    return;
  }

  if (lorawan.is_ready()) {
    LOG_INF("Sending packet via LoRaWAN...");
    if (lorawan.send_packet(packet) != LoRaWANHandler::Status::OK) {
      LOG_ERR("Failed to send LoRaWAN packet.");
    } else {
      LOG_INF("LoRaWAN packet sent/queued.");
    }
  } else {
    LOG_ERR("LoRaWAN handler not ready.");
  }
}

int main(void) {
  LOG_INF("===== Buzzverse Node Application Starting =====");

  initialize_peripherals();

#ifdef CONFIG_SOC_ESP32S3&& CONFIG_ENABLE_DEVICE_SLEEP
  if (sleep_manager.is_ready()) {
    esp_sleep_wakeup_cause_t cause = sleep_manager.get_wakeup_cause();
    LOG_INF("Wakeup cause: %d (0:POR/Undef, 2:Timer)", cause);
  }
#endif

  buzzverse_v1_BME280Data bme280_data = buzzverse_v1_BME280Data_init_zero;
  buzzverse_v1_BQ27441Data bq27441_data = buzzverse_v1_BQ27441Data_init_zero;

  read_sensor_data(&bme280_data, &bq27441_data);
  send_lora_packet(&bme280_data, &bq27441_data);

#ifdef CONFIG_ENABLE_DEVICE_SLEEP

  if (sleep_manager.is_ready()) {
    LOG_INF("Entering deep sleep for %d ms...", APP_SLEEP_DURATION_MS);
    sleep_manager.set_sleep_duration(APP_SLEEP_DURATION_MS);
    sleep_manager.enter_sleep(SleepManager::SleepMode::DEEP_SLEEP);
    // ESP32 resets on deep sleep wake; this line should ideally not be reached.
    LOG_ERR("!!! FATAL: Deep sleep did not initiate reset. !!!");
  } else {
    LOG_ERR("SleepManager not ready. Falling back to k_sleep for %d ms.", APP_SLEEP_DURATION_MS);
    k_sleep(K_MSEC(APP_SLEEP_DURATION_MS));
  }

  // Fallback if deep sleep fails or isn't supported; device will continuously k_sleep.
  LOG_WRN("Reached end of main after sleep attempt. Looping with k_sleep.");
  while (true) {
    k_sleep(K_SECONDS(APP_SLEEP_DURATION_MS / 1000));
    LOG_INF("Fallback k_sleep loop iteration.");
  }

#endif

  return 0;
}