## Description
A compact, standalone Wi-Fi diagnostic appliance designed using the DOIT ESP32-DevKit-V1 platform under ESP-IDF V6.0. The system actively samples the local wireless environment to determine immediate connection quality, tracking network stability metrics and exposing them simultaneously across a physical TFT display and an embedded local web server
## Hardware Requirements
- DOIT ESP32-DevKit-V1 (Xtensa Dual-core LX6, 4MB Flash, 520KB SRAM)
- 1 TFT LCD Display Module Screen 2.0in 240x320 Resolution (SPI-driven, utilizing ST7789 or ILI9341 controller circuitry)
## System Requirements
- [ ] **RQ-01 NVS Initialization**
	- Non-Volatile Storage (NVS) flash must initialize successfully at startup to retain internal Wi-Fi calibrations and parameters.
- [ ] **RQ-02 FreeRTOS Task Scheduling**
	- Monitor Task: Handles timing-dependent telemetry sampling.
	- Display Task: Schedules frame rendering onto the SPI bus.
	- HTTP Daemon Task: Asynchronously listens for and dispatches incoming network sockets.
- [ ] **RQ-03 Thread-Safe Shared memory**
	- The structural entry `wifi_diag_status_t` must use FreeRTOS Mutex primitives (`SemaphoreHandle_t`) during read/write cycles to prevent data race conditions between the core monitoring loop, the screen engine, and the web framework.
- [ ] **RQ-04 Wi-Fi Protocol Selection**
	- Protocol stack initialized explicitly in Station Mode (STA), restricting connectivity parameters to 2.4 GHz IEEE 802.11b/g/n channels.
- [ ] **RQ-05 ESP-IDF v6.0 Component Decoupling**
	- Any hardware features or abstraction module built inside custom components must link its dependencies dynamically using modular targets (*e.g.,* `esp_driver_gpio` or `esp_lcd`) inside local component `CMakeLists.txt` structures.
## Functional Requirements
- [ ] **RQ-06 API Serialization Engine**
	- An HTTP endpoint listening at `/api/status` must serialize the active `wifi_diag_status_t` structure into a valid JSON object response.
- [ ] **RQ-07 Uptime Tracking**
	- Internal timers must calculate and display absolute active runtime since boot up, incrementing continuously in seconds.
- [ ] **RQ-08 Telemetry Sampling Rate**
	- The monitoring thread must poll the Wi-Fi subsystem once every 1.0 second to collect a fresh Received Signal Strength Indication (RSSI) value.
- [ ] **RQ-09 Quality Scaling Algorithm**
	- Raw dBm values must pass through a normalization function to bound the telemetry between 0% (extremely poor/disconnected) and 100% (ideal saturation).
- [ ] **RQ-10 Event-Driven Drop Monitoring**
	- Subscribes to the system event loop via an asynchronous callback handle (`WIFI_EVENT_STA_DISCONNECTED`) to immediately flag connection breaks, execute background retries, and step up the global reconnection counter.

## Constraints
- [ ] **CN-01 Frequency Band Hard Constraint**
	- The network module is restricted to 2.4 GHz Wi-Fi profiles; the application cannot scan, associate with, or track metrics across 5GHz or 6 GHz frequency spectra.
- [ ] **CN-02 Local Network Access Boundary**
	- The dashboard is intended for local LAN use only. The project will not implement cloud access, port forwarding, external DNS exposure, or remote authentication. Testing will verify access from a same-subnet client and document that WAN access is outside project scope.
- [ ] **CN-03 Monolithic Web-Server Limits**
	- The embedded HTTP server is lightweight and resource-limited. It will serve simple static HTML and JSON only, avoid heavy external assets, use bounded socket/session limits, and rely on application-level synchronization when sharing data with other tasks.
- [ ] **CN-04 Modular Component Drivers**
	- The build system will reject legacy monolithic driver descriptors; all internal source components must declare exact hardware sub-element requirements explicitly via `PRIV_REQUIRES` or `REQUIRES` in their CMake parameters.