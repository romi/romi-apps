cmake_minimum_required(VERSION 3.10)
project(romi-datalog)

set(SOURCES
  main.cpp
  DataStore.h
  DataStore.cpp
  FileDataLog.h
  FileDataLog.cpp
  DataLogAdaptor.h
  DataLogAdaptor.cpp
  MultiDataLog.h
  MultiDataLog.cpp
  MQTTDataLog.h
  MQTTDataLog.cpp
#  RcomDataLog.h
#  RcomDataLog.cpp
  )

add_executable(romi-datalog ${SOURCES})
target_link_libraries(romi-datalog romi rcom mosquitto)
