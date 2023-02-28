llvm_map_components_to_libnames(llvm_libs support core irreader)

add_executable(
  rusty_c main.cc
)

target_link_libraries(rusty_c 
  PRIVATE ${llvm_libs}
)

target_include_directories(rusty_c 
  PRIVATE ${LLVM_INCLUDE_DIR}
)

file(GLOB FRONTEND_FILES "frontend/*.cpp")

add_library(frontend OBJECT 
  ${FRONTEND_FILES}
)

target_include_directories(frontend PUBLIC  "${CMAKE_CURRENT_LIST_DIR}")