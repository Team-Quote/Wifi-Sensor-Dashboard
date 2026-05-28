## Description
### Purpose
The purpose of this project is to design a compact Wi-Fi diagnostic appliance using an ESP32. The system monitors the quality of the local Wi-Fi connection and presents the result through a physical display and a browser-based dashboard.

Unlike previously implemented Internet Black Box Projects, which checks if the internet was usable. This project focuses on the quality and stability of the local Wi-Fi connection.
### What are we doing?
This project explores the design approach for the ESP32 Wi-Fi Diagnostic Dashboard. The approach uses the same basic hardware and organizes the software. The goal is to have a solid design and the best architecture.

## Design 
A layered modular architecture with separate modules for Wi-Fi management, monitoring, web server, display output, and shared application state.
### Core Hardware
- ESP32 development board
- External LCD/OLED display
	- TFT LCD Display Module Screen 2.0in 240x320
- USB power source
- Optional RGB/status LED
- Optional button for changing display pages
### Software Architecture
#### Top Level
`main.c`
- Application entry point
- Initializes NVS
- Initializes Wi-Fi
- Initializes shared app state
- Starts the HTTP server
- Starts monitor and display tasks
#### Storage Initialization Layer
`nvs_manager.c / nvs_manager.h`
- Initializes NVS flash during startup
- Handles NVS erase/retry behavior if initialization fails
- Provides a clean startup helper used by `main.c`
#### System Layer
`app_state.c / app_state.h`
- Stores current Wi-fi diagnostic state
- Provides safe access to shared data
- used by web server, monitor task, and display task
#### Wi-Fi Layer
`wifi_manager.c / wifi_manager.h`
- Initializes Wi-Fi station mode
- Connects to configured SSID
- Handles Wi-Fi events
- Tracks reconnect count
- Stores connection state
#### Monitoring Layer
`monitor.c / monitor.h`
- Samples RSSI
- Reads SSID, channel, IP address, and gateway
- Calls the signal processing layer to convert RSSI into quality percentage and signal label
- Updates uptime
- Updates shared application state
#### Signal Processing Layer
`signal_math.c / signal_math.h`
- Converts raw RSSI dBm values into a 0–100% signal quality score
- Assigns human-readable signal labels such as Excellent, Good, Fair, Weak, or Bad
- Clamps extreme RSSI values so the quality percentage never goes below 0% or above 100%
- Keeps Wi-Fi signal interpretation separate from the monitoring task
- Provides pure helper functions that can be unit tested without Wi-Fi hardware
#### Web Layer
`web_server.c / web_server.h`
- Hosts the main dashboard page
- Provides `/api/status`
- Later provides `/api/history`
- Later provides `/api/scan`
#### Display Layer
`display.c / display.h`
- Initializes physical LCD/OLED
- Draws current Wi-Fi status
- Shows RSSI, quality, SSID, IP address, and reconnect count
- Uses color/status labels for signal strength


## Module Table
| Module Name                       | Responsibility                                                               |
| --------------------------------- | ---------------------------------------------------------------------------- |
| `main.c`                          | Top-level integration, system initialization, task startup                   |
| `app_state.c / app_state.h`       | Shared diagnostic state used by all modules                                  |
| `wifi_manager.c / wifi_manager.h` | Wi-Fi initialization, event handling, reconnect tracking                     |
| `monitor.c / monitor.h`           | RSSI sampling, uptime tracking, IP/gateway reading, and shared state updates |
| `signal_math.c / signal_math.h`   | Converts RSSI dBm values into quality percentage and signal labels           |
| `web_server.c / web_server.h`     | Web dashboard, `/api/status`, future API endpoints                           |
| `display.c / display.h`           | Physical screen initialization and status rendering, TFT output              |
| `config.h`                        | Wi-Fi credentials, sampling intervals, feature flags                         |
