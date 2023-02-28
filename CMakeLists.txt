cmake_minimum_required(VERSION 3.23)
project(rusty_c LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(AddLLVM) 
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  FindLLVMAt("D:/llvm-sdk/msvcrt-dbg")
else() 
  FindLLVMAt("D:/llvm-sdk/msvcrt-release")
endif()


add_subdirectory(src bin)

add_subdirectory(test)

add_executable(draft draft/main.cc)
target_link_libraries(draft PUBLIC frontend)