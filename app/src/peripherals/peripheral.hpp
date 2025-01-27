#ifndef PERIPHERAL_HPP
#define PERIPHERAL_HPP

#include <etl/string.h>

class Peripheral {
 public:
  virtual ~Peripheral() = default;

  // Initialize the peripheral
  virtual bool init() = 0;

  // Check if the peripheral is ready
  virtual bool is_ready() const = 0;

  // Get the name of the peripheral
  virtual etl::string<64> get_name() const = 0;
};

#endif  // PERIPHERAL_HPP