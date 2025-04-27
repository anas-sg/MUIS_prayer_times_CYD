# MUIS_prayer_times_CYD

Display the local date, time and MUIS prayer times on an [ESP32 CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)

<iframe width="560" height="315" src="https://www.youtube-nocookie.com/embed/yT5R_Q9c7Bg?si=blqn8eUEw2HxQvJa" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" align="center" allowfullscreen></iframe>

## Installation

1. [Arduino IDE](https://www.arduino.cc/en/software/)
2. [ESP32 CYD](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md)
     - [ESP32 boards](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
     - [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)

## Background
My grandma is getting old and forgetful. Sometimes, she forgets the date and has trouble figuring out the prayer times from the calendar. So I decided to create a prayer times display for her. My original version uses an ESP8266 (Wemos D1 R1) that controls 6 7-segment displays (time and 5 prayers). This improved version uses the ESP32 CYD instead for convenience, since the display and ESP chip are integrated into a single module.

    
## Usage/Examples
1. Update `wifi_credentials.h` with your WiFi credentials
```c
#define _SSID    "YOUR_WIFI_SSID_HERE"
#define PASSWORD "YOUR_WIFI_PASSWORD_HERE"
```
2. Setup the board and libraries for CYD in Arduino IDE by following [this](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md)
3. Compile and flash (you might need to press and hold the `BOOT`button)

## Roadmap

- WiFi provisioning portal
- saving WiFi credentials to flash using `Preferences`
- script to build and flash using [arduino-cli](https://arduino.github.io/arduino-cli/1.2/)
- sprites to update display elements

## Licence

This work is published under the [MIT](https://choosealicense.com/licenses/mit/) License.
