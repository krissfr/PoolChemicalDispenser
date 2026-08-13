#include "dlc32_stepper.h"
#include "esphome/core/log.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include <cmath>

#define PIN_CLK   GPIO_NUM_16 // SH_CP (Horloge)
#define PIN_LATCH GPIO_NUM_17 // ST_CP (Verrou)
#define PIN_DATA  GPIO_NUM_21 // DS (Données)

#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#endif
#ifndef bitSet
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#endif
#ifndef bitClear
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#endif

namespace esphome {
namespace dlc32_stepper {

static const char *TAG = "dlc32_stepper";

void DLC32Stepper::setup() {
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << PIN_CLK) | (1ULL << PIN_LATCH) | (1ULL << PIN_DATA);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  this->my_register = (1 << this->xyzenregidx);
  this->updateShiftRegister();

  xTaskCreatePinnedToCore(
      DLC32Stepper::vTaskPeriodic,
      "vTaskPeriodicStepper",
      4096,
      this,
      1,
      NULL,
      0);
}

void DLC32Stepper::vTaskPeriodic(void *pvParameters) {
  DLC32Stepper *obj = (DLC32Stepper *)pvParameters;
  int64_t last_yield_time = esp_timer_get_time();

  float current_interval_ms = 0.5f;

  for (;;) {
    if (obj->xstep_ != nullptr && obj->xstep_->value() < 0) obj->xstep_->value() = 0;
    if (obj->ystep_ != nullptr && obj->ystep_->value() < 0) obj->ystep_->value() = 0;
    if (obj->zstep_ != nullptr && obj->zstep_->value() < 0) obj->zstep_->value() = 0;

    int x_val = (obj->xstep_ != nullptr && obj->xstep_->value() > 0) ? obj->xstep_->value() : 0;
    int y_val = (obj->ystep_ != nullptr && obj->ystep_->value() > 0) ? obj->ystep_->value() : 0;
    int z_val = (obj->zstep_ != nullptr && obj->zstep_->value() > 0) ? obj->zstep_->value() : 0;

    if (x_val == 0 && y_val == 0 && z_val == 0) {
      if (obj->my_register != (1 << obj->xyzenregidx)) {
        obj->my_register = (1 << obj->xyzenregidx);
        obj->updateShiftRegister();
      }
      current_interval_ms = 0.5f;
      vTaskDelay(pdMS_TO_TICKS(10));
      last_yield_time = esp_timer_get_time();
    } else {
      if (bitRead(obj->my_register, obj->xyzenregidx) == 1) {
        bitClear(obj->my_register, obj->xyzenregidx);
        obj->updateShiftRegister();
      }

      bool x_dir_state = obj->invert_x_;
      if (obj->xdir_ != nullptr) {
        x_dir_state = obj->xdir_->value() ^ obj->invert_x_;
      }
      if (x_dir_state) bitSet(obj->my_register, obj->xdirregidx);
      else bitClear(obj->my_register, obj->xdirregidx);

      bool y_dir_state = obj->invert_y_;
      if (obj->ydir_ != nullptr) {
        y_dir_state = obj->ydir_->value() ^ obj->invert_y_;
      }
      if (y_dir_state) bitSet(obj->my_register, obj->ydirregidx);
      else bitClear(obj->my_register, obj->ydirregidx);

      bool z_dir_state = obj->invert_z_;
      if (obj->zdir_ != nullptr) {
        z_dir_state = obj->zdir_->value() ^ obj->invert_z_;
      }
      if (z_dir_state) bitSet(obj->my_register, obj->zdirregidx);
      else bitClear(obj->my_register, obj->zdirregidx);

      float target_interval_ms = 1.0f;
      if (obj->step_interval_ != nullptr && !std::isnan(obj->step_interval_->state)) {
        target_interval_ms = obj->step_interval_->state;
      }
      if (target_interval_ms < 0.15f) target_interval_ms = 0.15f;

      const float ramp_step = 0.01f;
      if (current_interval_ms > target_interval_ms) {
        current_interval_ms -= ramp_step;
        if (current_interval_ms < target_interval_ms) current_interval_ms = target_interval_ms;
      } else if (current_interval_ms < target_interval_ms) {
        current_interval_ms += ramp_step;
        if (current_interval_ms > target_interval_ms) current_interval_ms = target_interval_ms;
      }

      obj->StepMotor();

      uint32_t delay_us = (uint32_t)(current_interval_ms * 1000.0f);
      if (delay_us > 5) {
        esp_rom_delay_us(delay_us - 5);
      }

      int64_t now = esp_timer_get_time();
      if ((now - last_yield_time) >= 10000) { 
        taskYIELD(); 
        last_yield_time = esp_timer_get_time();
      }
    }
  }
}

void DLC32Stepper::updateShiftRegister() {
  const uint32_t clk_mask   = (1UL << PIN_CLK);
  const uint32_t latch_mask = (1UL << PIN_LATCH);
  const uint32_t data_mask  = (1UL << PIN_DATA);

  GPIO.out_w1tc = latch_mask;

  for (int i = 7; i >= 0; i--) {
    GPIO.out_w1tc = clk_mask;
    if ((my_register >> i) & 0x01) {
      GPIO.out_w1ts = data_mask;
    } else {
      GPIO.out_w1tc = data_mask;
    }
    GPIO.out_w1ts = clk_mask;
  }

  GPIO.out_w1ts = latch_mask;
  GPIO.out_w1tc = clk_mask;
}

void DLC32Stepper::StepMotor() {
  int x_val = (this->xstep_ != nullptr && this->xstep_->value() > 0) ? this->xstep_->value() : 0;
  int y_val = (this->ystep_ != nullptr && this->ystep_->value() > 0) ? this->ystep_->value() : 0;
  int z_val = (this->zstep_ != nullptr && this->zstep_->value() > 0) ? this->zstep_->value() : 0;

  if (x_val == 0 && y_val == 0 && z_val == 0) return;

  if (x_val > 0) bitSet(my_register, xstepregidx);
  if (y_val > 0) bitSet(my_register, ystepregidx);
  if (z_val > 0) bitSet(my_register, zstepregidx);
  this->updateShiftRegister();

  esp_rom_delay_us(2);

  if (x_val > 0) {
    bitClear(my_register, xstepregidx);
    this->xstep_->value() = x_val - 1;
  }
  if (y_val > 0) {
    bitClear(my_register, ystepregidx);
    this->ystep_->value() = y_val - 1;
  }
  if (z_val > 0) {
    bitClear(my_register, zstepregidx);
    this->zstep_->value() = z_val - 1;
  }
  this->updateShiftRegister();
}

}  // namespace dlc32_stepper
}  // namespace esphome

