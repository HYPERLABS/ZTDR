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
3. Remove `ftdi_sio` and `usbserial` kernel modules.
```
sudo rmmod ftdi_sio
sudo rmmod usbserial
```
4. Run `./ztdr_test`
