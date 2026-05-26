#pragma once

#include "esphome/core/component.h"
#include "esphome/core/optional.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"

#ifdef USE_ESP32

namespace esphome {
namespace switchbot_meter {

class SwitchbotMeter : public Component, public esp32_ble_tracker::ESPBTDeviceListener {
 public:
  void set_address(uint64_t address) { this->address_ = address; }

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_temperature(sensor::Sensor *temperature) { this->temperature_ = temperature; }
  void set_humidity(sensor::Sensor *humidity) { this->humidity_ = humidity; }
  void set_battery_level(sensor::Sensor *battery_level) { this->battery_level_ = battery_level; }

 protected:
  struct ParseResult {
    optional<float> temperature;
    optional<float> humidity;
    optional<float> battery_level;
  };

  bool parse_payload_(const esp32_ble_tracker::ESPBTDevice &device, ParseResult &result);

  uint64_t address_{0};
  sensor::Sensor *temperature_{nullptr};
  sensor::Sensor *humidity_{nullptr};
  sensor::Sensor *battery_level_{nullptr};
};

}  // namespace switchbot_meter
}  // namespace esphome

#endif  // USE_ESP32
