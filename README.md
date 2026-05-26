# ESPHome SwitchBot Meter

An [ESPHome](https://esphome.io) external component that speaks the SwitchBot
BLE advertisement protocol and turns SwitchBot temperature/humidity meters into
native ESPHome sensor entities.

It runs entirely on your ESP32 — no SwitchBot Hub, no app, and no cloud. The
ESP32 passively listens for the readings each meter already broadcasts over
Bluetooth Low Energy and publishes them straight to Home Assistant (or MQTT,
the web server, etc.).

## Supported devices

| Device | Model | Notes |
| ------ | ----- | ----- |
| SwitchBot Meter | WoSensorTH (`T`) | Indoor |
| SwitchBot Meter Plus | `i` | Indoor |
| SwitchBot Outdoor Meter / Indoor-Outdoor Thermo-Hygrometer | WoIOSensorTH (`w`) | Outdoor |

A single `switchbot_meter` platform handles all of them — you do not need to
tell it which model you have. It decodes the reading from the service data
(older indoor meters) or the manufacturer data (outdoor meter and newer
firmware) automatically.

## Requirements

- An ESP32 (BLE is required, so the original ESP8266 is not supported).
- ESPHome with the [`esp32_ble_tracker`](https://esphome.io/components/esp32_ble_tracker.html)
  component. A **passive** scan is enough — the meters broadcast their data, so
  no connection or active scan is needed.

## Installation

Add this repository as an external component:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/onetxpanda/esphome-switchbot
    components: [switchbot_meter]
```

## Configuration

```yaml
esp32_ble_tracker:
  scan_parameters:
    active: false

sensor:
  # Indoor Meter / Meter Plus
  - platform: switchbot_meter
    mac_address: "C1:11:22:33:44:55"
    temperature:
      name: "Living Room Temperature"
    humidity:
      name: "Living Room Humidity"
    battery_level:
      name: "Living Room Meter Battery"

  # Outdoor Meter (WoIOSensorTH)
  - platform: switchbot_meter
    mac_address: "C2:66:77:88:99:AA"
    temperature:
      name: "Outdoor Temperature"
    humidity:
      name: "Outdoor Humidity"
    battery_level:
      name: "Outdoor Meter Battery"
```

A complete, flashable example is in [`example.yaml`](example.yaml).

### Configuration variables

- **mac_address** (*Required*, MAC address): the Bluetooth MAC address of the
  meter to read.
- **temperature** (*Optional*): a [Sensor](https://esphome.io/components/sensor/index.html)
  for the temperature, in °C.
- **humidity** (*Optional*): a Sensor for the relative humidity, in %.
- **battery_level** (*Optional*): a Sensor for the battery level, in %.
- **esp32_ble_tracker_id** (*Optional*, ID): the ID of the `esp32_ble_tracker`
  to use, if you have more than one.
- All other options from
  [Sensor](https://esphome.io/components/sensor/index.html) and the base
  [Component](https://esphome.io/guides/configuration-types.html#config-component)
  are also supported.

All three sensors are optional, so you can expose only the values you care
about.

## Finding a meter's MAC address

The easiest way is to let ESPHome discover it for you. Add an
`esp32_ble_tracker:` block, flash the device, and watch the logs — every nearby
BLE device is printed with its MAC address. SwitchBot meters show up with a
service data UUID of `0xFD3D`. You can also find the MAC in the SwitchBot app
under the device's settings, or with a BLE scanner app on your phone.

## How it works

SwitchBot meters continuously broadcast their current reading in their BLE
advertisements. This component registers an `esp32_ble_tracker` listener,
matches advertisements by MAC address, and decodes:

- **Temperature** — integer and decimal parts with a sign bit.
- **Humidity** — percentage.
- **Battery** — percentage, from the service data.

The byte layout follows SwitchBot's published BLE specification and matches the
[`pySwitchbot`](https://github.com/sblibs/pySwitchbot) parser used by Home
Assistant's native SwitchBot integration.

## License

See the repository for license details.
