from west.commands import WestCommand
from west import log
import argparse
import os
import secrets
from utils.hex import validate_hex

class RegisterDeviceCommand(WestCommand):
    def __init__(self):
        super().__init__(
            'register-device',
            'Register BuzzNode device with LoRaWAN credentials',
            'Configure LoRaWAN credentials for BuzzNode device')

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(self.name,
                                       help=self.help,
                                       description=self.description)

        parser.add_argument('--mode',
                          choices=['OTAA', 'ABP'],
                          default='ABP',
                          help='LoRaWAN activation mode (default: OTAA)')

        parser.add_argument('--dev-eui',
                          help='Device EUI (16 hex characters)')
        parser.add_argument('--join-eui',
                          help='Join EUI (16 hex characters)')
        parser.add_argument('--app-key',
                          help='Application Key (32 hex characters)')
        
        # ABP specific arguments
        parser.add_argument('--dev-addr',
                          help='Device Address (8 hex characters)')
        parser.add_argument('--app-skey',
                          help='Application Session Key (32 hex characters)')
        parser.add_argument('--nwk-skey',
                          help='Network Session Key (32 hex characters)')

        parser.add_argument('--region',
                          choices=['EU433', 'EU868'],
                          default='EU433',
                          help='LoRaWAN region frequency (default: EU433)')

        return parser

    def generate_random_hex(self, length):
        return secrets.token_hex(length//2).upper()

    def do_run(self, args, unknown_args):
        try:
            mode = args.mode
            region = args.region

            # Generate random values if not provided
            dev_eui = args.dev_eui if args.dev_eui else self.generate_random_hex(16)
            dev_eui = validate_hex(dev_eui, 16, 'Device EUI')

            if mode == 'OTAA':
                # OTAA specific values
                join_eui = args.join_eui if args.join_eui else self.generate_random_hex(16)
                app_key = args.app_key if args.app_key else self.generate_random_hex(32)
                
                join_eui = validate_hex(join_eui, 16, 'Join EUI')
                app_key = validate_hex(app_key, 32, 'Application Key')
            else:  # ABP mode
                # ABP specific values
                dev_addr = args.dev_addr if args.dev_addr else self.generate_random_hex(8)
                app_skey = args.app_skey if args.app_skey else self.generate_random_hex(32)
                nwk_skey = args.nwk_skey if args.nwk_skey else self.generate_random_hex(32)
                
                dev_addr = validate_hex(dev_addr, 8, 'Device Address')
                app_skey = validate_hex(app_skey, 32, 'Application Session Key')
                nwk_skey = validate_hex(nwk_skey, 32, 'Network Session Key')

            app_dir = os.path.abspath('app')
            if not os.path.exists(app_dir):
                app_dir = os.path.abspath('.')

            local_conf = os.path.join(app_dir, 'local.conf')
            gitignore_file = os.path.join(app_dir, '.gitignore')

            region_configs = {
                'EU433': {'EU433': 'y', 'EU868': 'n'},
                'EU868': {'EU433': 'n', 'EU868': 'y'}
            }
            region_conf = region_configs[region]

            # Generate configuration content based on mode
            config_content = f'''
# Choose join mode (OTAA or ABP)
CONFIG_LORAWAN_JOIN_OTAA={'y' if mode == 'OTAA' else 'n'}
CONFIG_LORAWAN_JOIN_ABP={'y' if mode == 'ABP' else 'n'}

# Device EUI (required for both modes)
CONFIG_LORAWAN_DEV_EUI="{dev_eui}"
'''

            if mode == 'OTAA':
                config_content += f'''
# OTAA Configuration
CONFIG_LORAWAN_JOIN_EUI="{join_eui}"
CONFIG_LORAWAN_APP_KEY="{app_key}"
'''
            else:  # ABP mode
                config_content += f'''
# ABP Configuration
CONFIG_LORAWAN_DEV_ADDR="{dev_addr}"
CONFIG_LORAWAN_APP_SKEY="{app_skey}"
CONFIG_LORAWAN_NWK_SKEY="{nwk_skey}"
'''

            config_content += f'''
# Region configuration
CONFIG_LORAMAC_REGION_EU433={region_conf['EU433']}
CONFIG_LORAMAC_REGION_EU868={region_conf['EU868']}
'''

            log.inf(f'Writing configuration to {local_conf}')
            with open(local_conf, 'w') as f:
                f.write(config_content)

            self._update_gitignore(gitignore_file)

            log.inf('LoRaWAN credentials saved successfully')
            log.inf('Generated credentials:')
            log.inf(f'Device EUI: {dev_eui}')
            
            if mode == 'OTAA':
                log.inf(f'Join EUI:   {join_eui} (can be shared across devices)')
                log.inf(f'App Key:    {app_key}')
            else:
                log.inf(f'DevAddr:    {dev_addr}')
                log.inf(f'AppSKey:    {app_skey}')
                log.inf(f'NwkSKey:    {nwk_skey}')
            
            log.inf(f'Mode:        {mode}')
            log.inf(f'Region:      {region}')
            log.inf(f'Configuration saved to: {local_conf}')

            # Print warnings for generated values
            if not args.dev_eui:
                log.wrn('Using randomly generated Device EUI')
            if mode == 'OTAA':
                if not args.join_eui:
                    log.wrn('Using default Join EUI (all zeros). Set --join-eui if your network server provides a specific value')
                if not args.app_key:
                    log.wrn('Using randomly generated App Key')
            else:
                if not args.dev_addr:
                    log.wrn('Using randomly generated Device Address')
                if not args.app_skey:
                    log.wrn('Using randomly generated Application Session Key')
                if not args.nwk_skey:
                    log.wrn('Using randomly generated Network Session Key')

        except Exception as e:
            log.die(f'Failed to save configuration: {str(e)}')

    def _update_gitignore(self, gitignore_file: str):
        if os.path.exists(gitignore_file):
            with open(gitignore_file, 'r') as f:
                gitignore = f.read()
            if 'local.conf' not in gitignore:
                with open(gitignore_file, 'a') as f:
                    f.write('\nlocal.conf\n')
        else:
            with open(gitignore_file, 'w') as f:
                f.write('local.conf\n')
