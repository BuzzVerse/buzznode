import questionary
import secrets
import os
import json


class BuzzCoreServices:
    def __init__(self):
        pass

    def choose_device_mode_of_config(self):
        pass

    def download_config_from_api(self):
        pass

    def register_device_to_api(self):
        pass

    def select_device(self):
        pass

    def login():
        pass

    def generate_random_hex(self, length):
        return secrets.token_hex(length // 2).upper()

    def save_device_conf(self, device_conf):
        app_dir = os.path.abspath("app")

        if not os.path.exists(app_dir):
            app_dir = os.path.abspath(".")

        local_conf = os.path.join(app_dir, "local.conf")

        with open(local_conf, "w") as file:
            json.dump(device_conf, file, indent=4)

    def generate_config(self, device):
        mode = "ABP" if device["nwkSEncKey"] else "OTAA"

        region = questionary.select(
            "Select region:",
            choices=["EU433", "EU868"],
        ).ask()

        config = []

        # Join mode configuration
        config.append("# Choose join mode (OTAA or ABP)")
        if mode == "ABP":
            config.append("CONFIG_LORAWAN_JOIN_OTAA=n")
            config.append("CONFIG_LORAWAN_JOIN_ABP=y")
        else:
            config.append("CONFIG_LORAWAN_JOIN_OTAA=y")
            config.append("CONFIG_LORAWAN_JOIN_ABP=n")

        # Device EUI
        dev_eui = device["devEui"].upper()
        config.append("")
        config.append("# Device EUI (required for both modes)")
        config.append(f'CONFIG_LORAWAN_DEV_EUI="{dev_eui}"')

        # Mode-specific configuration
        if mode == "OTAA":
            join_eui = device["joinEui"].upper()
            app_key = device["appKey"].upper()
            config.append("")
            config.append("# OTAA Configuration")
            config.append(f'CONFIG_LORAWAN_JOIN_EUI="{join_eui}"')
            config.append(f'CONFIG_LORAWAN_APP_KEY="{app_key}"')
        else:
            dev_addr = device["devAddr"].upper()
            app_skey = device["appSKey"].upper()
            nwk_skey = device["nwkSEncKey"].upper()
            config.append("")
            config.append("# ABP Configuration")
            config.append(f'CONFIG_LORAWAN_DEV_ADDR="{dev_addr}"')
            config.append(f'CONFIG_LORAWAN_APP_SKEY="{app_skey}"')
            config.append(f'CONFIG_LORAWAN_NWK_SKEY="{nwk_skey}"')

        # Region configuration
        config.append("")
        config.append("# Region configuration")
        if region == "EU433":
            config.append("CONFIG_LORAMAC_REGION_EU433=y")
            config.append("CONFIG_LORAMAC_REGION_EU868=n")
        else:
            config.append("CONFIG_LORAMAC_REGION_EU433=n")
            config.append("CONFIG_LORAMAC_REGION_EU868=y")

        return "\n".join(config) + "\n"
