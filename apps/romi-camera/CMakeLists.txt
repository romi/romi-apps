cmake_minimum_required(VERSION 3.10)
project(romi-camera)

set(CAMERA_SOURCES
  ICameraDistortion.h
  ICameraInfoIO.h
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
  CameraStatusIndicator.h
  CameraStatusIndicator.cpp
  CameraConfigManager.h
  CameraConfigManager.cpp
  ExternalCamera.h
  ExternalCamera.cpp
  FakeCamera.h
  FakeCamera.cpp
  FakeStatusIndicator.h
  FileCamera.h
  FileCamera.cpp
  ICameraStatusIndicator.h
  LibCamera.h
  LibCamera.cpp
  SimpleRadialDistortion.h        
  SimpleRadialDistortion.cpp
  USBCamera.h
  USBCamera.cpp
  camera_v4l.cpp
  camera_v4l.h
  )

add_executable(romi-camera main.cpp ${CAMERA_SOURCES})
target_link_libraries(romi-camera romi rcom jpeg)

add_executable(testlibcamera TestLibCamera.cpp ${CAMERA_SOURCES})
target_link_libraries(testlibcamera romi rcom jpeg)

#add_executable(test_mjpeg test_mjpeg.cpp)
#target_link_libraries(test_mjpeg romi rcom jpeg)
