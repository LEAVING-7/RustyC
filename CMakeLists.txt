cmake_minimum_required(VERSION 3.22.0)
project(rusty_c LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# clang
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")

include(FetchExternal)

include(FindLLVM) 

find_package(LLVM REQUIRED CONFIG)


add_subdirectory(src bin)

# add_subdirectory(test)

add_executable(draft draft/main.cc)
target_link_libraries(draft PUBLIC frontend)
