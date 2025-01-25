#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "packet.h"

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
LOG_MODULE_REGISTER(buzznode);

#define FREQUENCY 433000000
#define BANDWIDTH BW_125_KHZ
#define SPREADING_FACTOR SF_12
#define CODING_RATE CR_4_8
#define PREAMBLE_LENGTH 4

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define SW0_NODE DT_ALIAS(sw0)
#define LORA_NODE DT_ALIAS(lora0)

#if DT_NODE_EXISTS(LED0_NODE)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#endif

#if DT_NODE_EXISTS(LED1_NODE)
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#endif

#if DT_NODE_EXISTS(SW0_NODE)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
#endif

#if DT_NODE_EXISTS(LORA_NODE)
const struct device *const lora_dev = DEVICE_DT_GET(LORA_NODE);
#endif

void send_packet(const struct device *lora_dev) {
  packet_t packet;
  packet.version = 1;
  packet.id = 1;
  packet.msgID = 1;
  packet.msgCount = 1;
  packet.dataType = SMS;
  packet.data[0] = 0x01;

  uint8_t data[PACKET_SIZE];
  memcpy(data, &packet, PACKET_SIZE);

  LOG_INF("Sending packet...");
  gpio_pin_set_dt(&led1, 1);

  int ret = lora_send(lora_dev, data, PACKET_SIZE);

  gpio_pin_set_dt(&led1, 0);

  if (ret < 0) {
    LOG_ERR("Failed to send packet: %d", ret);
    return;
  }

  LOG_INF("Packet sent successfully");
}

void on_button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
  LOG_INF("Button pressed");
}

int main(void) {
#if DT_NODE_EXISTS(LED0_NODE) && DT_NODE_EXISTS(LED1_NODE)
  if (device_is_ready(led0.port) && device_is_ready(led1.port)) {
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT);
    gpio_pin_set_dt(&led0, 1);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT);
    gpio_pin_set_dt(&led1, 0);
  }
#endif

#if DT_NODE_EXISTS(SW0_NODE)
  static struct gpio_callback button_cb_data;
  if (device_is_ready(button.port)) {
    gpio_pin_configure_dt(&button, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    gpio_init_callback(&button_cb_data, on_button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
  }
#endif

#if DT_NODE_EXISTS(LORA_NODE)
  if (!device_is_ready(lora_dev)) {
    LOG_ERR("LoRa device not ready");
    return 0;
  }

  struct lora_modem_config config = {.frequency = FREQUENCY,
                                     .bandwidth = BANDWIDTH,
                                     .datarate = SPREADING_FACTOR,
                                     .coding_rate = CODING_RATE,
                                     .preamble_len = PREAMBLE_LENGTH,
                                     .tx_power = 20,
                                     .tx = true,
                                     .iq_inverted = false,
                                     .public_network = false};

  if (lora_config(lora_dev, &config) < 0) {
    printk("Failed to configure LoRa device");
    return 0;
  }

  LOG_INF("LoRa device configured, sending packets...");
  while (1) {
    send_packet(lora_dev);
    k_sleep(K_SECONDS(5));
  }

#endif

  return 0;
}