cmake_minimum_required(VERSION 2.8.3)
project(position_publisher)

find_package(catkin REQUIRED COMPONENTS roscpp std_msgs)

include(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

catkin_package(
  DEPENDS ${SDL2_LIBRARIES}
  DEPENDS ${SDL2IMAGE_LIBRARIES}
)

include_directories(include
  ${SDL2_INCLUDE_DIRS}
  ${SDL2IMAGE_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)

add_executable(interface src/interface.cpp)
target_link_libraries(interface
  ${SDL2_LIBRARIES}
  ${SDL2IMAGE_LIBRARIES}
  ${catkin_LIBRARIES}
)
