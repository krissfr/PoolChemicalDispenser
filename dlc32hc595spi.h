#include "esphome.h"

class DLC32HC595 : public Component {
 public:

  // HC595 input
  // int latchPin = 17;      // RCLK/Latch pin of 74HC595 is connected to Digital pin GPIO17
  // int clockPin = 16;      // SRCLK/Clock pin of 74HC595 is connected to Digital pin GPIO16
  // int dataPin = 21;       // SER/Data pin of 74HC595 is connected to Digital pin GPIO21

  // HC595 output
  int xyzenregidx = 0;        // my_register index to control Enable for X,Y and Z
  int xstepregidx = 1;        // my_register index to control Step for X
  int xdirregidx = 2;        // my_register index to control direction for X
  int zstepregidx = 3;        // my_register index to control Step for Z
  int zdirregidx = 4;        // my_register index to control direction for Z
  int ystepregidx = 5;       // my_register index to control Step for Y
  int ydirregidx = 6;        // my_register index to control direction for Y
  // idx 7 not used

  byte my_register = B01010100;         // Variable to hold the pattern of which Pins are currently turned on or off


  void setup() override {
    xTaskCreate(
        vTaskPeriodic, /* Task function. */
        "vTaskPeriodic", /* name of task. */
        10000, /* Stack size of task */
        NULL, /* parameter of the task */
        2, /* priority of the task */
        NULL); /* Task handle to keep track of created task */
 }

  static void vTaskPeriodic( void *pvParameters )
  {
    DLC32HC595 obj;
    for( ;; )
    {
        obj.StepMotor();
        // delayMicroseconds( id(step_interval).state );
        vTaskDelay( pdMS_TO_TICKS( id(step_interval).state ) ); // every 1 ms
    }
  }

  void updateShiftRegister() {
     id(spidev).enable();
     id(spidev).write_byte(my_register);
     id(spidev).disable();
  }

  void StepMotor() {
    if ( Xstep->value() != 0 or Ystep->value() != 0 or Zstep->value() != 0 ) {
      if ( bitRead(my_register, xyzenregidx) == 1 ) {
        bitClear(my_register, xyzenregidx);
        // ESP_LOGD("custom", "XYZ Enable register value is 0 (steppers are active with 0)");
      }
    }
    else {
      if ( bitRead(my_register, xyzenregidx) == 0 ) {
        bitSet(my_register, xyzenregidx);
        // ESP_LOGD("custom", "XYZ Enable register value is 1 (steppers are active with 0)");
      }
    }
    if ( Xstep->value() != 0 ) {
      bitToggle(my_register, xstepregidx);
      id(Xstep)--;
    }
    if ( Ystep->value() != 0 ) {
      bitToggle(my_register, ystepregidx);
      id(Ystep)--;
    }
    if ( Zstep->value() != 0 ) {
      bitToggle(my_register, zstepregidx);
      id(Zstep)--;
    }
    DLC32HC595::updateShiftRegister();
  }
};
