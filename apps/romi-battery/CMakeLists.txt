cmake_minimum_required(VERSION 3.10)
project(romi-battery)

set(SOURCES
  main.cpp
  BatteryAdaptor.h
  BatteryAdaptor.cpp
  BatteryStatusIndicator.cpp
  BatteryStatusIndicator.h
  IBattery.h
  IBatteryMonitor.h
  INA219BatteryMonitor.h
  INA219BatteryMonitor.cpp
  LithiumBattery.h
  LithiumBattery.cpp
  MethodsBattery.h
  )

add_executable(romi-battery ${SOURCES})
target_link_libraries(romi-battery romi rcom)
