# BuzzVerse Monitoring Firmware

Welcome to the BuzzVerse Science Club's embedded firmware repository! This firmware, based on the Zephyr framework, is tailored for ESP-32-C3 and STM32WL microcontrollers with LoRa communication capabilities, providing a powerful solution for monitoring applications. Whether you're a club member, a fellow student, or an enthusiast, feel free to explore our firmware, contribute, and join us in advancing monitoring technologies.

## Overview

Our monitoring firmware leverages the Zephyr framework, ESP-32-S3 or STM32WL microcontrollers, and LoRa communication for efficient and long-range data transfer. This project aims to address monitoring needs across various domains, including environmental sensing, industrial monitoring, and more.

## Features

- **Zephyr Framework**: Utilizing the Zephyr framework for robust and feature-rich firmware development.
- **LoRa Communication**: Enabling long-range communication for data transfer in monitoring scenarios.

## Getting Started

To use the monitoring firmware in your project:

1. **Install Zephyr**: Follow the [Zephyr installation guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html#) to set up the Zephyr framework.

2. **Clone the Repository**: Clone the repository to your local machine using <br> `git clone https://github.com/BuzzVerse/buzznode.git`.

3. **Integrate Sensors**: Customize the firmware by integrating sensors relevant to your monitoring application.

4. **Configure LoRa Settings**: Adjust LoRa communication parameters based on your deployment requirements.

5. **Build and Flash**: Use the Zephyr build system to build the firmware and flash it onto your device.

    1. Activate the virtual environment:
        
         ```source {ENVIRONMENT_LOCATION}/.venv/bin/activate```
    2. Set the build system variables (this is needed because this is a standalone app - it may change in future):
        
        ```source {ZEPHYR_REPO_LOCATION}/zephyr-env.sh```
    3. Build the app for selected board:

         ```west build -b esp32s3_devkitc/esp32s3/procpu``` for esp32s3
    4. Flash the app

         ```west flash```

## Contributing

We welcome contributions from the community. If you'd like to contribute to the development of this firmware, follow the steps outlined in our [Contribution Guidelines](CONTRIBUTING.md).

## Documentation

The technical documentation of the project is available at [docs.buzzverse.dev](https://docs.buzzverse.dev).

## License

This firmware is licensed under the [MIT License](LICENSE). Feel free to use, modify, and distribute it in accordance with the license terms.

## Contact

For any inquiries or collaboration opportunities, please contact us at [contact@buzzverse.dev](mailto:contact@buzzverse.dev).
