cmake_minimum_required(VERSION 3.10)
project(romi-relay)

set(SOURCES
  main.cpp
  RelayBoardAdaptor.h
  RelayBoardAdaptor.cpp
  GpioRelayBoard.h
  GpioRelayBoard.cpp
  MethodsRelayBoard.h
  )

add_executable(romi-relay ${SOURCES})
target_link_libraries(romi-relay romi rcom)
