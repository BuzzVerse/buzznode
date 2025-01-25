# Zephyr BuzzNode

<a href="https://zephyrproject-rtos.github.io/example-application">
  <img alt="Documentation" src="https://img.shields.io/badge/documentation-3D578C?logo=sphinx&logoColor=white">
</a>

https://github.com/zephyrproject-rtos/example-application

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Initialization

```shell
source ~/zephyrproject/.venv/bin/activate

west init -m https://github.com/BuzzVerse/buzznode --mr main buzznode
cd buzznode
west update
west blobs fetch hal_espressif
west blobs fetch hal_stm32
```

### Building and running BuzzNode ESP32

First time system build:
```shell
west build -b lora_node/esp32s3/procpu -p always app --sysbuild
west flash --esp-device /dev/cu.usbmodem101
```

To build the application, run the following command:
```shell
west build -b lora_node/esp32s3/procpu -p always app
```

Once you have built the application, run the following command to flash it:
```shell
west flash --esp-device /dev/cu.usbmodem101
```

Monitor logs:
```shell
west espressif monitor -p /dev/cu.usbmodem101
```

### Building and running STM32
Build:
```shell
west build -b nucleo_wl55jc -p always app --sysbuild
```

Flash:
```shell
west flash
```

Monitor logs:
```shell
minicom -D /dev/cu.usbmodem101
```

### Testing

To execute Twister integration tests, run the following command:

```shell
west twister -T tests --integration
```

### Documentation

A minimal documentation setup is provided for Doxygen and Sphinx. To build the
documentation first change to the ``doc`` folder:

```shell
cd doc
```

Before continuing, check if you have Doxygen installed. It is recommended to
use the same Doxygen version used in [CI](.github/workflows/docs.yml). To
install Sphinx, make sure you have a Python installation in place and run:

```shell
pip install -r requirements.txt
```

API documentation (Doxygen) can be built using the following command:

```shell
doxygen
```

The output will be stored in the ``_build_doxygen`` folder. Similarly, the
Sphinx documentation (HTML) can be built using the following command:

```shell
make html
```

The output will be stored in the ``_build_sphinx`` folder. You may check for
other output formats other than HTML by running ``make help``.