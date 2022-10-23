from pyocd.core.target import Target


class MacAddressReader():

    MEMORY_ADDRESS = 0x100000A4
    
    def __init__(self, target: Target) -> None:
        self.target = target

    def get_mac_address(self) -> str:
        memory_blocks = self._get_blocks()

        return self._convert_blocks(memory_blocks)

    def _get_blocks(self):
        memory_blocks = self.target.read_memory_block32(self.MEMORY_ADDRESS, 2)

        return memory_blocks

    def _convert_blocks(self, memory_blocks: list[int]) -> str:
        first_block = f'{memory_blocks[0]:x}'.rjust(8, '0')

        modified_second_block = memory_blocks[1] | 0xc000 # Bitwise OR

        second_block = f'{modified_second_block:x}'

        mac_string = (second_block[4:] + first_block)
        prettified_mac = ':'.join(mac_string[i:i+2] for i in range(0, len(mac_string), 2))

        return prettified_mac.upper()
