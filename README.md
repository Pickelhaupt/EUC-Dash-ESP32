# EUC-Dash-ESP32

**2020-12-04 added wheel settings**

**2020-12-03 Gotway/Veteran support in progress**

**2020-11-24 Added KS16X/XS, KS18L/XL and KS-S18**

**2020-11-24 Autodetection of KS wheels has been implemented, at the moment only KS14M/D/S, KS16/KS16S and KS18A/S are detected**


Complete rewrite added, only buildable with PlatformIO please see https://github.com/Pickelhaupt/EUC-Dash-ESP32/tree/master/src for more info

Stand-alone Bluetooth dashboard for electric unicycles (EUCs) for ESP32.
You can build it with both the Arduino IDE and with PlatformIO.
For instructions on how to setup PlatformIO and building the project see the README.md file in the src directory.

## Introduction
The latest version will no longer build using the Arduino IDE, I have migrated to PlatformIO and will not update the arduino versions.

This pre-release version of a dashboard for electric unicycles. It currently only supports KingSong wheels and ttgo t-watch 2020. There are probably still some bugs. 

I got a lot of help from reading the code from the WheelLogAndroid project, it spared me from having to reverse engineer the protocol.
I used My-TTGO-Watch by Dirk Brosswick as a template to implement multiple screen support, settings and a lot of other functions:
https://github.com/sharandac/My-TTGO-Watch


## Features
Reads BLE notifications from the electric unicycle and display data on the ESP32 display
- Reads KS BLE notifications
- Full and simple dashboard
- Clock display
- Power saving implemented, will go to sleep when wheel is disconnected and display is off. Not optimised when connected yet. Currently the battery lasts for:
  - Around 3 days not connected
  - Around 6 hours when continuously connected
  - Around 1 day of standby and 4-5 hours of continuous riding
- Wake up from accelerometer, button and double tap
- long press on dashboard to toggle lights
- Settings screen:
  - clock
  - dashboard
  - BLE
  - system utilities
  - battery and power
  - display (contrast, wake time, rotation)
  - OTA upgrades
  - wifi

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
### Data displayed on dashboards
- Current speed 
- Max speed
- Wheel battery level
- Wheel max battery
- Wheel min battery
- Current
- Max current
- Max regen breaking current
- Temperature -- full dash only
- Max temperature -- full dash only
- Trip meter -- full dash only
- Current time -- full dash only
- Watch battery level -- full dash only

Dashboard functions:
<div> 
<img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/dashboards-s.png" width="70%" align="center"/>
</div>

Dashboard alerts:
<div> 
<img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/alerts-note-s.png" width="60%" align="center"/>
</div>

### Other screens
- Time and date screen
- Settings screen
- trip info screen
- wheel info screen

Screen layout:
<div> 
<img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/maintile-s.png" width="80%" align="center"/>
</div>

Map of all settings screens:
<div> 
<img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/settingsmanual2s.png" width="100%" align="center"/>
</div>

### Supported Models
Only supports Kingsong wheels at the moment. Might work with Gotway as well since the protocols are very similar.
## Screenshots
Some new screenshots, colours are a bit off though
<div> 
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/20201114_005122-fix_proc.jpg" width="20%" align="left"/>
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/20201114_005212-fix_proc.jpg" width="20%" align="center"/>
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/20201114_005143-fix_proc.jpg" width="20%" align="left"/>
  
  <img src="https://github.com/Pickelhaupt/EUC-Dash-ESP32/raw/master/Images/20201114_005231-fix_proc.jpg" width="20%" align="center"/>
</div>

## Dependencies
### Required libraries:
Libraries are downloaded automatically by PlatformIO when compiling for the first time.
- TTGO T-Watch Library; modified version with LVGL 7.6
- AsyncTCP@>=1.1.1
- ArduinoJson@>=6.15.2
- PubSubClient@>=2.8

## Bugs and issues
- I have not verified if the http upgrade feature works, also the published firmware is out of date.

## Todo
- Support for more wheel brands (Gotway/Veteran, Inmotion, Ninebot planned)
- Autodetection of wheel make and model --done, only detects KS at this time

## Building the project
Requires PlatformIO and driver for the USB chip in the t-watch. for more info see: 
https://github.com/Pickelhaupt/EUC-Dash-ESP32/tree/master/src
## Connecting to the wheel
It should connect automatically when it finds a compatible wheel when the screen is on, it will not connect when screen is off as the device is in sleep mode. However there is currently no function implemented to make it possible to choose what wheel it will connect to of there are more than one compatible wheel in range, it will simply connect to the first one it finds. Also it will not autodetect the wheel model. Edit the wheelmodel string in Kingsong.cpp (initks function at the bottom of the file)
