ESP8266 SDK

/Users/aardvarkk/Library/Arduino15/packages/esp8266/hardware/esp8266/2.4.1/libraries/ESP8266WiFi/src/

https://www.letscontrolit.com/wiki/index.php/Basics:_Connecting_and_flashing_the_ESP8266
http://esp32.net/usb-uart/

https://docs.influxdata.com/influxdb/v1.3/introduction/getting_started/
https://docs.influxdata.com/influxdb/v1.3/query_language/data_exploration/

```
brew install influxdb
brew services start influxdb
curl "http://localhost:8086/query?q=show+databases"
influx -precision rfc3339
CREATE DATABASE kobold
SHOW DATABASES
USE kobold
INSERT readings,sensor=287AA6A807000029 temperature=21.56,voltage=3.29
curl -i -XPOST 'http://localhost:8086/write?db=kobold' --data-binary 'readings,sensor=287AA6A807000029 temperature=21.56',voltage=3.29
SELECT * FROM readings
SELECT * FROM readings WHERE sensor = '287AA6A807000029'
SELECT * FROM readings WHERE sensor = '287AA6A807000029' AND time >= now() - 1d
curl -G 'http://localhost:8086/query?pretty=true' --data-urlencode "db=kobold" --data-urlencode "q=SELECT * FROM readings WHERE sensor = '287AA6A807000029' AND time >= now() - 1d"
exit
```

`pg_dump -s kobold > schema.psql`

Sonoff Setup

- macOS High Sierra (10.13.2)
- Arduino IDE 1.8.5
- Issue: espcomm_upload_mem failed (https://github.com/arendst/Sonoff-Tasmota/issues/957) -- Apple default drivers don't seem to work
- Issue: upload from Arduino IDE stalls out

- `export SONOFF_PORT=/dev/cu.SLAB_USBtoUART`
- `export SONOFF_BIN=~/Desktop/esp8266_blink/esp8266_blink.ino.generic.bin`

esptool-ck (https://github.com/igrr/esptool-ck)

- For uploading firmware to the device given a USB -> serial connection
- `./esptool -h`
- Compile from Arduino to get location of bin file
- `ls -al /var/folders/rr/lyvlvm_d4573vc3r2ksk8x600000gn/T/arduino_build_275654/`
- Get name of device `find /dev -iname tty.*`
- `./esptool -cp $SONOFF_PORT -cf $SONOFF_BIN`
- `./esptool -vvv -cp $SONOFF_PORT -cf $SONOFF_BIN`
- Same stall -- seems to be what Arduino IDE uses under the hood

esptool.py (https://github.com/espressif/esptool)

- `sudo pip install --upgrade pip`
- `sudo pip install esptool`
- `esptool.py -h`
- `esptool.py -t`
- `export SONOFF_PORT=/dev/cu.SLAB_USBtoUART`
- `esptool.py -p $SONOFF_PORT erase_flash`
- `esptool.py -p $SONOFF_PORT write_flash 0x0 $SONOFF_BIN`
- `--after no-reset` to stay in bootloader mode
- `verify_flash` to check that values match
- `esptool.py --chip esp8266 elf2image --out ./ /var/folders/rr/lyvlvm_d4573vc3r2ksk8x600000gn/T/arduino_build_770259/esp8266_blink.ino.elf`
- `esptool.py -p $SONOFF_PORT write_flash 0x00000 0x00000.bin 0x01010 0x01010.bin`
- `esptool.py -p $SONOFF_PORT run`

Ino build
- doesn't seem to find ESP8266 board info

# node

## Install

### macOS

brew install node
brew install typescript
npm install --save @types/node
npm install --save express
npm install --save @types/express

### Ubuntu

curl -sL https://deb.nodesource.com/setup_10.x | sudo -E bash -
sudo apt-get install gcc g++ make build-essential nodejs

### Add authorized SSH key
ssh root@kosi.ca
mkdir ~/.ssh
cat ~/.ssh/id_rsa.pub | ssh root@kosi.ca ' cat >>.ssh/authorized_keys'

## Add GitHub deploy key

## Compiling

tsc --watch
npm start

## Running

# nginx

vim /etc/nginx/nginx.conf

# Working Setup

Board: Generic ESP8266 Module
Flash Mode: DOUT <- DID NOT WORK OTHERWISE!!!
Flash Size: 1M (no SPIFFS)
IwIP Variant: v2 Lower Memory
Reset Method: ck
Crystal Freq: 26MHz
Flash Freq: 40MHz
CPU Freq: 80MHz
Builtin LED: 13
Upload Speed: 115200
Erase Flash: Sketch Only <- All Flash Contents will erase the EEPROM too!
