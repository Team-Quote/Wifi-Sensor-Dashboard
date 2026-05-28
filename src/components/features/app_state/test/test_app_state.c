// test_app_state.c - Unit tests for the shared dashboard state model

#include "app_state.h"
#include "unity.h"

TEST_CASE("app_state initializes safe default values", "[app_state]")
{
    TEST_ASSERT_EQUAL(ESP_OK, app_state_init());

    wifi_diag_status_t status;
    TEST_ASSERT_EQUAL(ESP_OK, app_state_get(&status));

    TEST_ASSERT_FALSE(status.wifi_connected);
    TEST_ASSERT_EQUAL(WIFI_DIAG_STATE_BOOTING, status.connection_state);
    TEST_ASSERT_EQUAL_INT(-127, status.rssi);
    TEST_ASSERT_EQUAL_INT(0, status.quality);
    TEST_ASSERT_EQUAL_STRING("Unknown", status.signal_label);
    TEST_ASSERT_EQUAL_STRING("0.0.0.0", status.ip_addr);
}

TEST_CASE("app_state stores and returns signal fields", "[app_state]")
{
    TEST_ASSERT_EQUAL(ESP_OK, app_state_init());
    TEST_ASSERT_EQUAL(ESP_OK, app_state_set_signal(-62, 70, "Good", 6));

    wifi_diag_status_t status;
    TEST_ASSERT_EQUAL(ESP_OK, app_state_get(&status));

    TEST_ASSERT_EQUAL_INT(-62, status.rssi);
    TEST_ASSERT_EQUAL_INT(70, status.quality);
    TEST_ASSERT_EQUAL_STRING("Good", status.signal_label);
    TEST_ASSERT_EQUAL_UINT8(6, status.channel);
    TEST_ASSERT_EQUAL_UINT32(1, status.total_samples);
}

TEST_CASE("app_state rejects NULL output pointer", "[app_state][edge]")
{
    TEST_ASSERT_EQUAL(ESP_OK, app_state_init());
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, app_state_get(NULL));
}
