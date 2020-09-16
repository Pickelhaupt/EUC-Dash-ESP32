# EUC-Dash-ESP32
Bluetooth Dashboard for electric unicycles (EUCs) for ESP32, Arduino sketch

## Introduction
This is the first draft version of a dashboard for electric unicycles. It currently only supports KingSong 67V wheels and ttgo t-watch 2020. The interface is still very unpolished but usable.

I got a lot of help from reading the code from the cedbossneo/palachzzz /WheelLogAndroid project, it spared me from having to reverse engineer the protocol.


## Features
Reads BLE notifications from the electric unicycle and display data on the ESP32 display
### Data read and decoded
- Current speed
- Voltage
- Current
- Total Distance traveled
- Distance traveled since power on
- Time since power on
- Power
- Battery percentage remaining
- Speed alarm settings
- Tiltback (Max speed) setting
- Max speed since power on
- Cooling fan status
- Ride mode setting
- EUC temperature
### Data displayed 
- Current speed
- Voltage
- Current
- Max speed since power on
- Trip counter
- Odometer
- 3:rd alarm speed setting
- Tiltback speed setting
## Dependencies
### Required libraries:
- ESP32-BLE-Arduino
- TTGO T-Watch Library
- LVGL
## Building the project
It is easiest to build it using the Arduino IDE. If you are building for the T-Watch, make both the ESP and t-watch libraries are added
## Connecting to the wheel
It should connect automatically when it finds a compatible wheel. However there is currently no function implemented to make it possible to choose what wheel it will connect to of there are more than one compatible wheel in range, it will simply connect to the first one it finds. 
