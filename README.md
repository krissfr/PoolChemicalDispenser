# PoolChemicalDispenser
a pool chemical dispenser with esphome and MKS DLC32 board

![alt text](https://github.com/krissfr/PoolChemicalDispenser/blob/main/PoolChemicalDispenser.jpg)

For this projet, you need :
- 1 MKS DLC32 board
- 3 stepper drivers (i use a4988)
- 3 pumps with stepper motor
- 1 128x64 SSD1309 SPI Oled display
- 1 rotary encoder with switch
- 1 12v or 24v power supply
- 1 ads115 (for the current sensor)
- 1 SCT-013 clamp with burden resistor (current sensor)
- 3 level sensors (to put in pool chemical tanks)

![alt text](https://github.com/krissfr/PoolChemicalDispenser/blob/main/components.jpg)

For a unkown reason, i have to add a small diode (i use 1n4148) to lower a bit the 5v input voltage of the SSD1309 oled display.

