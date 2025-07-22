from west.commands import WestCommand
from west import log
import argparse
import os
import secrets
import requests
from utils.hex import validate_hex
import json
import questionary

from buzzcore_api import BuzzCoreApi
from buzzcore_service import BuzzCoreServices


class RegisterDeviceCommand(WestCommand):
    def __init__(self):
        super().__init__(
            "register-device",
            "Register BuzzNode device with LoRaWAN credentials",
            "Configure LoRaWAN credentials for BuzzNode device",
        )

        self.buzzcoreapi = BuzzCoreApi()
        self.buzzcoreservice = BuzzCoreServices()

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name, help=self.help, description=self.description
        )
        return parser

    def do_run(self, args, unknown_args):
        config_mode = questionary.select(
            "Select mode of config:",
            choices=["Download config from API", "Register device in api"],
        ).ask()

        if config_mode == "Download config from API":
            devices = self.buzzcoreapi.get_devices()
            choices = [
                questionary.Choice(
                    title=f"{d['name']} | {d['deviceProfileName']}", value=d["devEui"]
                )
                for d in devices
            ]
            selected_device = questionary.select(
                "Select device:",
                choices=choices,
            ).ask()
            device = self.buzzcoreapi.get_device_by_deveui(selected_device)
            device_conf = self.buzzcoreservice.generate_config(device)
            self.buzzcoreservice.save_device_conf(device_conf)
            print("Device config saved!")

        elif config_mode == "Register device in api":
            name = input("name:")
            region = questionary.select(
                "Region",
                choices=["EU433", "EU868"],
            ).ask()
            mode = questionary.select(
                "Mode",
                choices=["OTAA", "ABP"],
            ).ask()
            if mode == "ABP":
                devEui = self.buzzcoreservice.generate_random_hex(16)
                devAddr = self.buzzcoreservice.generate_random_hex(8)
                appSKey = self.buzzcoreservice.generate_random_hex(32)
                nwkSKey = self.buzzcoreservice.generate_random_hex(32)
                device = {
                    "devEui": f"{devEui}",  # 16
                    "name": f"{name}",
                    "region": f"{region}",
                    "joinMode": f"{mode}",
                    "devAddr": f"{devAddr}",  # 8
                    "appSKey": f"{appSKey}",  # 32
                    "nwkSKey": f"{nwkSKey}",  # 32
                }
            elif mode == "OTAA":
                devEui = self.generate_random_hex(16)
                joinEui = self.generate_random_hex(16)
                appKey = self.generate_random_hex(32)
                device = {
                    "devEui": f"{devEui}",  # 16
                    "name": f"{name}",
                    "region": f"{region}",
                    "joinMode": f"{mode}",
                    "joinEui": f"{joinEui}",  # 16
                    "appKey": f"{appKey}",  # 32
                }
            self.buzzcoreapi.post_register_device(device)
            self.buzzcoreservice.save_device_conf(device)
