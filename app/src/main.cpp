#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
LOG_MODULE_REGISTER(buzznode);

#define FREQUENCY 433000000
#define BANDWIDTH BW_125_KHZ
#define SPREADING_FACTOR SF_12
#define CODING_RATE CR_4_8
#define PREAMBLE_LENGTH 8

void send_packet(const struct device *lora_dev) {
    uint8_t tx_payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
    
    int ret = lora_send(lora_dev, tx_payload, sizeof(tx_payload));
    if (ret < 0) {
        LOG_ERR("Failed to send packet: %d", ret);
        return;
    }

    LOG_INF("Packet sent: %s", tx_payload);
}

int main(void) {
    const struct device *lora_dev;

    lora_dev = DEVICE_DT_GET(DT_ALIAS(lora0));
    if (!device_is_ready(lora_dev)) {
        printk("%s: device not ready.", lora_dev->name);
        return 0;
    }

    struct lora_modem_config config = {
        .frequency = FREQUENCY,
        .bandwidth = BANDWIDTH,
        .datarate = SPREADING_FACTOR,
        .coding_rate = CODING_RATE,
        .preamble_len = PREAMBLE_LENGTH,
        .tx_power = 4,
        .tx = true,
        .iq_inverted = false,
        .public_network = false
    };

    if (lora_config(lora_dev, &config) < 0) {
        printk("Failed to configure LoRa device");
        return 0;
    }

    LOG_INF("LoRa device configured, sending packets...");
    while (1) {
        send_packet(lora_dev);
        k_sleep(K_SECONDS(10));
    }

    return 0;
}