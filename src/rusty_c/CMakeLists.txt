llvm_map_components_to_libnames(llvm_libs support core irreader)

add_executable(
  rusty_c main.cc
)

target_link_libraries(rusty_c 
  PUBLIC ${llvm_libs}
)

target_include_directories(rusty_c 
  PUBLIC ${LLVM_INCLUDE_DIR}
)

add_library(utils OBJECT ${CMAKE_CURRENT_LIST_DIR}/utils/utils.cpp)
target_link_libraries(utils PUBLIC fmt::fmt)

file(GLOB FRONTEND_FILES "Frontend/*.cpp" "Frontend/Sema/*.cpp")

add_library(frontend OBJECT 
  ${FRONTEND_FILES}
)

target_link_libraries(frontend PUBLIC ${llvm_libs} utils )

target_include_directories(frontend 
  PUBLIC "${CMAKE_CURRENT_LIST_DIR}" 
  PUBLIC ${LLVM_INCLUDE_DIR}
)

