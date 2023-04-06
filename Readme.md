# ESP-NOW playground

The code is based on [this](https://randomnerdtutorials.com/esp-now-esp8266-nodemcu-arduino-ide/) article.

## Init the pio project

```console
pio boards nodemcu
```

```console
pio project init --board nodemcuv2
```

To create a project for multiple environments:

```console
pio project init -b nodemcuv2 -b d1_mini
```

To upload a project for a specific environment:

```console
pio run -t upload -e d1_mini
```

To monitor the device:

```console
pio device monitor -b 115200
```

To upload a project ad start monitoring the device right after that:

```console
pio run -e nodemcuv2 -t upload -t monitor
```

The baud rate can be defined in the `platformio.ini` file:

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200
```
