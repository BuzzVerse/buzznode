# Zephyr BuzzNode

https://github.com/zephyrproject-rtos/example-application

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Initialization

```shell
source ~/zephyrproject/.venv/bin/activate

git clone https://github.com/BuzzVerse/buzznode.git -b zephyr-workspace --recurse-submodules
west init -l buzznode
cd buzznode
west update
west blobs fetch hal_espressif
```

### Building and running

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
