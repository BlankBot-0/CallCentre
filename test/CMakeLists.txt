cmake_minimum_required(VERSION 3.22.1)
project(testCallCentre)
add_executable(testExe test_all.cpp)

target_include_directories(testExe PUBLIC ${PROJ_INCLUDE_DIRS} CallCentre_lib)
target_link_libraries(testExe PUBLIC ${PROJ_LINK_LIBRARIES} GTest::gtest_main GTest::gmock_main CallCentre_lib)
add_test(all_gtest testExe)