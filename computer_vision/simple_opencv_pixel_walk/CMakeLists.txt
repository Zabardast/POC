cmake_minimum_required(VERSION 3.10)

# Set the project name
project(read_image)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the OpenCV package
find_package(OpenCV REQUIRED)

# Include the OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Add the executable
add_executable(read_image main.cpp)

# Link the OpenCV libraries
target_link_libraries(read_image ${OpenCV_LIBS})