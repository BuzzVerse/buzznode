#include "lorawan_handler.hpp"

#include <etl/array.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>

LOG_MODULE_REGISTER(lorawan_handler, LOG_LEVEL_DBG);

bool LoRaWANHandler::init() {
  const device* lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
  if (!device_is_ready(lora_dev)) {
    LOG_WRN("%s: device not ready.", lora_dev->name);
    return false;
  }

  lorawan_enable_adr(false);
  lorawan_set_class(LORAWAN_CLASS_A);

  int ret = lorawan_start();
  if (ret < 0) {
    LOG_WRN("lorawan_start failed: %d", ret);
    return false;
  }

  struct lorawan_join_config join_cfg;
  join_cfg.mode = LORAWAN_ACT_OTAA;
  join_cfg.dev_eui = dev_eui.data();
  join_cfg.otaa.join_eui = join_eui.data();
  join_cfg.otaa.app_key = app_key.data();
  join_cfg.otaa.nwk_key = app_key.data();

  LOG_INF("Joining LoRaWAN network...");
  ret = lorawan_join(&join_cfg);
  if (ret < 0) {
    LOG_WRN("lorawan_join failed: %d", ret);
    return false;
  }

  LOG_INF("Successfully joined LoRaWAN network");
  connected = true;
  return true;
}

void LoRaWANHandler::send_message(const char* msg) const {
  etl::array<uint8_t, MAX_MSG_SIZE> buffer{};
  size_t msg_len = strlen(msg);
  size_t msg_size = etl::min(msg_len, MAX_MSG_SIZE);

  memcpy(buffer.data(), msg, msg_size);

  int ret = lorawan_send(2, buffer.data(), msg_size, LORAWAN_MSG_UNCONFIRMED);
  if (ret < 0) {
    LOG_ERR("lorawan_send failed: %d", ret);
  } else {
    LOG_INF("Message sent successfully: %s", msg);
  }
}