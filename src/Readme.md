# PlatformIO version
Completely rewritten using the My-TTGO-Watch as template, still only works with ttgo-t-watch and Kingsong wheels

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
  
