## Description
### Project 
**Another Black Box: ESP32 Wi-Fi Diagnostic Dashboard**
### Purpose
To implement and test modules independently before system integration to ensure thread safety, correct hardware initialization, and accurate data parsing
### Architecture Requirement
**Drivers**
Register-level hardware control (SPI,GPIO, NVS).
**Modules**
Feature/application logic (Wi-Fi STA connection, HTTP Server, Signal Math, Thread-Safe Data)
### Test Criteria
1. Implement hardware control in `components/drivers`.
2. Implement application logic in `components/features`.
3. Create Unity test files (e.g, `test_driver.c`) inside a `test/` subdirectory for each component
4. Flash and monitor the project using the ESP-IDF unit test application build process to run isolated tests.
5. Each core feature/driver must have at least 2 normal `TEST_CASE` blocks and 1 edge case `TEST_CASE` block.
6. Document test evidence (terminal logs of unity passing `PASS` outputs, or photos for hardware states) in the project repository.

## Module-Level Traceability Table
| **Requirement / Constraint**            | **Responsible Module**             | **Test Case** | **Expected Module Test Evidence**                                                                                           |
| --------------------------------------- | ---------------------------------- | ------------- | --------------------------------------------------------------------------------------------------------------------------- |
| **RQ-01:** NVS Initialization           | Hardware Driver: `nvs_manager.c`   | TC-NVS-01     | Terminal log showing successful `nvs_flash_init()`                                                                          |
| **RQ-02:** FreeRTOS Task Scheduling     | App Logic: `main.c`                | TC-RTOS-01    | Unity test confirming `xTaskCreate` returns `pdPASS` for 3 core tasks                                                       |
| **RQ-03:** Thread-Safe Memory           | App Logic: `app_state.c`           | TC-DATA-01    | Console log showing successful Mutex Take/Give cycles                                                                       |
| **RQ-04:** Wi-Fi Protocol Selection     | Service Module: `wifi_manager.c`   | TC-WIFI-01    | Terminal log showing IP assignment from AP on 2.4GHz                                                                        |
| **RQ-05:** ESP-IDF Component Decoupling | Build System: `CMakeLists.txt`     | TC-BUILD-01   | Clean compilation log showing successful dynamic linking                                                                    |
| **RQ-06:** API Serialization Engine     | App Logic: `web_server.c`          | TC-WEB-01     | Postman/cURL screenshot of valid JSON output from `/api/status`                                                             |
| **RQ-07:** Uptime Tracking              | App Logic: `monitor.c/app_state.c` | TC-DATA-02    | Unity test confirming uptime integer increments by 1 after 1000ms                                                           |
| **RQ-08:** Telemetry Sampling Rate      | Service Module: `monitor.c`        | TC-WIFI-02    | Terminal log printing real-time raw RSSI dBm values at 1Hz                                                                  |
| **RQ-09:** Quality Scaling Alg.         | App Logic: `signal_math.c`         | TC-MATH-01    | Console output verifying -50 dBm converts to 100%                                                                           |
| **RQ-10:** Event-Driven Drop Monitoring | Service Module: `wifi_manager.c`   | TC-WIFI-03    | Log showing connection break flag and retry increment                                                                       |
| **CN-01:** Frequency Band Constraint    | Service Module: `wifi_manager.c`   | TC-WIFI-04    | Unity assertion confirming STA mode is active and connected AP channel is within the 2.4 GHz channel range                  |
| **CN-02:** Subnet Isolation Boundary    | App Logic: `web_server.c`          | TC-WEB-02     | Test confirming `/` and `/api/status` are reachable from a same-LAN client; WAN/cloud access is documented as outside scope |
| **CN-03:** Web-Server Limits            | App Logic: `web_server.c`          | TC-WEB-03     | Terminal log showing server gracefully handling rapid refresh limits                                                        |
| **CN-04:** Modular Component Drivers    | Build System: `CMakeLists.txt`     | TC-BUILD-02   | `CMakeLists.txt` file inspection showing `REQUIRES` parameter usage                                                         |
## Module Test Results

### Normal Test Cases

#### TC-NVS-01 – NVS Flash Initialization

`nvs_manager.c` will be tested by calling the initialization routine on boot. The Unity framework will assert that the returned `esp_err_t` is `ESP_OK`.

#### TC-RTOS-01 – FreeRTOS Task Instantiation

The task manager will be tested by invoking the initialization functions for the Monitor, Display, and HTTP tasks. The Unity framework will check that `xTaskCreate` returns `pdPASS` for all three, proving the scheduler successfully allocated memory for them.

#### TC-DATA-01 – Mutex Lock/Unlock Verification

`app_state.c` will be tested by spanning two temporary FreeRTOS tasks. Task A will take the mutex and write a dummy RSSI value; Task B will take the mutex and read it. Success is verified by terminal logs showing no race conditions or corrupted values.

#### TC-DATA-02 – Uptime Increment Verification

`data_model.c` internal timers will be tested by taking a baseline read of the system uptime, utilizing `vTaskDelay` to wait exactly 1000 ticks (1 second), and verifying the internal struct has incremented the uptime value strictly by 1.

#### TC-WIFI-01 – Station Mode Connection

`wifi_manager.c` will be tested by hardcoding local SSID and Password credentials. Success is verified by the ESP32 triggering the `WIFI_EVENT_STA_CONNECTED` and `IP_EVENT_STA_GOT_IP` event loops, and successfully asserting an assigned local IP address.

#### TC-WIFI-02 – Raw RSSI Extraction

`wifi_manager.c` will be tested by calling `esp_wifi_sta_get_ap_info()`. Success is verified by Unity asserting the return of a negative integer (dBm) between -10 and -100, proving the radio is actively sampling the environment.

#### TC-MATH-01 – Signal Quality Normalization

`signal_math.c` will be tested by passing an array of RSSI values (e.g., -50, -70, -90) through the scaling algorithm. The Unity test will strictly assert the expected integer outputs (e.g., 100%, 50%, 0%) to prove the math logic functions independently.

#### TC-WEB-01 – JSON API Serialization

`web_server.c` will be tested using a dummy `wifi_diag_status_t` struct populated with mock data. The output generation function will be called, and Unity will assert that the resulting string contains valid JSON keys (e.g., `"rssi"`, `"quality"`).

#### TC-BUILD-01 & TC-BUILD-02 – CMake Modular Verification

Verification is conducted via build inspection. The build compiler will successfully generate the `.bin` executable without throwing monolithic driver dependency warnings, proving the `REQUIRES` syntax in component CMake files is correctly bounding scope.

### Edge Case Tests

#### TC-WIFI-03 – Event-Driven Drop Monitoring

`wifi_manager.c` will be tested by forcing an artificial `WIFI_EVENT_STA_DISCONNECTED` software event loop trigger. The module must successfully catch the event, increment the reconnection counter, and issue a retry without causing a hardware panic.

#### TC-WIFI-04 – Frequency Band Protocol Constraint

`wifi_manager.c` will be tested by confirming the Wi-Fi interface is initialized in station mode and that the connected AP channel is within the supported 2.4 GHz channel range. The test will call `esp_wifi_sta_get_ap_info()` after connection and assert that the returned primary channel is valid for 2.4 GHz operation.

#### TC-WEB-02 – Local Network Access Boundary

`web_server.c` will be tested by accessing `/` and `/api/status` from a client on the same LAN/subnet. The test confirms that the dashboard is served locally and documents that WAN/cloud access, port forwarding, and remote authentication are outside the project scope.

#### TC-WEB-03 – Lightweight HTTP Server Limit Test

`web_server.c` will be tested by repeatedly refreshing `/` and `/api/status` from a browser or curl loop. The expected result is that the ESP32 continues serving valid responses without crashing, resetting, or corrupting shared diagnostic data.