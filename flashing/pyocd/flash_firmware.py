import argparse

from pathlib import Path

from pyocd.core.helpers import ConnectHelper
from pyocd.core.target import Target
from pyocd.flash.file_programmer import FileProgrammer

from mac_address_reader import MacAddressReader

parser = argparse.ArgumentParser()
parser.add_argument("-s", "--softdevice", action="store", help="Optional file path to softdevice file.", default="../../sdk/s140nrf52720/s140_nrf52_7.2.0_softdevice.hex", type=Path)
parser.add_argument("-f", "--firmware", action="store", help="Optional file path to firmware file.", default="../../code/b-parasite/_build/nrf52840_xxaa.hex", type=Path)
parser.add_argument("--skip-softdevice", action="store_true", help="Skip flashing of the softdevice.")
parser.add_argument("--skip-firmware", action="store_true", help="Skip flashing of firmware.")

args = parser.parse_args()

with ConnectHelper.session_with_chosen_probe() as session:

    board = session.board
    target: Target = board.target

    if not args.skip_softdevice:
        softdevice_path: Path = args.softdevice
        print(f"Flashing softdevice {softdevice_path.name}...")
        FileProgrammer(session).program(str(softdevice_path))

        target.reset_and_halt()
        target.resume()

    if not args.skip_firmware:
        firmware_path: Path = args.firmware
        print(f"Flashing firmware {firmware_path.name}...")
        FileProgrammer(session).program(str(firmware_path))

        target.reset_and_halt()
        target.resume()
    
    mac_address_reader = MacAddressReader(target)
    mac_address = mac_address_reader.get_static_mac_address()
    print(f'Static manufacturer MAC address (DEVICEADDR): {mac_address}')
