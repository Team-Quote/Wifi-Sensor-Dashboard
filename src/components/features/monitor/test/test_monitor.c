// test_monitor.c - Unity tests for monitor helper logic
//
// The full monitor task talks to Wi-Fi hardware and runs forever, so these unit
// tests focus on small deterministic helper logic. Hardware/integration tests can
// later verify actual RSSI sampling with esp_wifi_sta_get_ap_info().

#include <stdint.h>
#include "unity.h"
#include "monitor.h"

TEST_CASE("monitor converts one second of microseconds to uptime seconds", "[monitor]")
{
    TEST_ASSERT_EQUAL_UINT32(1, monitor_uptime_seconds_from_microseconds(1000000LL));
}

TEST_CASE("monitor drops fractional uptime seconds", "[monitor]")
{
    TEST_ASSERT_EQUAL_UINT32(1, monitor_uptime_seconds_from_microseconds(1999999LL));
    TEST_ASSERT_EQUAL_UINT32(2, monitor_uptime_seconds_from_microseconds(2000000LL));
}

TEST_CASE("monitor edge case: negative uptime becomes zero", "[monitor][edge]")
{
    TEST_ASSERT_EQUAL_UINT32(0, monitor_uptime_seconds_from_microseconds(-1));
}
