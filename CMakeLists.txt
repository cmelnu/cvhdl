cmake_minimum_required(VERSION 3.10)
project(compi VERSION 0.1 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add executable

# Add compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(compi PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install targets
install(TARGETS compi
        RUNTIME DESTINATION bin)

# Add a custom target for testing
add_custom_target(run_test
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/compi test.c test.vhdl
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running compi with test input"
    DEPENDS compi
)