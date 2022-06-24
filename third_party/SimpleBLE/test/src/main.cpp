
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    auto test_result = RUN_ALL_TESTS();
    return test_result;
}
