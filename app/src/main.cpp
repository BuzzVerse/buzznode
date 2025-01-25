#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>

#include "lorawan.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DELAY K_SECONDS(10)

int main(void) {
  uint8_t dev_eui[] = LORAWAN_DEV_EUI;
  uint8_t join_eui[] = LORAWAN_JOIN_EUI;
  uint8_t app_key[] = LORAWAN_APP_KEY;

  const struct device *lora_dev;

  lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
  if (!device_is_ready(lora_dev)) {
    LOG_ERR("%s: device not ready.", lora_dev->name);
    return -ENODEV;
  }

  lorawan_enable_adr(false);
  lorawan_set_class(LORAWAN_CLASS_A);

  int ret = lorawan_start();
  if (ret < 0) {
    LOG_ERR("lorawan_start failed: %d", ret);
    return ret;
  }

  struct lorawan_join_config join_cfg;
  join_cfg.mode = LORAWAN_ACT_OTAA;
  join_cfg.dev_eui = dev_eui;
  join_cfg.otaa.join_eui = join_eui;
  join_cfg.otaa.app_key = app_key;
  join_cfg.otaa.nwk_key = app_key;

  LOG_INF("Joining LoRaWAN network...");
  ret = lorawan_join(&join_cfg);
  if (ret < 0) {
    LOG_ERR("lorawan_join failed: %d", ret);
    return ret;
  }

  uint32_t counter = 0;

  while (1) {
    char msg[32];
    snprintf(msg, sizeof(msg), "Hello, World! %u", counter);

    ret = lorawan_send(2, (uint8_t *)msg, strlen(msg), LORAWAN_MSG_UNCONFIRMED);
    if (ret < 0) {
      LOG_ERR("lorawan_send failed: %d", ret);
    } else {
      LOG_INF("Message sent successfully: %s", msg);
    }

    counter++;
    k_sleep(DELAY);
  }

  return 0;
}