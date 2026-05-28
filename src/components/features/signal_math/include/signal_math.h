// signal_math.h - Pure helper functions for interpreting Wi-Fi RSSI
//
// These functions do not touch ESP-IDF Wi-Fi, FreeRTOS, or hardware. That makes
// them easy to unit test on their own.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Convert raw RSSI in dBm into a user-friendly 0..100 percentage.
int signal_math_rssi_to_quality(int rssi_dbm);

// Convert a 0..100 quality value into a dashboard label.
const char *signal_math_quality_to_label(int quality_percent);

// Convenience helper: RSSI -> quality -> label.
const char *signal_math_rssi_to_label(int rssi_dbm);

#ifdef __cplusplus
}
#endif
