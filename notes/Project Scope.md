# Another Black Box: ESP32 Wi-Fi Diagnostic Dashboard
**Main Question: How good is my local Wi-Fi connection right here right now?** 

## Core idea
Build a small ESP32-based diagnostic dashboard that connects to Wi-Fi, measures connection quality, and displays that information on both:
1. A physical display
2. A browser-based dashboard served by the ESP32

Version 1 focuses on current Wi-Fi signal quality, connection status, reconnect behavior, and live dashboard/display output. Later versions will add RSSI history and nearby network scanning.

## What this project should do
### Required features
These are the features for Version 1

| Feature                   | Description                              |
| ------------------------- | ---------------------------------------- |
| Wi-Fi connection status   | Connected, connecting, disconnected      |
| SSID                      | Name of currently connected Wi-Fi        |
| RSSI                      | Signal strength in dBm                   |
| Signal Quality percentage | Human-readable quality score             |
| Channel                   | Wi-Fi channel currently used             |
| IP address                | ESP32's local IP address                 |
| Gateway address           | Router/gateway IP                        |
| Uptime                    | How long the ESP32 has been running      |
| Reconnect count           | How many times Wi-Fi dropped/reconnected |
| Web dashboard             | Browser page served by the ESP32         |
| JSON status endpoint      | `/api/status` returns current status     |
| Physical status screen    | Local display with key Wi-Fi info        |
### Future Features

| Feature             | Version | Description                               |
| ------------------- | ------- | ----------------------------------------- |
| RSSI history graph  | V2      | Small graph on web dashboard              |
| Nearby network scan | V3      | Manual scan button, not constant scanning |
| Status LED          | V4      | LED color reflects signal state           |
| mDNS hostname       | V4      | Access dashboard using a local hostname   |
| Enclosure           | V4      | Make the device feel like an appliance    |
## Project Objective
The objective of this project is to design and implement a compact Wi-Fi diagnostic appliance using an ESP32. The system connects to a configured Wi-Fi network, monitors signal strength, tracks connection stability, and presents the results through both a physical display and a browser-based dashboard hosted directly on the ESP32.

The project demonstrates embedded networking, Wi-Fi station mode, HTTP server design, JSON API endpoints, real-time data polling, RSSI measurement, FreeRTOS task design, embedded UI design, and practical home network debugging.

Unlike a general internet-health monitor, this project focuses specifically on local Wi-Fi quality and connection behavior. It is intended to help identify weak signal areas, unstable Wi-Fi coverage, and crowded wireless environments inside a home.

## Main User Question
**Wtf are we showing to the user?**

1. Am I connected to Wi-Fi?
2. How strong is the signal?
3. Is the signal stable?
4. Is this room a good Wi-Fi location?
5. Could nearby networks be crowding the area? Later versions will answer this with manual scan results.

## MVP Definition
### Minimum Viable Product
The MVP is complete when:
1. The ESP32 connects to a hardcoded Wi-Fi network.
2. The ESP32 reads current Wi-Fi RSSI.
3. The ESP32 calculates a signal quality percentage.
4. The ESP32 displays Wi-Fi status on the physical screen.
5. The ESP32 hosts a web dashboard.
6. The dashboard shows SSID, RSSI, quality, channel, IP address, uptime, and reconnect count.
7. The dashboard updates without reflashing the device.
8. The ESP32 exposes a JSON endpoint at /api/status.

## System Behavior
The system starts automatically when the ESP32 is powered on. A serial connection may be used during development for debugging, flashing, and monitoring logs, but it is not required for normal operation.

After boot, the ESP32 initializes Wi-Fi, connects to the configured network, starts a local HTTP server, begins sampling Wi-Fi signal data, and updates the physical display with the current network status.
### System outline/System Startup Flow
1. Power on ESP32.
2. Initialize NVS flash.
3. Initialize Wi-Fi in station mode.
4. Connect to configured Wi-Fi network. 
5. Obtain IP address from router.
6. Start HTTP web server. 
7. Start Wi-Fi monitor task.
8. Start display update task.
9. Periodically sample RSSI and connection status.
10. Update local display and web dashboard data.

## Software Architecture
### Sample Software modules
`main.c`
Application entry point (Top-Level). Initializes system services and starts tasks
`wifi_manager.c`
Handles Wi-Fi initialization, connection events, reconnect counting, and current Wi-Fi state.
`monitor.c`
Samples RSSI, channel, IP information, and uptime, then updates shared diagnostic state.
`web_server.c`
Hosts the web dashboard and JSON API endpoints.
`display.c`
Updates the physical LCD or OLED display.
`LED_status.c`
Controls optional status LED behavior
`app_state.c`
Stores shared system state used by the display and web server.

### Data Model
The system stores the current diagnostic information in one shared application state structure. This structure is updated by the monitor task and read by the webserver and display task.
#### Example state fields

| Field             | Description                                       |
| ----------------- | ------------------------------------------------- |
| `wifi_connected`  | Whether the ESP32 is currently connected to Wi-Fi |
| `ssid`            | Current Wi-Fi network name                        |
| `rssi`            | Current signal strength in dBm                    |
| `quality`         | Signal quality from 0% to 100%                    |
| `signal_label`    | Excellent, Good, Fair, Weak, or Bad               |
| `channel`         | Wi-Fi channel currently used                      |
| `ip_addr`         | ESP32 local IP address                            |
| `gateway_addr`    | Router/gateway IP address                         |
| `uptime_sec`      | Time since boot                                   |
| `reconnect_count` | Number of Wi-Fi reconnect events                  |
| `total_samples`   | Number of RSSI samples collected                  |

#### Example C structure:
```C
typedef struct {
    bool wifi_connected;
    char ssid[33];
    int rssi;
    int quality;
    char signal_label[16];
    uint8_t channel;
    char ip_addr[16];
    char gateway_addr[16];
    uint32_t uptime_sec;
    uint32_t reconnect_count;
    uint32_t total_samples;
} wifi_diag_status_t;
```
Note: Version 2 will have `int rssi_history[120]` but for now we just need the basic dashboard to be properly working
### API Endpoints
#### Version 1 Endpoints

| Endpoint      | Method | Description                                   |
| ------------- | ------ | --------------------------------------------- |
| `/`           | GET    | Returns the main web dashboard                |
| `/api/status` | GET    | Returns current Wi-Fi diagnostic data as JSON |

#### Future Endpoints

| Endpoint       | Method | Version | Description                         |
| -------------- | ------ | ------- | ----------------------------------- |
| `/api/history` | GET    | V2      | Returns recent RSSI history samples |
| `/api/scan`    | GET    | V3      | Returns nearby Wi-Fi scan results   |
| `/api/reboot`  | POST   | V4      | Optional endpoint to reboot device  |
#### Example `/api/status` response
```JSON
{
  "wifi_connected": true,
  "ssid": "HomeWiFi",
  "rssi": -62,
  "quality": 76,
  "signal_label": "Good",
  "channel": 6,
  "ip_addr": "192.168.1.52",
  "gateway_addr": "192.168.1.1",
  "uptime_sec": 3821,
  "reconnect_count": 2
}
```

### Web Dashboard Layout
The web dashboard is served directly by the ESP32 and can be opened from a browser on the same local network.
#### Version 1 Dashboard Cards

| Card              | Information Shown                                |
| ----------------- | ------------------------------------------------ |
| Connection Status | Connected/disconnected state, SSID, signal label |
| Signal Quality    | RSSI IN dBm, quality percentage, Wi-Fi channel   |
| Network Info      | IP address, gateway address, uptime              |
| Stability         | Reconnect count and last known event             |
#### Future Dashboard Cards

| Card            | Version | Information Shown                                      |
| --------------- | ------- | ------------------------------------------------------ |
| RSSI History    | V2      | Graph of recent RSSI samples                           |
| Nearby Networks | V3      | Manual scan results with SSID, RSSI, channel, security |
#### Basic Layout
```
+--------------------------------+
| Another Black Box              |
| Wi-Fi Diagnostic Dashboard     |
+--------------------------------+

[ Connection Status ]
SSID: HomeWiFi
Status: Connected

[ Signal Quality ]
RSSI: -62 dBm
Quality: Good, 76%
Channel: 6

[ Network Info ]
IP: 192.168.1.52
Gateway: 192.168.1.1
Uptime: 01:03:41

[ Stability ]
Reconnects: 2
```

## Hardware/Physical Architecture
### Core hardware
The project uses an ESP32-based development board as the main controller. The ESP32 connects to the local Wi-Fi network, monitors signal quality, hosts the web dashboard, and updates the physical display.

| Component        | Purpose                                |
| ---------------- | -------------------------------------- |
| ESP32 board      | Main processor  and Wi-Fi interface    |
| LCD/OLED display | Local status output                    |
| USB power        | Powers the diagnostic appliance        |
| Optional RGB LED | Quick visual status indicator          |
| Optional Button  | Changes display pages or triggers scan |

### Physical Display Layout
The physical display should show the most important information without needing a browser.
#### Example layout
```text
ANOTHER BLACK BOX  
  
Wi-Fi: CONNECTED  
SSID: HomeWiFi  
  
RSSI: -62 dBm  
Quality: GOOD 76%  
  
IP: 192.168.1.52  
Ch: 6  
  
Drops: 2  
Up: 01:03:41
```
#### Status Colors

| State          | Display/LED Color |
| -------------- | ----------------- |
| Booting        | Blue              |
| Connecting     | Amber             |
| Excellent/Good | Green             |
| Fair           | Yellow/Amber      |
| Weak/Bad       | Red               |
| Disconnected   | Dim white or red  |

## Development Phases
### Phase 1: Wi-Fi Connection and Serial Output
**Goal:** Prove that the ESP32 can connect to Wi-Fi and read signal strengths
#### Tasks
1. Create ESP-IDF project
2. Hardcode Wi-Fi SSID and password
3. Connect to Wi-Fi in station mode
4. Print assigned IP address
5. Read current RSSI
6. Convert RSSI to quality percentage
7. Print values over serial once per second
#### Ideal Complete Phase 1
```text
Connected to HomeWiFi  
IP: 192.168.1.52  
RSSI: -62 dBm  
Quality: 76%
```
### Phase 2: Shared Application State
**Goal:** Create a clean internal data model
#### Tasks
1. `wifi_diag_status_t`
2. Store Wi-Fi status, SSID, RSSI, quality, channel, IP, gateway, uptime, and reconnect count
3. Create helper functions for reading and updating state
4. Make monitor task update this state periodically
#### Ideal Complete Phase 2
All important Wi-Fi diagnostic data is stored in one shared structure
### Phase 3: Web API
**Goal:** Expose current diagnostic data through the ESP32
#### Tasks
1. Start ESP-IDF HTTP server
2. Create /api/status
3. Return JSON status data
4. Test from browser
#### Ideal Complete Phase 3
Finished when 
`http://esp32-ip-address/api/status`
### Phase 4: Web Dashboard
**Goal:** Make the data readable from a browser
#### Tasks
1. Serve HTML dashboard at `/`
2. Use JavaScript `fetch()` to call `/api/status`
3. Update dashboard once per second
4. Display connection, signal, network, and stability cards
#### Ideal Complete Phase 4
Browser dashboard updates live without reflashing the ESP32
### Phase 5: Physical Display
**Goal:** Make the device useful without a computer or phone
#### Tasks
1. Initialize LCD/OLED display
2. Draw title and connection state
3. Display SSID, RSSI, quality, IP address, channel, uptime, and reconnect count
4. Refresh display once per second
#### Ideal Complete Phase 5
The ESP32 can be powered from USB and used as a standalone Wi-Fi monitor
### Phase 6: RSSI History
**Goal:** Show whether the signal is changing over time.
#### Tasks
1. Store recent RSSI samples in a ring buffer
2. Add `/api/history`
3. Draw a simple graph on the web dashboard
#### Ideal Complete Phase 6
Moving the ESP32 around causes visible changes in the RSSI graph
### Phase 7: Manual Nearby Network Scan
**Goal:** Show nearby Wi-Fi networks and possible channel crowding
#### Tasks
1. Add dashboard scan button
2. Add `/api/scan`
3. Run scan only when requested
4. Return SSID, RSSI, channel, and security type
5. Display results in a table
#### Ideal Complete Phase 7
User can manually scan and view nearby networks from the dashboard
### Phase 8: Polish
**Goal:** Make the project feel like a complete appliance
#### Tasks
1. Add status LED behavior 
2. Add mDNS hostname
3. Improve display layout
4. Improve dashboard styling
5. Add README screenshots
6. Add final demo video
7. Add final demo report IEEE style