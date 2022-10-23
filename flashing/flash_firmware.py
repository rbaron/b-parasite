import argparse

from pathlib import Path

from pyocd.core.helpers import ConnectHelper
from pyocd.core.target import Target
from pyocd.flash.file_programmer import FileProgrammer

from mac_converter import MacConverter

parser = argparse.ArgumentParser()
parser.add_argument("-a", "--all", action="store_true", help="Flash softdevice and firmware.")
parser.add_argument("--only-mac", action="store_true", help="Read only the mac address from controller.")
parser.add_argument("-s", "--softdevice", action="store", help="File path to softdevice file.", default="../sdk/s140nrf52720/s140_nrf52_7.2.0_softdevice.hex", type=Path)
parser.add_argument("-f", "--firmware", action="store", help="File path to firmware file.", default="../code/b-parasite/_build/nrf52840_xxaa.hex", type=Path)

args = parser.parse_args()

with ConnectHelper.session_with_chosen_probe() as session:

    board = session.board
    target: Target = board.target

    if not args.only_mac:
        if args.all:
            softdevice_path: Path = args.softdevice
            print(f"Flashing softdevice {softdevice_path.name}...")
            FileProgrammer(session).program(str(softdevice_path))

            target.reset_and_halt()
            target.resume()

        firmware_path: Path = args.firmware
        print(f"Flashing firmware {firmware_path.name}...")
        FileProgrammer(session).program(str(firmware_path))

        target.reset_and_halt()
        target.resume()
    
    mac_converter = MacConverter(target)
    mac_address = mac_converter.get_mac_address()
    print(f'Device MAC Address: {mac_address}')
