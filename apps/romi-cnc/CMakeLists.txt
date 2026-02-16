cmake_minimum_required(VERSION 3.10)
project(romi-cnc)

set(SOURCES
  CNCFactory.cpp
  CNCFactory.h
  CNCOptions.cpp
  CNCOptions.h
  CNCAdaptor.h
  CNCAdaptor.cpp
  ATDC.h
  ATDC.cpp
  FakeCNCController.h
  FakeCNCController.cpp
  Helix.h
  Helix.cpp
  ICNCController.h
  is_valid.h
  is_valid.cpp
  CNC.h
  CNC.cpp
  CNCSettings.h
  CNCSettings.cpp
  plotter.h
  plotter.cpp
  print.h
  print.cpp
  Section.h
  Section.cpp
  SmoothPath.h
  SmoothPath.cpp
  StepperController.h
  StepperController.cpp
  StepperSettings.h
  StepperSettings.cpp
  )

add_library(cnc SHARED ${SOURCES})
set_target_properties(cnc PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(cnc
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )

target_link_libraries(cnc
        romi
        romiserial
        rcom
        ${libcamera_LINK_LIBRARIES}
        )


add_executable(romi-cnc main.cpp)
target_link_libraries(romi-cnc cnc romi rcom romiserial)
target_include_directories(romi-cnc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(test)


