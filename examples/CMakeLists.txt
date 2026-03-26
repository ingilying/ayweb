set(EXAMPLE_SOURCES src/echo.cpp)
add_executable(example_echo ${EXAMPLE_SOURCES})
target_link_libraries(example_echo PRIVATE ayweb)
