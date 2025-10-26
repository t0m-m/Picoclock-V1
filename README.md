# Picoclock-V1
Simple little desktop clock based on Raspberry Pi Pico on custom PCB
# Information
This version of the clock is only made for testing purposes. The final version will feature vintage VQB-71 7 segment displays. However everything else apart from the displays is the same and will be featured in the final version. 
On board is DS3231 RTC chip, MAX7221 that is used as display driver as well as a buzzer and 4 buttons for controls, such as brightness, confirm, cancel and mode. 

There is no on-board battery, so the unit must be connected to external power supply using Pico's micro USB port. The final version may feature Li-on pack however.
# Implemented features
* Time Keeping
* Date Mode
* Alarm Mode
# Hardware
I wanted the board to be 2 layers, but during the designing, i added 2 more layers for better routing of the signals. The unit is also equipped with 4 3M holes to attach a plexi sheet or to mount the unit.

<img width="1408" height="1026" alt="image" src="https://github.com/user-attachments/assets/c7c8651d-e4ba-4494-9a38-49d9c36d8359" />


