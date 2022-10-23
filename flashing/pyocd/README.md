# Flashing with pyocd

This is a different approach to flash the firmware on the b-parasites. It is completely optional.

## Basic instructions

(Based on the discussion in issue #67)

The instructions will probably differ slightly depending on your debugging probe and hardware setup.

The setup is fairly straightforward and documented in the `pyocd` [docs](https://pyocd.io/).

I used an inexpensive debugging probe from Aliexpress called `nanoDAP` from `Muse Labs` which is less than 10 € and readily available. The J-LINKs are hard to get and quite expensive. In my understanding basically all `CMSIS-DAP`-compatible probes which are recognized by `pyocd` should work.

After connecting the debugging probe via usb and connecting the b-parasite as described in the wiki, flashing was easy and fast.

For a complete flashing of softdevice and firmware use the provided python script. Assuming you put the `*.hex` files in default locations it boils down to one simple command:

```
python ./flash_firmware.py
```

For help and documentation of the available command line flags use

```
python ./flash_firmware.py -h
```