#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "buzzverse/bme280.pb.h"
#include "buzzverse/packet.pb.h"

class BME280;
class BQ27441;
class LoRaWANHandler;
class SleepManager;

class Application {
 public:
  /**
   * @brief Construct a new Application object.
   *
   * @param bme280 Reference to the BME280 sensor.
   * @param bq27441 Reference to the BQ27441 fuel gauge.
   * @param lorawan Reference to the LoRaWAN handler.
   * @param sleep_manager Pointer to the SleepManager. Can be nullptr if sleep is disabled.
   */
  Application(BME280& bme280, BQ27441& bq27441, LoRaWANHandler& lorawan,
              SleepManager* sleep_manager);
  ~Application() = default;

  // Main phases of the application
  bool init();       // Initialize all components
  void run_cycle();  // Perform one operational cycle (read sensors, send data)

  /**
   * @brief Puts the device into a low-power state.
   *
   * If a SleepManager was not provided during construction, this function will fall back
   * to a simple k_sleep to prevent high power consumption in an infinite loop.
   *
   * @param sleep_duration_ms The duration to sleep for.
   */
  void enter_low_power_mode(int sleep_duration_ms);

  /**
   * @brief Populates a status packet with the initialization state of peripherals.
   *
   * This should be called after a failed init() to create a diagnostic report.
   * @param packet The packet to be filled with status information.
   */
  void generate_init_failure_report(buzzverse_v1_Packet& packet);

 private:
  // Internal helper methods
  bool initialize_peripherals();
  void read_sensor_data(buzzverse_v1_BME280Data& bme_data);
  void send_lora_packet(const buzzverse_v1_BME280Data& bme_data);

  BME280& m_bme280;
  BQ27441& m_bq27441;
  LoRaWANHandler& m_lorawan;
  SleepManager* m_sleep_manager;
};

#endif  // APPLICATION_HPP