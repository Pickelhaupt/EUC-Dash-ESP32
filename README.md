# EUC-Dash-ESP32
Complete rewrite added, only buildable with PlatformIO please see https://github.com/Pickelhaupt/EUC-Dash-ESP32/tree/master/src for more info



Stand-alone Bluetooth dashboard for electric unicycles (EUCs) for ESP32.
You can build it with both the Arduino IDE and with PlatformIO.
For instructions on how to setup PlatformIO and building the project see the README.md file in the src directory.

## Introduction
There are now two versions, one stable(ish) and one for development, I will add new features to the development version and merge when it is stable. At the moment I would recommend using the dev version as the stable version is a bit out of date.

This is early version of a dashboard for electric unicycles. It currently only supports KingSong 67V wheels and ttgo t-watch 2020. The code is still quite unpolished and there are probably some bugs. 
Even though there are still a lot of functions missing, the dashboard now works quite well.

I got a lot of help from reading the code from the WheelLogAndroid project, it spared me from having to reverse engineer the protocol.


## Features
Reads BLE notifications from the electric unicycle and display data on the ESP32 display
- Reads KS BLE notifications
- Full and simple dashboard
- Clock display when not connected to wheel
- Power saving implemented, will go to sleep when wheel is disconnected and display is off. Not optimised when connected yet. Currently the battery lasts for:
  - Around 3 days not connected
  - Around 6 hours when continuously connected
  - Around 1 day of standby and 4-5 hours of continuous riding
- Wake up from accelerometer, button and double tap

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
- Max speed
- Wheel battery level
- Wheel max battery
- Wheel min battery
- Current
- Max current
- Temperature
- Max temperature
- Trip meter
- Current time
- Watch battery level
- Time and date when disconnected
### Supported Models
Only supports Kingsong wheels at the moment. Might work with Gotway as well since the protocols are very similar.
## Screenshot
Version 0.4, getting close, there are still some more things to add.
<div>
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/fulldash-0.4-small.jpg" width="30%" align="left"/>
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/clock-0.4-small.jpg" width="30%" align="center"/>
</div>

## Dependencies
### Required libraries:
- ESP32-BLE-Arduino
- TTGO T-Watch Library
- LVGL (The lvgl library that comes with the TTGO T-Watch Library needs to be updated to ver 7.5+)
## Bugs and issues

## Todo
- Add haptic feedback to alarms
- Add missing dashboard objects
  - Ride time
  - Alert icons and red corners
- Add simple dashboard
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
