# EUC-Dash-ESP32
Bluetooth Dashboard for electric unicycles (EUCs) for ESP32, Arduino sketch

## Introduction
Update - code rewritten and a number of features added:
- New dashboard
- Display clock when not connected to wheel
- Power saving enabled
- Accelerometer event wake up

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
- Add multiple screens (2 dashboard detail levels)
- Add info screen
- Add settings sreen and settings
- Make alarms wake up screens
- Make alarms more visible
- Add persistent settings storage
- Add gesture controls of light ans horn
## Building the project
It is easiest to build it using the Arduino IDE. If you are building for the T-Watch, make sure both the ESP and t-watch libraries are added. The lvgl condiguration file needs to be edited to enable some of the font sizes used in this version of the interface.
## Connecting to the wheel
It should connect automatically when it finds a compatible wheel. However there is currently no function implemented to make it possible to choose what wheel it will connect to of there are more than one compatible wheel in range, it will simply connect to the first one it finds. Make sure the wheel is on before switching on the ESP device.
