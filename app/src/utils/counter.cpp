#include "counter.hpp"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(counter_manager, LOG_LEVEL_DBG);

struct nvs_fs Counter::fs;

int Counter::mount() {
	int result;
	struct flash_pages_info page_info;

	/* Defining the NVS file system:
	 * sector_size equal to the pagesize,
	 * 2 sectors,
	 * starting at NVS_PARTITION_OFFSET.
	 */
	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		LOG_ERR("CounterManager::mount: Flash device %s is not ready", fs.flash_device->name);
		return 1;
	}

	fs.offset = NVS_PARTITION_OFFSET;

	result = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &page_info);
	if (result) {
		LOG_ERR("CounterManager::mount: Unable to get page info, error code: %d", result);
		return result;
	}
	fs.sector_size = page_info.size;

	fs.sector_count = 2U;

	result = nvs_mount(&fs);
	if (result) {
		LOG_ERR("CounterManager::mount: Mount failed, error code: %d", result);
		return result;
	}

	// The first time this code runs on a machine, it will save 0 as the counter value in the machine's NVS.
	// Every subsequent time, it will read the counter value from the NVS into the private field "counter".
	result = read_value_from_nvs();
	if (result == -2) { // error code -2 means "No such file or directory": docs.zephyrproject.org/latest/doxygen/html/errno_8h.html
		LOG_INF("CounterManager::mount: No counter found, adding it at id %d", COUNTER_ID);

		value = 0;
		result = write_value_to_nvs();
		if (result < 0) // Write error
			return result;
	} else if (result < 0) { // A different read error
		return result; 
	}

	return 0;
}

uint16_t Counter::get() {
	ssize_t result = read_value_from_nvs();

	if (result <= 0) // Counter not found
		return -1;

	return value;
}

void Counter::increment() {
	value++;
	write_value_to_nvs();
}

void Counter::reset() {
	value = 0;
	write_value_to_nvs();
}

ssize_t Counter::read_value_from_nvs() {
	ssize_t result = nvs_read(&fs, COUNTER_ID, &value, sizeof(value));

	if (result == 0)
		LOG_WRN("CounterManager::read_counter_from_nvs: no data read");
	else if (result < 0 && result != -2)
		LOG_ERR("CounterManager::read_counter_from_nvs: error code: %d", result);
	else if (result == -2)
		// This is expected to appear when this is called from mount() when this program runs for the first time on a device,
		// when the counter value hasn't been saved to NVS yet.
		LOG_WRN("CounterManager::read_counter_from_nvs: no such file or directory (error code: %d)", result);

	return result;
}

ssize_t Counter::write_value_to_nvs() {
	ssize_t result = nvs_write(&fs, COUNTER_ID, &value, sizeof(value));

	if (result == 0)
		LOG_WRN("CounterManager::write_counter_to_nvs: a rewrite of the same data already stored was attempted, nothing written");
	else if (result < 0)
		LOG_ERR("CounterManager::write_counter_to_nvs: error code: %d", result);

	return result;
}
