cmake_minimum_required(VERSION 3.10)
project(compi VERSION 0.1 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add executable and sources
add_executable(compi compi.c token.c parse.c utils.c)

# Add compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(compi PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install targets
install(TARGETS compi
        RUNTIME DESTINATION)