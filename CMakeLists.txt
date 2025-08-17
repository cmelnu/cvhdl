cmake_minimum_required(VERSION 3.10)
project(compi VERSION 0.1 LANGUAGES C)

option(DEBUG "Enable debug features" OFF)
list(APPEND CMAKE_ARGS_C -DDEBUG=${DEBUG})

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Automatically include all .c files in src/
file(GLOB COMPI_SRC "src/*.c")

add_executable(compi ${COMPI_SRC})

# Add compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(compi PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(DEBUG)
  message(STATUS "DEBUG enabled")
  target_compile_definitions(compi PRIVATE DEBUG)
endif()

# Install targets
install(TARGETS compi
        RUNTIME DESTINATION)

# Docs target for Sphinx documentation
add_custom_target(docs
    COMMAND sphinx-build -b html docs/source docs/build/html
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building HTML documentation with Sphinx"
)