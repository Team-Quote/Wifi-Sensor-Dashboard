// test_nvs_manager.c - Unity tests for the nvs_manager component
//
// These tests are intentionally simple because NVS is hardware/flash-backed.
// The main goal is to prove that the wrapper returns ESP_OK and can be called
// repeatedly without crashing the test application.

#include "unity.h"
#include "esp_err.h"
#include "nvs_manager.h"

TEST_CASE("nvs_manager initializes NVS", "[nvs_manager]")
{
    TEST_ASSERT_EQUAL(ESP_OK, nvs_manager_init());
}

TEST_CASE("nvs_manager can be called more than once", "[nvs_manager]")
{
    TEST_ASSERT_EQUAL(ESP_OK, nvs_manager_init());
    TEST_ASSERT_EQUAL(ESP_OK, nvs_manager_init());
}

TEST_CASE("nvs_manager edge case: repeat init remains stable", "[nvs_manager][edge]")
{
    for (int i = 0; i < 3; ++i) {
        TEST_ASSERT_EQUAL(ESP_OK, nvs_manager_init());
    }
}
