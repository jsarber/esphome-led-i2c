#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/light/light_output.h"
#include <cstdint>

namespace esphome {
namespace led_i2c {

// One RGB LED on one STM32 module, exposed as a native ESPHome light.
class LedI2C : public Component, public light::LightOutput, public i2c::I2CDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  void resync();
  light::LightTraits get_traits() override;
  void write_state(light::LightState *state) override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void set_command_interval(uint32_t interval) { command_interval_ = interval < 1 ? 1 : interval; }

 protected:
  uint8_t rgb_[3]{0, 0, 0};
  uint8_t pending_{0};
  uint8_t next_channel_{0};
  bool started_{false};
  uint32_t command_interval_{10};
  uint32_t wait_{10};
  uint32_t last_attempt_{0};
};

}  // namespace led_i2c
}  // namespace esphome
