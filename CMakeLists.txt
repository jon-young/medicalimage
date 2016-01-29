cmake_minimum_required(VERSION 2.8)

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(extractROI extractROI.cpp)

target_link_libraries(extractROI ${ITK_LIBRARIES})
