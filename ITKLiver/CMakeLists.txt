cmake_minimum_required(VERSION 2.8)

project(ITKLiver)

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(geodesic_active_contour geodesicActiveContour.cpp)

target_link_libraries(geodesic_active_contour ${ITK_LIBRARIES})
