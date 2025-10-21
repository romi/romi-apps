cmake_minimum_required(VERSION 3.10)
project(romi-datalog)

set(SOURCES
  main.cpp
  DataLog.h
  DataLog.cpp
  DataLogAdaptor.h
  DataLogAdaptor.cpp
  )

add_executable(romi-datalog ${SOURCES})
target_link_libraries(romi-datalog romi rcom)
