import requests
import os
import json


class BuzzCoreApi:
    def __init__(self):
        self.BASE_URL = "https://buzzcore.bykowski.dev"
        self.session = requests.Session()
        self.token_file = os.path.join(os.path.expanduser("~"), ".buzznode_token")
        with open(self.token_file, "r") as plik:
            token_data = json.load(plik)
            access_token = token_data["access_token"]
        self.session.headers.update(
            {
                "Content-Type": "application/json",
                "Authorization": f"Bearer {access_token}",
            }
        )

    def _get(self, endpoint):
        r = self.session.get(url=f"{self.BASE_URL}/{endpoint}")
        r.raise_for_status()
        return r.json()

    def _post(self, endpoint, data):
        r = self.session.post(url=f"{self.BASE_URL}/{endpoint}", json=data)
        r.raise_for_status()

        if r.text.strip():  # Only parse JSON if there's content
            return r.json()
        else:
            return {}  # Or return None if preferred

    def get_devices(self):
        devices = self._get("devices")
        return devices

    def get_device_by_deveui(self, deveui):
        devices = self._get(f"devices/{deveui}")
        return devices

    def post_register_device(self, device):
        response = self._post("devices", device)
        return response

    def me(self):
        response = self._get("me")
        return response
