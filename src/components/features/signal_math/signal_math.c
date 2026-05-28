// signal_math.c - Wi-Fi signal interpretation helpers
//
// RSSI is negative. A value closer to 0 is stronger.
// Example: -40 dBm is much better than -85 dBm.

#include "signal_math.h"

int signal_math_rssi_to_quality(int rssi_dbm)
{
    // Strong signal clamp.
    // Anything at or above -50 dBm is treated as excellent/saturated.
    if (rssi_dbm >= -50) {
        return 100;
    }

    // Weak signal clamp.
    // Anything at or below -90 dBm is treated as unusable for this dashboard.
    if (rssi_dbm <= -90) {
        return 0;
    }

    // Linear scaling between -90 and -50 dBm.
    // -90 -> 0%, -70 -> 50%, -50 -> 100%
    return (rssi_dbm + 90) * 100 / 40;
}

const char *signal_math_quality_to_label(int quality_percent)
{
    if (quality_percent >= 85) {
        return "Excellent";
    }

    if (quality_percent >= 65) {
        return "Good";
    }

    if (quality_percent >= 40) {
        return "Fair";
    }

    if (quality_percent >= 20) {
        return "Weak";
    }

    return "Bad";
}

const char *signal_math_rssi_to_label(int rssi_dbm)
{
    return signal_math_quality_to_label(signal_math_rssi_to_quality(rssi_dbm));
}
