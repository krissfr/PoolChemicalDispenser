#pragma once

#include "esphome/core/component.h"
#include "esphome/components/globals/globals_component.h"
#include "esphome/components/number/number.h"

namespace esphome {
namespace dlc32_stepper {

class DLC32Stepper : public Component {
 public:
  void setup() override;
  static void vTaskPeriodic(void *pvParameters);

  void set_xstep(globals::GlobalsComponent<int> *xstep) { xstep_ = xstep; }
  void set_ystep(globals::GlobalsComponent<int> *ystep) { ystep_ = ystep; }
  void set_zstep(globals::GlobalsComponent<int> *zstep) { zstep_ = zstep; }
  
  void set_xdir(globals::GlobalsComponent<bool> *xdir) { xdir_ = xdir; }
  void set_ydir(globals::GlobalsComponent<bool> *ydir) { ydir_ = ydir; }
  void set_zdir(globals::GlobalsComponent<bool> *zdir) { zdir_ = zdir; }

  void set_invert_x(bool invert) { invert_x_ = invert; }
  void set_invert_y(bool invert) { invert_y_ = invert; }
  void set_invert_z(bool invert) { invert_z_ = invert; }

  void set_step_interval(number::Number *step_interval) { step_interval_ = step_interval; }

  int xyzenregidx{0};
  int xstepregidx{1};
  int xdirregidx{2};
  int zstepregidx{3};
  int zdirregidx{4};
  int ystepregidx{5};
  int ydirregidx{6};

 private:
  void updateShiftRegister();
  void StepMotor();

  uint8_t my_register{0};

  globals::GlobalsComponent<int> *xstep_{nullptr};
  globals::GlobalsComponent<int> *ystep_{nullptr};
  globals::GlobalsComponent<int> *zstep_{nullptr};

  globals::GlobalsComponent<bool> *xdir_{nullptr};
  globals::GlobalsComponent<bool> *ydir_{nullptr};
  globals::GlobalsComponent<bool> *zdir_{nullptr};

  bool invert_x_{true};
  bool invert_y_{true};
  bool invert_z_{true};

  number::Number *step_interval_{nullptr};
};

}  // namespace dlc32_stepper
}  // namespace esphome

