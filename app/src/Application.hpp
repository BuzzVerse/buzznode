#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <etl/vector.h>

#include "buzzverse/bme280.pb.h"
#include "buzzverse/bq27441.pb.h"
#include "buzzverse/packet.pb.h"

// Forward declare to reduce include dependencies in this header
class BME280;
class BQ27441;
class LoRaWANHandler;
class SleepManager;
class Peripheral;

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  #ifndef CONFIG_SLEEP_TIME_MS
    #define APP_DEFAULT_SLEEP_DURATION_MS (30 * 1000)  // Default if Kconfig not set
  #else
    #define APP_DEFAULT_SLEEP_DURATION_MS CONFIG_SLEEP_TIME_MS
  #endif
#endif

class Application {
 public:
  Application();
  ~Application() = default;

  // Main phases of the application
  bool init();       // Initialize all components
  void run_cycle();  // Perform one operational cycle (read sensors, send data)

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  void enter_low_power_mode();  // Handle entering sleep
#endif

 private:
  // Internal helper methods
  bool initialize_peripherals();
  void read_sensor_data(buzzverse_v1_BME280Data& bme_data);
  void send_lora_packet(const buzzverse_v1_BME280Data& bme_data);

  // Peripheral members
  BME280& m_bme280;
  BQ27441& m_bq27441;
  LoRaWANHandler& m_lorawan;

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  SleepManager& m_sleep_manager;
#endif
  static constexpr uint8_t PERIPHERAL_COUNT =
#ifdef CONFIG_ENABLE_DEVICE_SLEEP
    4;
#else
    3;
#endif
  etl::vector<Peripheral*, PERIPHERAL_COUNT> m_peripherals;

#ifdef CONFIG_ENABLE_DEVICE_SLEEP
  int m_app_sleep_duration_ms = APP_DEFAULT_SLEEP_DURATION_MS;
#endif
};

#endif  // APPLICATION_HPP