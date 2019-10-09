cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
project(abcp)

find_package(PCL 1.2 REQUIRED)
find_package(realsense2 REQUIRED)
find_package(OpenCV REQUIRED core highgui)
find_package(glfw3 REQUIRED)

include_directories(${PCL_INCLUDE_DIRS}
${OpenCV_INCLUDE_DIRS}
${realsense2_INCLUDE_DIRS}
)
link_directories(${PCL_LIBRARY_DIRS}
${realsense2_LIBRARY_DIRS}
${OpenCV_LIBRARY_DIRS}
)
add_definitions(${PCL_DEFINITIONS})

add_executable (abcp_pcl abcp_pcl.cpp)
target_link_libraries (abcp_pcl ${PCL_LIBRARIES} ${OpenCV_LIBS} ${realsense2_LIBS}
${glfw3_LIBS}
realsense2
glfw
)
target_include_directories(abcp_pcl PUBLIC
 /home/fred/Documents/github/pcl-pcl-1.9.1/build/lib/
 ../librealsense/examples/)
