cmake_minimum_required(VERSION 3.21..4.2.3)

project(
  ayweb
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# Option: build shared libraries (default OFF → static by default)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)
# Option: build examples
option(BUILD_EXAMPLES "Build all examples" ON)

find_package(asio CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
  TooManyCooks
  GIT_REPOSITORY https://github.com/tzcnt/TooManyCooks.git
  GIT_TAG fb153ed2c77c9fb3bc8f747406acd16bc0485409)
FetchContent_MakeAvailable(TooManyCooks)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(PSOURCES src/http_server.cpp src/protocol.cpp src/router.cpp
             src/connection.cpp)

set(PHEADERS
    include/ayweb/ayweb.hpp include/ayweb/http_server.hpp
    include/ayweb/protocol.hpp include/ayweb/router.hpp
    include/ayweb/util/func.hpp include/ayweb/connection.hpp)

# Create library (type controlled by BUILD_SHARED_LIBS; default is static)
add_library(ayweb ${PSOURCES} ${PHEADERS})

target_link_libraries(ayweb PUBLIC asio::asio)

# Public include directory for consumers
target_include_directories(
  ayweb
  PUBLIC ${toomanycooks_SOURCE_DIR}/include
         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>)

target_compile_definitions(ayweb PRIVATE AYWEB_COMPILE_LIBRARY)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(ayweb PUBLIC AYWEB_SHARED_LIBRARY)
endif(BUILD_SHARED_LIBS)

# Examples
add_subdirectory(examples)
# Test
add_subdirectory(test)
