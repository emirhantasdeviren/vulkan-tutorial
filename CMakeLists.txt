cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

project(vulkan-tutorial VERSION 0.1.0)

add_executable(vulkan-tutorial
    ${CMAKE_SOURCE_DIR}/src/window.cpp
    ${CMAKE_SOURCE_DIR}/src/renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/app.cpp
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

target_include_directories(vulkan-tutorial PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    "C:\\VulkanSDK\\1.3.216.0\\Include"
)
target_link_directories(vulkan-tutorial PUBLIC ${CMAKE_SOURCE_DIR}/lib "C:\\VulkanSDK\\1.3.216.0\\Lib")
target_link_libraries(vulkan-tutorial PUBLIC vulkan-1)
