#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <zephyr/kernel.h>
#include "buzzverse/packet.pb.h"
#include "sensor.hpp"
#include "peripherals/sleep/sleep_manager.hpp"

// Number of supported sensor types used in the sensors array
#define BASE_SENSOR_COUNT 1

#ifdef CONFIG_ENABLE_ANALOG
    #define ANALOG_SENSOR_COUNT 1 // Analog sensor is enabled
#else
    #define ANALOG_SENSOR_COUNT 0 // Analog sensor is disabled
#endif

// The final fixed size is calculated by the preprocessor
#define NUMBER_OF_SENSORS (BASE_SENSOR_COUNT + ANALOG_SENSOR_COUNT)

class LoRaWANHandler;

class Application {
 public:
  /**
   * @brief Construct a new Application object.
   *
   * @param sensors Array of pointers to available sensors.
   * @param lorawan Reference to the LoRaWAN handler.
   * @param sleep_manager Pointer to the SleepManager. Can be nullptr if sleep is disabled.
   */
  Application(
          etl::array<etl::unique_ptr<Sensor>, NUMBER_OF_SENSORS>& sensors,
          LoRaWANHandler& lorawan,
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
  void send_lora_packet(const buzzverse_v1_Packet& packet);

  etl::array<etl::unique_ptr<Sensor>, NUMBER_OF_SENSORS>& m_sensors;
  LoRaWANHandler& m_lorawan;
  SleepManager* m_sleep_manager;
};

#endif  // APPLICATION_HPP
