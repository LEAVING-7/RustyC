include(FetchContent)

FetchContent_Declare(
  fmt 
  GIT_REPOSITORY https://ghproxy.com/https://github.com/fmtlib/fmt
  GIT_TAG        9.1.0
)

FetchContent_MakeAvailable(fmt)

