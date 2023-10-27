ZTDR
====

Universal software for HL1xxx instruments (FIFO architecture)

## ARM
### Build
1. Download FTDI drivers from [here](https://ftdichip.com/drivers/d2xx-drivers/). For RPi you would most likely need to use [ARMv6 driver](https://ftdichip.com/wp-content/uploads/2022/07/libftd2xx-arm-v6-hf-1.4.27.tgz) since Raspbian is compiled using ARMv6 instruction set.
2. Run the following to install the driver
```
tar xfvz libftd2xx-arm-v6-hf-1.4.27.tgz
cd build
sudo -s
cp libftd2xx.* /usr/local/lib
chmod 0755 /usr/local/lib/libftd2xx.so.1.4.27
ln -sf /usr/local/lib/libftd2xx.so.1.4.27 /usr/local/lib/libftd2xx.so
cd ..
cp ftd2xx.h /usr/local/include
cp WinTypes.h /usr/local/include
ldconfig -v
exit
```
3. Change directory to `<path>/ZTDR/CVI/` and run `make`

### Test
1. Change directory to `<path>/ZTDR/CVI/
2. Update the linker path using the following command **(make sure to replace** `<path>` **with the installation path)**:
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:~/<path>/ZTDR/CVI/
```
3. On most distributions, the linux kernel will have either a built-in or optional module called `ftdi_sio`. This will detect an FTDI device and automatically invoke the `usbserial` module and create devices such as `/dev/ttyUSB0`. When the `ftdi_sio` module is controlling an FTDI device it is not available to `libftd2xx`. If the library attempts to access the device it will receive a message `FT_Open failed`. Several workarounds are suggested by FTDI in the readme file attached in the driver package mentioned above. For testing you can simply remove `ftdi_sio` and `usbserial` kernel modules by running:
```
sudo rmmod ftdi_sio
sudo rmmod usbserial
```
4. Run `./ztdr_test`
