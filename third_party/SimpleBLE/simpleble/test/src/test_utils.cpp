#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

#include <simpleble/Utils.h>

TEST(Utils, GetOS) {
    switch (SimpleBLE::get_operating_system()) {
        case SimpleBLE::OperatingSystem::WINDOWS:
            std::cout << "Windows" << std::endl;
            break;
        case SimpleBLE::OperatingSystem::LINUX:
            std::cout << "Linux" << std::endl;
            break;
        case SimpleBLE::OperatingSystem::MACOS:
            std::cout << "MacOS" << std::endl;
            break;
    }
}