cmake_minimum_required(VERSION 3.10)
project(Luxcore VERSION 0.1.0 LANGUAGES C CXX)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Set available build types
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

# Set compiler flags for different build types
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra)
    add_definitions(-DLUXDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DLUXRELEASE)
endif()

# Global configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vendor dependencies
add_subdirectory(vendor/glad)

# add_subdirectory(vendor/glfw)
# add_subdirectory(vendor/imgui)
# add_subdirectory(vendor/stb)
file(GLOB_RECURSE LUXCORE_SOURCES
    "src/*.cpp"
    "include/*.h"
)

# Luxcore library
add_library(Luxcore STATIC ${LUXCORE_SOURCES})

target_include_directories(Luxcore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(Luxcore PRIVATE
    glad
    #glfw
    #imgui
    #stb_image
)

# Tests
add_subdirectory(sandbox)
