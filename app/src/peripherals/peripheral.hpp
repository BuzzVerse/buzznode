#ifndef PERIPHERAL_HPP
#define PERIPHERAL_HPP

#include <etl/string.h>

constexpr size_t PERIPHERAL_NAME_SIZE = 32;

class Peripheral {
 public:
  /**
   * @brief Default destructor
   */
  virtual ~Peripheral() = default;

  /**
   * @brief Generic peripheral status codes
   */
  enum class Status {
    OK = 0,        /**< Operation successful */
    INIT_ERR = -1, /**< Initialization failed */
    NOT_READY = -2 /**< Peripheral is not ready */
  };

  /**
   * @brief Initialize the peripheral
   *
   * @return Status
   * @retval OK if the peripheral is successfully initialized
   * @retval INIT_ERR if the peripheral initialization fails
   * @retval NOT_READY if the peripheral is not ready
   */
  virtual Status init() = 0;

  /**
   * @brief Check if the peripheral is ready
   *
   * @return bool
   * @retval true if the peripheral is ready
   * @retval false if the peripheral is not ready
   */
  virtual bool is_ready() const = 0;

  /**
   * @brief Get the name of the peripheral
   *
   * @return etl::string<PERIPHERAL_NAME_SIZE>
   */
  virtual etl::string<PERIPHERAL_NAME_SIZE> get_name() const = 0;
};

#endif  // PERIPHERAL_HPP