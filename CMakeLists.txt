cmake_minimum_required(VERSION 3.14)
project(compi VERSION 0.1 LANGUAGES C CXX)

# If a Conan toolchain file was injected at the super-project level but is missing pieces,
# allow an opt-out for this standalone build by setting SKIP_PARENT_TOOLCHAIN.
if(DEFINED SKIP_PARENT_TOOLCHAIN AND SKIP_PARENT_TOOLCHAIN)
  unset(CMAKE_TOOLCHAIN_FILE CACHE)
endif()

option(DEBUG "Enable debug features" OFF)
list(APPEND CMAKE_ARGS_C -DDEBUG=${DEBUG})

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Collect all C sources
file(GLOB COMPI_ALL_SRC "src/*.c")

# Separate main file to allow creating a reusable core library for tests
set(COMPI_MAIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/compi.c")
set(COMPI_CORE_SRC ${COMPI_ALL_SRC})
list(REMOVE_ITEM COMPI_CORE_SRC "${COMPI_MAIN_SRC}")

add_library(compi_core ${COMPI_CORE_SRC})
target_include_directories(compi_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(compi ${COMPI_MAIN_SRC})
target_link_libraries(compi PRIVATE compi_core)

# Add compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(compi_core PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(compi PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(DEBUG)
  message(STATUS "DEBUG enabled")
  target_compile_definitions(compi_core PUBLIC DEBUG)
  target_compile_definitions(compi PRIVATE DEBUG)
endif()

# Install targets
install(TARGETS compi RUNTIME DESTINATION bin)

# Docs target for Sphinx documentation
add_custom_target(docs
    COMMAND sphinx-build -b html docs/source docs/build/html
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building HTML documentation with Sphinx"
)

# =====================
# Testing (GoogleTest)
# =====================
option(ENABLE_TESTING "Build tests" ON)
if(ENABLE_TESTING)
  enable_testing()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  # Try find_package first (Conan or system may provide GTest / gtest_main)
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    message(STATUS "GTest not found via package manager; fetching v1.14.0")
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  else()
    message(STATUS "Using existing GTest package")
  endif()

  file(GLOB TEST_SOURCES "tests/*.cpp" "tests/*.cc" "tests/*.cxx")
  if(TEST_SOURCES)
    add_executable(compi_tests ${TEST_SOURCES})
    if(TARGET GTest::gtest_main)
      target_link_libraries(compi_tests PRIVATE GTest::gtest_main compi_core)
    elseif(TARGET gtest_main)
      target_link_libraries(compi_tests PRIVATE gtest_main compi_core)
    elseif(TARGET gtest)
      target_link_libraries(compi_tests PRIVATE gtest compi_core)
    else()
      message(FATAL_ERROR "No gtest_main or gtest target available")
    endif()
    target_include_directories(compi_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

    # Discover each GoogleTest test case so they show individually in CTest
    include(GoogleTest)
    gtest_discover_tests(
      compi_tests
      DISCOVERY_TIMEOUT 30
      # Optionally set WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    # Convenience aggregate target: build tests then run them with verbose failure output
    add_custom_target(test_all
      COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
      DEPENDS compi_tests
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Building and running all GoogleTest cases"
    )
  endif()
endif()