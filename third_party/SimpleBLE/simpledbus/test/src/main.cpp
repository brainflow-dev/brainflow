
#include <gtest/gtest.h>
#include "helpers/PythonRunner.h"

int main(int argc, char** argv) {
    PythonRunner runner("test_fixture.py");

    runner.init();

    ::testing::InitGoogleTest(&argc, argv);
    auto test_result = RUN_ALL_TESTS();

    runner.uninit();

    return test_result;
}
