## NVS
### Definition
Specifically for ESP-IDF

A specialized flash memory component used to store data persistently across device reboots and power cycles. 

It operates as a structured, flash-based key-value store rather than a traditional file system.
### A good analogy
If you're application doesn't need a file system, NVS gives you a way to store (small amounts of) configuration or other info needed across restarts/reboots/loss of power recovery.
[Non-Volatile Storage ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/v4.4.4/esp32/api-reference/storage/nvs_flash.html#introduction)

## Wi-Fi Station Example Project
### The example project itself
It shows how to make the ESP32 act like a normal Wi-Fi client
- Connections to an existing router/access point, gets an IP address, and prints connection status to the serial monitor
### What "station" means
**"Station mode"** or STA mode means
- The ESP32 connects to another Wi-Fi network
In other words
- The ESP32, like your laptop is joining your home Wi-Fi. Not creating its own Wi-Fi network
```text
Router / Access Point
        ↑
        |
     ESP32
```
#### The Station Example
Boot ESP32
Initialize storage
Initialize networking
Configure Wi-Fi SSID/password
Start Wi-Fi driver
Try to connect
Wait until connected or failed
Print result
### Files in the Station example
#### Important files
```
station/
├── CMakeLists.txt
├── README.md
├── main/
│   ├── CMakeLists.txt
│   ├── Kconfig.projbuild
│   └── station_example_main.c
```
#### Example Configuration `Kconfig.projbuild`
This creates Example Configuration menu 
- Sets Wi-Fi SSID, Wi-Fi password, retry count, WPA3 options, and minimum accepted Wi-Fi security mode
Usually configured with 
``` Bash
idf.py menuconfig
```
then 
``` 
Example Configuration
```
and set 
```
WiFi SSID
WiFi Passowrd
Maximum retry
WiFi Scan auth mode threshold
```
### What `station_example_main.c` does
There are three major pieces
```C
event_handler()
wifi_init_sta()
app_main()
```
The flow is 
```
app_main()
    └── initializes NVS
    └── sets log level if needed
    └── calls wifi_init_sta()

wifi_init_sta()
    └── initializes network stack
    └── creates default event loop
    └── creates Wi-Fi station interface
    └── initializes Wi-Fi driver
    └── registers event handler
    └── sets Wi-Fi mode to station
    └── sets SSID/password config
    └── starts Wi-Fi
    └── waits for connected/fail event bits

event_handler()
    └── reacts to Wi-Fi started
    └── reacts to disconnected
    └── reacts to got IP address
```

#### Include files
```C
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
```
`freertos/FreeRTOs.h` and `freertos/event_groups.h`
- ESP-IDF runs on FreeRTOS
- Example uses a FreeRTOS event group to wait until Wi-Fi either connects or fails
`esp_wifi.h`
- Gives access to Wi-Fi functions like
	```C
	esp_wifi_init()
	esp_wifi_set_mode()
	esp_wifi_set_config()
	esp_wifi_start()
	esp_wifi_connect()
	```
- `esp_wifi_init()` initializes the Wi-Fi driver, allocates Wi-Fi resources, and starts the Wi-Fi task
- [Wi-Fi documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_wifi.html) 
`esp_event.h`
- Used because Wi-Fi doesn't behave like simple blocking code
- Wi-Fi reports things through events.
- ESP-IDF's event loop library lets components register handlers that run when events occur, such as Wi-Fi connecting or receiving an IP address
- [Event Loop Library](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_event.html)
`nvs_flash.h`
- Wi-Fi stack uses NVS flash storage for persistent Wi-Fi/system data.
#### Wi-Fi configuration macros
Example lines like
```C
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY
```
These values are from `menuconfig`
- Instead of hardcoding it
ESP-IDF lets you configure it through 
`idf.py menuconfig → Example Configuration`
#### WPA3 and auth-mode settings
