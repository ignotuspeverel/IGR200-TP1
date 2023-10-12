cmake_minimum_required(VERSION 3.10)

#set(CMAKE_C_STANDARD 11)
#set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(_MY_OPENGL_IS_33_)
# Set the project name
project(tpOpenGL)

# Include directories for header files
#include_directories(/usr/local/include ./include)

# Find required package
find_package(glfw3 REQUIRED)

# GLOB source files (notice fixed variable name in the GLOB line)
file(GLOB project_files main.cpp ./glad/src/glad.c)

# Add the executable
add_executable(${PROJECT_NAME} ${project_files})

# Specify include directories
#target_include_directories(${PROJECT_NAME} PRIVATE glad/include/)

# Link against libraries
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR})
