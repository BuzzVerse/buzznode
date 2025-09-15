import re

def validate_hex(value: str, length: int, name: str) -> str:
    if not re.match(f'^[0-9A-Fa-f]{{{length}}}$', value):
        raise ValueError(f'Invalid {name}. Must be {length} hex characters')
    return value.upper() 