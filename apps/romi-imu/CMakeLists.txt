cmake_minimum_required(VERSION 3.10)
project(romi-imu)

SET(romi-imu-sources
  vector.h
  vector.c
  quaternion.h
  quaternion.c
  IIMU.h
  IMURT.h
  IMURT.cpp
  IMUAdaptor.h
  IMUAdaptor.cpp
  MethodsIMU.h
  main.cpp
  RTIMULib/RTFusion.cpp
  RTIMULib/RTFusionRTQF.cpp
  RTIMULib/RTMath.cpp
  RTIMULib/RTFusionKalman4.cpp
  RTIMULib/RTIMUAccelCal.cpp
  RTIMULib/RTIMUHal.cpp
  RTIMULib/RTIMUMagCal.cpp
  RTIMULib/RTIMUSettings.cpp
  RTIMULib/IMUDrivers/RTIMU.cpp
  RTIMULib/IMUDrivers/RTIMULSM9DS1.cpp
  RTIMULib/IMUDrivers/RTIMUNull.cpp
)

add_executable(romi-imu ${romi-imu-sources})
target_include_directories(romi-imu PRIVATE RTIMULib)
target_link_libraries(romi-imu romi rcom)


SET(romi-imu-test-sources
  vector.h
  vector.c
  quaternion.h
  quaternion.c
  IIMU.h
  IMURT.h
  IMURT.cpp
  test.cpp
  RTIMULib/RTFusion.cpp
  RTIMULib/RTFusionRTQF.cpp
  RTIMULib/RTMath.cpp
  RTIMULib/RTFusionKalman4.cpp
  RTIMULib/RTIMUAccelCal.cpp
  RTIMULib/RTIMUHal.cpp
  RTIMULib/RTIMUMagCal.cpp
  RTIMULib/RTIMUSettings.cpp
  RTIMULib/IMUDrivers/RTIMU.cpp
  RTIMULib/IMUDrivers/RTIMULSM9DS1.cpp
  RTIMULib/IMUDrivers/RTIMUNull.cpp
)

add_executable(romi-imu-test ${romi-imu-test-sources})
target_include_directories(romi-imu-test PRIVATE RTIMULib)
target_link_libraries(romi-imu-test romi rcom)
