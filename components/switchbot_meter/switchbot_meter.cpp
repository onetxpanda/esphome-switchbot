#include "switchbot_meter.h"

#ifdef USE_ESP32

#include "esphome/core/log.h"

namespace esphome {
namespace switchbot_meter {

static const char *const TAG = "switchbot_meter";

// Service data UUID advertised by SwitchBot meters (0xFD3D), plus the legacy
// 0x0D UUID used by early firmware.
static const uint16_t SERVICE_UUID = 0xFD3D;
static const uint16_t SERVICE_UUID_LEGACY = 0x000D;
// SwitchBot manufacturer (company) identifier used in manufacturer-specific data.
static const uint16_t MANUFACTURER_ID = 0x0969;

// Decode the SwitchBot temperature encoding from the two relevant bytes.
//   decimal_byte: lower nibble holds the tenths of a degree
//   integer_byte: bit 7 is the sign (set = positive), bits 6..0 the integer part
static float decode_temperature(uint8_t decimal_byte, uint8_t integer_byte) {
  float temperature = (integer_byte & 0x7F) + (decimal_byte & 0x0F) / 10.0f;
  if ((integer_byte & 0x80) == 0)
    temperature = -temperature;
  return temperature;
}

bool SwitchbotMeter::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  if (device.address_uint64() != this->address_)
    return false;

  ParseResult result;
  if (!this->parse_payload_(device, result))
    return false;

  ESP_LOGD(TAG, "Got SwitchBot Meter (%s):", device.address_str().c_str());

  if (result.temperature.has_value()) {
    ESP_LOGD(TAG, "  Temperature: %.1f°C", *result.temperature);
    if (this->temperature_ != nullptr)
      this->temperature_->publish_state(*result.temperature);
  }
  if (result.humidity.has_value()) {
    ESP_LOGD(TAG, "  Humidity: %.0f%%", *result.humidity);
    if (this->humidity_ != nullptr)
      this->humidity_->publish_state(*result.humidity);
  }
  if (result.battery_level.has_value()) {
    ESP_LOGD(TAG, "  Battery Level: %.0f%%", *result.battery_level);
    if (this->battery_level_ != nullptr)
      this->battery_level_->publish_state(*result.battery_level);
  }

  return true;
}

bool SwitchbotMeter::parse_payload_(const esp32_ble_tracker::ESPBTDevice &device, ParseResult &result) {
  bool have_temperature = false;

  // Newer indoor meters and the outdoor meter carry temperature/humidity in the
  // manufacturer-specific data (bytes 8..10), which is preferred when present.
  for (auto &md : device.get_manufacturer_datas()) {
    if (md.uuid == esp32_ble_tracker::ESPBTUUID::from_uint16(MANUFACTURER_ID) && md.data.size() >= 11) {
      result.temperature = decode_temperature(md.data[8], md.data[9]);
      result.humidity = static_cast<float>(md.data[10] & 0x7F);
      have_temperature = true;
      break;
    }
  }

  // Service data carries the battery level (byte 2) for every meter, and the
  // temperature/humidity (bytes 3..5) for older indoor meters.
  for (auto &sd : device.get_service_datas()) {
    if (!(sd.uuid == esp32_ble_tracker::ESPBTUUID::from_uint16(SERVICE_UUID) ||
          sd.uuid == esp32_ble_tracker::ESPBTUUID::from_uint16(SERVICE_UUID_LEGACY)))
      continue;

    if (sd.data.size() >= 3)
      result.battery_level = static_cast<float>(sd.data[2] & 0x7F);

    if (!have_temperature && sd.data.size() >= 6) {
      result.temperature = decode_temperature(sd.data[3], sd.data[4]);
      result.humidity = static_cast<float>(sd.data[5] & 0x7F);
      have_temperature = true;
    }
  }

  if (!have_temperature)
    return false;

  // Guard against empty advertisement frames that decode to all-zero values.
  if (result.temperature.value_or(0.0f) == 0.0f && result.humidity.value_or(0.0f) == 0.0f &&
      result.battery_level.value_or(0.0f) == 0.0f)
    return false;

  return true;
}

void SwitchbotMeter::dump_config() {
  ESP_LOGCONFIG(TAG, "SwitchBot Meter");
  ESP_LOGCONFIG(TAG, "  MAC address: %02X:%02X:%02X:%02X:%02X:%02X", static_cast<uint8_t>(this->address_ >> 40),
                static_cast<uint8_t>(this->address_ >> 32), static_cast<uint8_t>(this->address_ >> 24),
                static_cast<uint8_t>(this->address_ >> 16), static_cast<uint8_t>(this->address_ >> 8),
                static_cast<uint8_t>(this->address_ >> 0));
  LOG_SENSOR("  ", "Temperature", this->temperature_);
  LOG_SENSOR("  ", "Humidity", this->humidity_);
  LOG_SENSOR("  ", "Battery Level", this->battery_level_);
}

}  // namespace switchbot_meter
}  // namespace esphome

#endif  // USE_ESP32
