add_executable(frontend_test frontend_test.cpp)
target_link_libraries(frontend_test PRIVATE frontend)
AddTest(frontend_test)
