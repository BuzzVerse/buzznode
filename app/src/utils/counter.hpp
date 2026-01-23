#ifndef COUNTER_HPP
#define COUNTER_HPP

#include <zephyr/drivers/flash.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>

#define NVS_PARTITION			storage_partition // label of the storage parition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)
#define COUNTER_ID 				1

/*
 * A class that stores a 16-bit unsigned integer value
 * in the device's flash memory (Non-Volatile Storage).
 * To use, create a Counter object and call mount() 
 * to initialize and mount the NVS filesystem, as well as to start the counter.
 * After that, it's possible to increment and reset the counter, as well as to get its value.
 */
class Counter {
	public:
		/**
		 * @brief Defines and mounts the NVS filesystem with settings
		 * defined in this file
		 *
		 * @return 0 on success, 1 if the flash device isn't ready, or a negative error code on error
		 */
		int mount();

		/**
		 * @brief Reads the counter value from the NVS and returns it.
		 *
		 * @return The counter's current value or -1 if read failed
		 */
		uint16_t get();

		/**
		 * @brief Increments the counter value and writes it to the NVS.
		 */
		void increment();

		/**
		 * @brief Sets the counter value to 0 and writes it to the NVS.
		 */
		void reset();

	private:
		static struct nvs_fs fs;
		uint16_t value;

		/**
		 * @brief Wrapper for nvs_read()
		 *
		 * @return Return values same as nvs_read(): 
		 * docs.zephyrproject.org/latest/doxygen/html/group__nvs__high__level__api.html#ga341fd2ad029709cbb6eafde1ae88603f
		 */
		ssize_t read_value_from_nvs();

		/**
		 * @brief Wrapper for nvs_write().
		 *
		 * @return Return values same as nvs_write():
		 * docs.zephyrproject.org/latest/doxygen/html/group__nvs__high__level__api.html#ga34d40e9f63ba514d7915b72c4fef0b82
		 */
		ssize_t write_value_to_nvs();
};

#endif  // COUNTER_HPP
