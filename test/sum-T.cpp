#include <gtest/gtest.h>
#include "Example_library/sum.hpp"



TEST(Example_library, sum) {
    int arr[] = { 9, 3, 5 };

    long result = sum(arr, 3);
    
    EXPECT_EQ(result, 17);

    // EXAMPLE: Expect two strings not to be equal.
    EXPECT_STRNE("hello", "world");
}