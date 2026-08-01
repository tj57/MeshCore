cmake_minimum_required(VERSION 3.16)
project(mcrpc VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mcrpc
  src/mcrpc/Parser.cpp
  src/mcrpc/Dispatcher.cpp
  src/mcrpc/CommandRegistry.cpp
  src/mcrpc/CapabilityRegistry.cpp
  src/mcrpc/EventBus.cpp
  src/mcrpc/FeatureManager.cpp
  src/mcrpc/McRpc.cpp
  src/mcrpc/Config.cpp
  src/mcrpc/Registry.cpp
  src/mcrpc/features/core/CoreFeature.cpp
  src/mcrpc/features/battery/BatteryFeature.cpp
  src/mcrpc/features/button/ButtonFeature.cpp
  src/mcrpc/features/gps/GpsFeature.cpp
  src/mcrpc/features/relay/RelayFeature.cpp
  src/mcrpc/features/display/DisplayFeature.cpp
)

add_library(mcrpc::mcrpc ALIAS mcrpc)

target_include_directories(mcrpc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(mcrpc PRIVATE -Wall -Wextra)

# Host / CI tests
option(MCRPC_BUILD_TESTS "Build mcRPC host tests" ON)
if(MCRPC_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../test/mcrpc/test_mcrpc.cpp")
  enable_testing()
  add_executable(mcrpc_tests ${CMAKE_CURRENT_SOURCE_DIR}/../../test/mcrpc/test_mcrpc.cpp)
  target_link_libraries(mcrpc_tests PRIVATE mcrpc)
  add_test(NAME mcrpc_host COMMAND mcrpc_tests)
endif()

install(DIRECTORY src/mcrpc/
  DESTINATION include/mcrpc
  FILES_MATCHING PATTERN "*.h")
install(TARGETS mcrpc
  EXPORT mcrpcTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib)
