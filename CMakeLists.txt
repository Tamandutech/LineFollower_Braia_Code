cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
list(
    APPEND EXTRA_COMPONENT_DIRS
    "modules/common/components"
    "modules/common/services"
    "modules/robot/components"
    "modules/robot/services"
)
project(robot)
