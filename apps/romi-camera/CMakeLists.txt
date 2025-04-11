cmake_minimum_required(VERSION 3.10)
project(romi-camera)

set(SOURCES
  main.cpp
  ICameraDistortion.h
  ICameraInfo.h
  ICameraInfoIO.h
  ICameraIntrinsics.h
  CameraAdaptor.h
  CameraAdaptor.cpp
  CameraFactory.h
  CameraFactory.cpp
  CameraInfo.h
  CameraInfo.cpp
  CameraInfoIO.h
  CameraInfoIO.cpp
  CameraIntrinsics.h
  CameraIntrinsics.cpp
  CameraWithConfig.h
  CameraWithConfig.cpp
  ExternalCamera.h
  ExternalCamera.cpp
  FakeCamera.h
  FakeCamera.cpp
  FileCamera.h
  FileCamera.cpp
  LibCamera.h
  LibCamera.cpp
  SimpleRadialDistortion.h        
  SimpleRadialDistortion.cpp
  USBCamera.h
  USBCamera.cpp
  camera_v4l.cpp
  camera_v4l.h
  )

add_executable(romi-camera ${SOURCES})
target_link_libraries(romi-camera romi rcom)
