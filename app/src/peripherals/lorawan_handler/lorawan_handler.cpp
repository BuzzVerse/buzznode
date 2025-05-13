#include "lorawan_handler.hpp"

#include <etl/array.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/lorawan/lorawan.h>
#include <zephyr/pm/device.h>

#include "utils/hex.hpp"
#include "utils/packet_handler.hpp"

LOG_MODULE_REGISTER(lorawan_handler, LOG_LEVEL_DBG);

Sensor<buzzverse_v1_BQ27441Data>* LoRaWANHandler::battery_sensor = nullptr;

LoRaWANHandler::LoRaWANHandler(Sensor<buzzverse_v1_BQ27441Data>& battery_sensor) {
  LoRaWANHandler::battery_sensor = &battery_sensor;

#if defined(CONFIG_LORAWAN_JOIN_OTAA)
  const char* dev_eui_str = CONFIG_LORAWAN_DEV_EUI;
  const char* join_eui_str = CONFIG_LORAWAN_JOIN_EUI;
  const char* app_key_str = CONFIG_LORAWAN_APP_KEY;

  LOG_INF("LoRaWAN Handler (OTAA) initialized");
  LOG_INF("Device EUI: %s", dev_eui_str);
  LOG_INF("Join EUI: %s", join_eui_str);
  LOG_INF("App Key: %s", app_key_str);

  for (size_t i = 0; i < 8; i++) {
    dev_eui[i] = utils::hex2val<uint8_t>(&dev_eui_str[i * 2]);
    join_eui[i] = utils::hex2val<uint8_t>(&join_eui_str[i * 2]);
  }

  for (size_t i = 0; i < 16; i++) {
    app_key[i] = utils::hex2val<uint8_t>(&app_key_str[i * 2]);
  }

#elif defined(CONFIG_LORAWAN_JOIN_ABP)
  const char* dev_addr_str = CONFIG_LORAWAN_DEV_ADDR;
  const char* app_skey_str = CONFIG_LORAWAN_APP_SKEY;
  const char* nwk_skey_str = CONFIG_LORAWAN_NWK_SKEY;

  LOG_INF("LoRaWAN Handler (ABP) initialized");
  LOG_INF("DevAddr: %s", dev_addr_str);
  LOG_INF("AppSKey: %s", app_skey_str);
  LOG_INF("NwkSKey: %s", nwk_skey_str);

  dev_addr = utils::hex2val<uint32_t>(dev_addr_str);

  for (size_t i = 0; i < 16; i++) {
    app_skey[i] = utils::hex2val<uint8_t>(&app_skey_str[i * 2]);
    nwk_skey[i] = utils::hex2val<uint8_t>(&nwk_skey_str[i * 2]);
  }
#endif  // CONFIG_LORAWAN_JOIN_OTAA vs CONFIG_LORAWAN_JOIN_ABP
}

uint8_t LoRaWANHandler::battery_level_callback() {
  if (nullptr == battery_sensor) {
    LOG_ERR("Battery sensor is not set. Returning 255.");
    return 255;
  }

  if (!battery_sensor->is_ready()) {
    LOG_ERR("Battery sensor is not ready, returning 255.");
    return 255;
  }

  buzzverse_v1_BQ27441Data battery_data = buzzverse_v1_BQ27441Data_init_zero;
  auto status = battery_sensor->read_data(&battery_data);
  if (Sensor<buzzverse_v1_BQ27441Data>::Status::OK != status) {
    LOG_ERR("Battery sensor read failed: %d", static_cast<int>(status));
    return 255;
  }

  uint8_t soc = battery_data.state_of_charge;

  if (0 == soc) {
    LOG_WRN("Battery level is 0, possibly external power source.");
    return 0;
  } else if (soc > 100) {
    LOG_ERR("Invalid battery percentage: %d", soc);
    return 255;
  }

  uint8_t lorawan_battery_level = (soc * 254) / 100;
  return lorawan_battery_level == 0 ? 1 : lorawan_battery_level;
}

Peripheral::Status LoRaWANHandler::init() {
  const device* lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
  if (!device_is_ready(lora_dev)) {
    LOG_WRN("%s: device not ready.", lora_dev->name);
    return Peripheral::Status::INIT_ERR;
  }

  int ret = lorawan_start();
  if (ret < 0) {
    LOG_WRN("lorawan_start failed: %d", ret);
    return Peripheral::Status::INIT_ERR;
  }

  lorawan_enable_adr(false);

  struct lorawan_join_config join_cfg = {};

#if defined(CONFIG_LORAWAN_JOIN_OTAA)
  // OTAA join
  join_cfg.mode = LORAWAN_ACT_OTAA;
  join_cfg.dev_eui = dev_eui.data();
  join_cfg.otaa.join_eui = join_eui.data();
  join_cfg.otaa.app_key = app_key.data();
  join_cfg.otaa.nwk_key = app_key.data();

  LOG_INF("Joining LoRaWAN network using OTAA...");
#elif defined(CONFIG_LORAWAN_JOIN_ABP)
  // ABP join
  join_cfg.mode = LORAWAN_ACT_ABP;
  join_cfg.dev_eui = dev_eui.data();
  join_cfg.abp.dev_addr = dev_addr;
  join_cfg.abp.app_skey = app_skey.data();
  join_cfg.abp.nwk_skey = nwk_skey.data();

  LOG_INF("Joining LoRaWAN network using ABP...");
#endif

  ret = lorawan_join(&join_cfg);
  if (ret < 0) {
    LOG_WRN("lorawan_join failed: %d", ret);
    return Peripheral::Status::INIT_ERR;
  }

  LOG_INF("Successfully joined LoRaWAN network");

  lorawan_register_battery_level_callback(&LoRaWANHandler::battery_level_callback);

  LOG_INF("LoRaWAN battery level callback registered.");
  connected = true;
  return Peripheral::Status::OK;
}

LoRaWANHandler::Status LoRaWANHandler::send_packet(const buzzverse_v1_Packet& packet) const {
  uint8_t buffer[MAX_MSG_SIZE];
  size_t size = 0;

  // Encode the protobuf message
  if (!PacketHandler::encode(packet, buffer, size)) {
    LOG_ERR("Packet encoding failed");
    return Status::ENCODE_ERR;
  }

  // Send the encoded message over LoRaWAN
  int ret = lorawan_send(LORAWAN_PORT, buffer, size, LORAWAN_MSG_UNCONFIRMED);
  if (ret < 0) {
    LOG_ERR("LoRaWAN send failed: %d", ret);
    return Status::SEND_ERR;
  }

  LOG_INF("Packet sent successfully (size: %zu bytes)", size);
  return Status::OK;
}
