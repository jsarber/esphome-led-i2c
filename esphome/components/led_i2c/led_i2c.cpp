#include "led_i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace led_i2c {

static const char *const TAG = "led_i2c";

static uint8_t to_byte(float value) {
  if (!(value > 0.0f)) return 0;  // Also guards NaN before integer conversion.
  if (value >= 1.0f) return 255;
  return static_cast<uint8_t>(value * 255.0f + 0.5f);
}

light::LightTraits LedI2C::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::RGB});
  return traits;
}

void LedI2C::setup() {
  started_ = true;
  this->resync();
  wait_ = command_interval_;
  last_attempt_ = millis() - wait_;
}

void LedI2C::resync() {
  if (!started_) return;
  pending_ = 0x07;
  next_channel_ = 0;
}

void LedI2C::write_state(light::LightState *state) {
  float red, green, blue;
  // ESPHome applies on/off, overall/color brightness and gamma before this API returns.
  state->current_values_as_rgb(&red, &green, &blue);
  const uint8_t next[3] = {to_byte(red), to_byte(green), to_byte(blue)};
  for (uint8_t i = 0; i < 3; ++i) {
    if (rgb_[i] != next[i]) {
      rgb_[i] = next[i];
      if (started_) pending_ |= 1U << i;
    }
  }
}

void LedI2C::loop() {
  if (!started_ || pending_ == 0) return;
  if (static_cast<uint32_t>(millis() - last_attempt_) < wait_) return;
  uint8_t channel = next_channel_;
  while (!(pending_ & (1U << channel))) channel = (channel + 1) % 3;
  // Exact register/value pair, not three-register auto-increment or pointer read.
  const uint8_t command[2] = {static_cast<uint8_t>(0x20 + channel), rgb_[channel]};
  const auto error = this->write(command, sizeof(command));
  last_attempt_ = millis();
  if (error != i2c::ERROR_OK) {
    this->status_set_warning();
    ESP_LOGW(TAG, "Register 0x%02X at 0x%02X failed: %d; retrying", command[0], this->get_i2c_address(), error);
    this->resync();  // A slave reset may have cleared channels already sent.
    wait_ = command_interval_ > 1000 ? command_interval_ : 1000;
    return;
  }
  pending_ &= ~(1U << channel);
  next_channel_ = (channel + 1) % 3;
  wait_ = command_interval_;
  if (pending_ == 0) this->status_clear_warning();
}

void LedI2C::dump_config() {
  ESP_LOGCONFIG(TAG, "Single RGB LED I2C: 0x%02X, command spacing %u ms", this->get_i2c_address(),
                static_cast<unsigned>(command_interval_));
  ESP_LOGCONFIG(TAG, "  Write-only registers 0x20..0x22; requires STM32 neon_mode=0");
}

}  // namespace led_i2c
}  // namespace esphome
