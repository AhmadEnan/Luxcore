# Auto-find all sandbox source files
file(GLOB_RECURSE SANDBOX_SOURCES
    "*.cpp"
    "*.h"
)

# Create executable
add_executable(Sandbox ${SANDBOX_SOURCES})

# Link engine and dependencies
target_link_libraries(Sandbox PRIVATE Luxcore)

target_include_directories(Luxcore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Copy resources/shaders next to executable
add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:Sandbox>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:Sandbox>/shaders
)