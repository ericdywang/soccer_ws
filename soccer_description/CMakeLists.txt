cmake_minimum_required(VERSION 2.8.3)
project(soccer_description)

find_package(catkin REQUIRED urdf xacro)

catkin_package(
    CATKIN_DEPENDS urdf xacro
)

xacro_add_files(models/soccerbot.xacro OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/models/soccerbot_stl.urdf TARGET media_files)
