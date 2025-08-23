cmake_minimum_required(VERSION 3.10)
project(romi-battery)

set(SOURCES
  main.cpp
  BatteryMonitorAdaptor.h
  BatteryMonitorAdaptor.cpp
  IBatteryMonitor.h
  INA219BatteryMonitor.h
  INA219BatteryMonitor.cpp
  MethodsBatteryMonitor.h
  )

add_executable(romi-battery ${SOURCES})
target_link_libraries(romi-battery romi rcom)
