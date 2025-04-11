cmake_minimum_required(VERSION 3.10)
project(romi-cnc)

set(SOURCES
  main.cpp
  CNCFactory.cpp
  CNCFactory.h
  CNCOptions.cpp
  CNCOptions.h
  CNCAdaptor.h
  CNCAdaptor.cpp
  oquam/ATDC.h
  oquam/FakeCNCController.h
  oquam/Helix.h
  oquam/ICNCController.h
  oquam/is_valid.h
  oquam/Oquam.h
  oquam/OquamSettings.h
  oquam/plotter.h
  oquam/print.h
  oquam/Section.h
  oquam/SmoothPath.h
  oquam/StepperController.h
  oquam/StepperSettings.h
  oquam/ATDC.cpp
  oquam/Helix.cpp
  oquam/is_valid.cpp
  oquam/Oquam.cpp
  oquam/OquamSettings.cpp
  oquam/plotter.cpp
  oquam/print.cpp
  oquam/Section.cpp
  oquam/SmoothPath.cpp
  oquam/StepperController.cpp
  oquam/StepperSettings.cpp
  )



add_executable(romi-cnc ${SOURCES})
target_link_libraries(romi-cnc romi romiserial)
target_include_directories(romi-cnc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#add_subdirectory(test)


