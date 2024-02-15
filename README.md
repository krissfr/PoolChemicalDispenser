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

![alt text](https://github.com/krissfr/PoolChemicalDispenser/blob/main/components.jpg)

For a unkown reason, i have to add a small diode (i use 1n4148) to lower a bit the 5v input voltage of the SSD1309 oled display.


## roadmap :
- a sensor for the main circulation pump (to inject chemical product only when filtration is ON). We can use the X input plug of the DLC32 wich is an analog input. i will probably a clamp current sensor.
- 3 sensors to detect when the chemical product cans are low (1 for each). We can use Probe input, Z input and Y input plugs of the DLC32 board. i will probably use contact less liquid sensors.
