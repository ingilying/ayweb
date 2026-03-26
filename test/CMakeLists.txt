enable_testing()
set(TEST_SOURCES src/test.cpp)
add_executable(ayweb_test ${TEST_SOURCES})

target_link_libraries(ayweb_test PRIVATE GTest::gtest_main ayweb)
include(GoogleTest)
gtest_discover_tests(ayweb_test)
