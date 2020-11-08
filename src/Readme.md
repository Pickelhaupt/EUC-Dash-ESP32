# PlatformIO version
Completely rewritten using the My-TTGO-Watch as template, still only works with ttgo-t-watch and Kingsong wheels
## Current features
- Tile based multiple screens 
- Clock display
- Simple Dashboard
- Full dashboard
- wifi NTP clock sync with timezone support
- Kingsong EUC support
- OTA updates on wifi (no binary available yet)
- Settings storage on flash
- settings for:
  - clock
  - dashboard
  - BLE
  - system utilities
  - battery and power
  - display (contrast, wake time, rotation)
  - OTA upgrades
  - wifi
## In progress:
- Adding wheel settings 
- Fix non working haptic feedback
## Next steps
- Add gesture to turn off lights
- Add info screens with trip data and wheel info
## Still todo
- Add support for more wheel models
- Add more advanced settings options
- Add flash based settings storage
- Add wheel model auto detection
- Add wheel selection setting
- Add missing dash elements (ride mode and alert indicators)

This version requires the TTGO_TWatch_Library, this will automatically be downloaded when building the project the first time.
I have forked the Twatch library and upgraded the LVGL version, the forked version is the one that is downloaded when building the project.
## Setting up PlatformIO (on windows VScode)
  - install Visual Studio Code
  - Install Git
  - Add the PlatformIO extension:
    - Click the extension icon on the left hand bar, 
    - search for PlatformIO
    - Hit install
  - Wait for PlatformIO to install completely, this might take some time.
  - Restart VSCode
  - If you don't have a working Arduino IDE set up you will probably have to install the CP210x USB UART drivers for the watch: https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers
  - Click the Source control icon on the left bar
  - Choose clone and enter https://github.com/Pickelhaupt/EUC-Dash-ESP32.git
  - Click on clone from URL
  - Once the repository has been cloned you can build it by hitting the tick sign on the bottom bar
  - To upload to the watch, make sure it is connected to USB and click the arrow icon on the bottom bar
  
