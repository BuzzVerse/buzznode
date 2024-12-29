#include "etl/circular_buffer.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/sensor_data_types.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/kernel.h>
#include <zephyr/rtio/rtio.h>

const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme280);
SENSOR_DT_READ_IODEV(iodev, DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280),
                     {SENSOR_CHAN_AMBIENT_TEMP, 0}, {SENSOR_CHAN_HUMIDITY, 0},
                     {SENSOR_CHAN_PRESS, 0});

RTIO_DEFINE(ctx, 1, 1);
int main() {
  etl::circular_buffer<int32_t, 100> temp_buffer;

  printk("Hello %s\n", CONFIG_BOARD);

  if (dev == NULL) {
    return 0;
  }

  while (1) {
    uint8_t buf[128];

    int rc = sensor_read(&iodev, &ctx, buf, 128);

    if (rc != 0) {
      printk("%s: sensor_read() failed: %d\n", dev->name, rc);
      return rc;
    }

    const struct sensor_decoder_api *decoder;

    rc = sensor_get_decoder(dev, &decoder);

    if (rc != 0) {
      printk("%s: sensor_get_decode() failed: %d\n", dev->name, rc);
      return rc;
    }

    uint32_t temp_fit = 0;
    struct sensor_q31_data temp_data = {0};

    decoder->decode(buf, (struct sensor_chan_spec){SENSOR_CHAN_AMBIENT_TEMP, 0},
                    &temp_fit, 1, &temp_data);
    temp_buffer.push(temp_data.readings[0].temperature);

    uint32_t press_fit = 0;
    struct sensor_q31_data press_data = {0};

    decoder->decode(buf, (struct sensor_chan_spec){SENSOR_CHAN_PRESS, 0},
                    &press_fit, 1, &press_data);

    uint32_t hum_fit = 0;
    struct sensor_q31_data hum_data = {0};

    decoder->decode(buf, (struct sensor_chan_spec){SENSOR_CHAN_HUMIDITY, 0},
                    &hum_fit, 1, &hum_data);

    printk("temp: %s%d.%d; press: %s%d.%d; humidity: %s%d.%d, elements in "
           "buffer: %d\n",
           PRIq_arg(temp_data.readings[0].temperature, 6, temp_data.shift),
           PRIq_arg(press_data.readings[0].pressure, 6, press_data.shift),
           PRIq_arg(hum_data.readings[0].humidity, 6, hum_data.shift),
           temp_buffer.size());

    k_sleep(K_MSEC(500));
  }
  return 0;
}