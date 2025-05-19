cmake_minimum_required(VERSION 3.10)
project(romi-dc-motors)

set(SOURCES
  main.cpp
  BrushMotorDriver.h
  BrushMotorDriver.cpp
  MotorsAdaptor.h
  MotorsAdaptor.cpp
  MotorsOptions.h
  MotorsOptions.cpp
  NavigationSettings.h
  NavigationSettings.cpp
  )

add_executable(romi-dc-motors ${SOURCES})
target_link_libraries(romi-dc-motors romi rcom romiserial)
#target_include_directories(romi-cnc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
