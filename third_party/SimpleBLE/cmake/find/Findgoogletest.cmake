# Read the documentation of FetchContent first!
# https://cmake.org/cmake/help/latest/module/FetchContent.html

include(FetchContent)

# Library includes
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

# Because we are in a find module, we are solely responsible for resolution.
# Setting this *_FOUND variable to a truthy value will signal to the calling
# find_package() command that we were successful.
# More relevant info regarding find modules and what variables they use can be
# found in the documentation of find_package() and
# https://cmake.org/cmake/help/latest/manual/cmake-developer.7.html
set(googletest_FOUND 1)