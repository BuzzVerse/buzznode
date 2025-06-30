from west.commands import WestCommand
from west import log
import requests, time, webbrowser, sys
import os
import json

class LoginCommand(WestCommand):
    TOKEN_FILE = os.path.join(os.path.expanduser("~"), ".buzznode_token")
    BASE = "http://localhost:8080"

    def __init__(self):
        super().__init__(
            "login",
            "Login to BuzzCore API",
            "Register your device with the BuzzCore API ",
        )

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name, help=self.help, description=self.description
        )
        return parser

    def _save_token(self, token):
        try:
            with open(self.TOKEN_FILE, "w") as f:
                json.dump({"access_token": token}, f)
            log.inf("Token saved successfully.")
        except IOError as e:
            log.err(f"Error saving token: {e}")

    def _load_token(self):
        if os.path.exists(self.TOKEN_FILE):
            try:
                with open(self.TOKEN_FILE, "r") as f:
                    data = json.load(f)
                return data.get("access_token")
            except (IOError, json.JSONDecodeError) as e:
                log.err(f"Error loading token: {e}. Re-authenticating.")
                os.remove(self.TOKEN_FILE) # Remove corrupted file
        return None

    def _delete_token(self):
        """Deletes the token file."""
        if os.path.exists(self.TOKEN_FILE):
            try:
                os.remove(self.TOKEN_FILE)
                log.inf("Expired or invalid token removed.")
            except IOError as e:
                log.err(f"Error deleting token file: {e}")

    def _verify_and_use_token(self, token):
        log.inf("Verifying token...")
        r = requests.get(f"{self.BASE}/me", headers={"Authorization": f"Bearer {token}"})
        if r.status_code == 200:
            print("Successfully authenticated.")
            print(r.json())

            r = requests.get(f"{self.BASE}/devices", headers={"Authorization": f"Bearer {token}"})
            print("Devices:", r.json())
            return True
        else:
            log.wrn(f"Token verification failed (status: {r.status_code}).")
            return False

    def do_run(self, args, unknown_args):
        token = self._load_token()
        
        if token:
            log.inf("Attempting to use saved token...")
            if self._verify_and_use_token(token):
                return
            else:
                self._delete_token()    

        log.inf("Initiating new login process...")
        resp = requests.post(f"{self.BASE}/cli/auth/request").json()
        device = resp["device_code"]
        user_code = resp["user_code"]
        verify_uri = f"{self.BASE}{resp['verification_uri']}?user_code={user_code}"

        print(f"Open this URL in your browser and login:\n{verify_uri}\n")
        webbrowser.open(verify_uri)

        while True:
            poll = requests.get(f"{self.BASE}/cli/auth/poll", params={"deviceCode": device})
            if poll.status_code == 200:
                token = poll.json()["access_token"]
                self._save_token(token)
                break
            elif poll.status_code == 410:
                print("Code expired")
                sys.exit(1)
            time.sleep(3)

        if not self._verify_and_use_token(token):
            log.err("Failed to verify newly acquired token. This should not happen.")
            sys.exit(1)