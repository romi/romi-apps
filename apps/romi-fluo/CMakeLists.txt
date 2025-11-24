cmake_minimum_required(VERSION 3.10)
project(romi-fluo)

set(SOURCES
  main.cpp
  IFluoSensor.h
  FluoSensorAdaptor.h
  FluoSensorAdaptor.cpp
  Ambit.h
  Ambit.cpp
  MethodsFluoSensor.h
)

add_executable(romi-fluo ${SOURCES})
target_link_libraries(romi-fluo romi rcom)
