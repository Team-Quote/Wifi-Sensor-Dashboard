// web_server.h - Local HTTP API server for the Wi-Fi Diagnostic Dashboard
//
// Phase 3 responsibility:
// - Start a lightweight ESP-IDF HTTP server.
// - Expose /api/status as JSON.
// - Read from app_state instead of directly touching Wi-Fi.
//
// Phase 4 will expand this module to serve the full browser dashboard at /.

#pragma once

#include <stddef.h>

#include "app_state.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Start the HTTP server.
// Safe to call once from main.c after app_state_init().
esp_err_t web_server_start(void);

// Stop the HTTP server.
// Not needed during normal operation, but useful for tests or future cleanup.
esp_err_t web_server_stop(void);

// Build JSON from a status snapshot.
// Exposed so TC-WEB-01 can unit test JSON serialization without needing Wi-Fi.
int web_server_build_status_json(char *buffer,
                                 size_t buffer_size,
                                 const wifi_diag_status_t *status);

#ifdef __cplusplus
}
#endif