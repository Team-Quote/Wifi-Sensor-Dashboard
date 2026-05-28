// monitor.h - Public interface for the telemetry monitor task
//
// The monitor task periodically reads Wi-Fi telemetry and updates app_state.

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Start the background FreeRTOS task that samples RSSI/IP/channel periodically.
esp_err_t monitor_start(void);

// Exposed for unit/integration tests. Normal application code should call
// monitor_start() instead of creating this task directly.
void monitor_task(void *arg);

#ifdef __cplusplus
}
#endif
