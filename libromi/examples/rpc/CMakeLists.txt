cmake_minimum_required(VERSION 3.10)
project(libromi)

set(SOURCES
  main.cpp
  ClockAdaptor.cpp
  ClockAdaptor.h
  )

add_executable(romi-clock ${SOURCES})
target_link_libraries(romi-clock romi rcom)
