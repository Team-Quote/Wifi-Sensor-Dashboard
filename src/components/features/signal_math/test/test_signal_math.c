// test_signal_math.c - Unit tests for pure RSSI math helpers

#include "signal_math.h"
#include "unity.h"

TEST_CASE("RSSI converts to expected quality values", "[signal_math]")
{
    TEST_ASSERT_EQUAL_INT(100, signal_math_rssi_to_quality(-50));
    TEST_ASSERT_EQUAL_INT(50, signal_math_rssi_to_quality(-70));
    TEST_ASSERT_EQUAL_INT(0, signal_math_rssi_to_quality(-90));
}

TEST_CASE("Quality converts to expected labels", "[signal_math]")
{
    TEST_ASSERT_EQUAL_STRING("Excellent", signal_math_quality_to_label(100));
    TEST_ASSERT_EQUAL_STRING("Good", signal_math_quality_to_label(70));
    TEST_ASSERT_EQUAL_STRING("Fair", signal_math_quality_to_label(50));
    TEST_ASSERT_EQUAL_STRING("Weak", signal_math_quality_to_label(25));
    TEST_ASSERT_EQUAL_STRING("Bad", signal_math_quality_to_label(10));
}

TEST_CASE("RSSI quality clamps at edge values", "[signal_math][edge]")
{
    TEST_ASSERT_EQUAL_INT(100, signal_math_rssi_to_quality(-10));
    TEST_ASSERT_EQUAL_INT(0, signal_math_rssi_to_quality(-127));
}
