enable_testing()

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY "https://ghproxy.com/https://github.com/google/googletest"
  GIT_TAG v1.13.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

macro(AddTest target)
  target_link_libraries(${target} PRIVATE gtest_main)
  gtest_discover_tests(${target})
endmacro()