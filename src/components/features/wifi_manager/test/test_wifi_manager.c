// test_wifi_manager.c - Unit-testable Wi-Fi manager helpers
//
// These tests do not require joining a real AP. Hardware connection tests such as
// TC-WIFI-01 can be kept as separate integration tests.

#include "wifi_manager.h"
#include "unity.h"

TEST_CASE("Common 2.4 GHz Wi-Fi channels are accepted", "[wifi_manager]")
{
    TEST_ASSERT_TRUE(wifi_manager_channel_is_24ghz(1));
    TEST_ASSERT_TRUE(wifi_manager_channel_is_24ghz(6));
    TEST_ASSERT_TRUE(wifi_manager_channel_is_24ghz(11));
}

TEST_CASE("Upper 2.4 GHz edge channel is accepted", "[wifi_manager]")
{
    TEST_ASSERT_TRUE(wifi_manager_channel_is_24ghz(14));
}

TEST_CASE("Invalid and non-2.4 GHz channels are rejected", "[wifi_manager][edge]")
{
    TEST_ASSERT_FALSE(wifi_manager_channel_is_24ghz(0));
    TEST_ASSERT_FALSE(wifi_manager_channel_is_24ghz(15));
    TEST_ASSERT_FALSE(wifi_manager_channel_is_24ghz(36));
}
