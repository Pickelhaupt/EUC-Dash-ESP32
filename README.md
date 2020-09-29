# EUC-Dash-ESP32
Stand-alone Bluetooth dashboard for electric unicycles (EUCs) for ESP32, Arduino sketch

## Introduction
Update new version in EUCDash-test, moved the old version to archive - code rewritten and a number of features added:
- New dashboard
- Display clock when not connected to wheel
- Power saving enabled
- Accelerometer event wake up
- Time based backlight brightness config

This is the first draft version of a dashboard for electric unicycles. It currently only supports KingSong 67V wheels and ttgo t-watch 2020. The code is still very unpolished and probably buggy.

I got a lot of help from reading the code from the WheelLogAndroid project, it spared me from having to reverse engineer the protocol.


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
- Battery level
- Current
- Temperature
- Current time
- Time and date when disconnected
### Supported Models
Only supports Kingsong wheels at the moment. Might work with Gotway as well since the protocols are very similar.
## Screenshot
First version of the new design, there are still some more things to add.
<div>
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/ride-crop-small.jpg" width="30%" align="left"/>
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/watch-crop-small.jpg" width="30%" align="center"/>
</div>

## Dependencies
### Required libraries:
- ESP32-BLE-Arduino
- TTGO T-Watch Library
- LVGL (The lvgl library that comes with the TTGO T-Watch Library needs to be updated to ver 7.5+)
## Bugs and issues
- Sometimes hangs and require a restart
- Does not properly remove tasks when switching modes
- Backlight adjustment does not work at the moment
## Todo
- Add haptic feedback to alarms
- Add missing dashboard objects
  - <strikethrough> Max/Avg/Min bars on arcs </strikethrough>
  - Trip distance
  - Ride time
  - Alert icons and red corners
- Add multiple screens (2 dashboard detail levels)
- Add info screen
- Add settings sreen and settings
- Make alarms wake up screens
- Make alarms more visible
- Add persistent settings storage
- Add gesture controls of light and horn
- Support for more wheel models
- Autodetection of wheel make and model

Below are mockups of the planned end state for version 1.0

All dashboard functions explained:

<img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/Dashboard-manual-small.png" width="70%" align="center"/>

Set of screens in v1.0:

<img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/Tileview-screens-small.png" width="70%" align="center"/>

## Building the project
It is easiest to build it using the Arduino IDE. If you are building for the T-Watch, make sure both the ESP and t-watch libraries are added. The lvgl library included in the TTGO-Twatch-Library might need to be upgraded. If you don't gave Git, download the ZIP and extract the files. All source files are currently in the EUCDash-test directory. Open EUCDash-test.ino with the Arduino IDE and is should compile and upload if you have all the required libraries installed.
## Connecting to the wheel
It should connect automatically when it finds a compatible wheel when the screen is on, it will not connect when screen is off as the device is in sleep mode. However there is currently no function implemented to make it possible to choose what wheel it will connect to of there are more than one compatible wheel in range, it will simply connect to the first one it finds.
