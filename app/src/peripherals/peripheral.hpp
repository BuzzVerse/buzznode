#ifndef PERIPHERAL_HPP
#define PERIPHERAL_HPP

#include <etl/string.h>

constexpr size_t PERIPHERAL_NAME_SIZE = 32;

class Peripheral {
 public:
  virtual ~Peripheral() = default;

  // Initialize the peripheral
  virtual bool init() = 0;

  // Check if the peripheral is ready
  virtual bool is_ready() const = 0;

  // Get the name of the peripheral
  virtual etl::string<PERIPHERAL_NAME_SIZE> get_name() const = 0;
};

#endif  // PERIPHERAL_HPP