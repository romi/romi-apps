cmake_minimum_required(VERSION 3.10)
project(romi-config)

set(SOURCES
  main.cpp
  ConfigAdaptor.h
  ConfigAdaptor.cpp
  )

add_executable(romi-config ${SOURCES})
target_link_libraries(romi-config romi rcom)
