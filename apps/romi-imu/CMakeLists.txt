cmake_minimum_required(VERSION 3.10)
project(romi-imu)

#set(SOURCES
#  main.cpp
#  IIMU.h
#  ITransport.h
#  ISHTP.h
#  I2CTransport.h
#  I2CTransport.cpp
#  BNO085.h 
#  BNO085.cpp
#  IMUAdaptor.cpp
#  IMUAdaptor.h
#  MethodsIMU.h
#  SHTP.h
#  SHTP.cpp
#  vector.h
#  vector.c
#  quaternion.h
#  quaternion.c
#)
#
#add_executable(romi-imu ${SOURCES})
#target_link_libraries(romi-imu romi rcom)
#
#
#set(TESTSOURCES
#  test.cpp
#  IIMU.h
#  ITransport.h
#  ISHTP.h
#  I2CTransport.h
#  I2CTransport.cpp
#  BNO085.h 
#  BNO085.cpp
#  SHTP.h
#  SHTP.cpp
#  vector.h
#  vector.c
#  quaternion.h
#  quaternion.c
#)
#
#add_executable(romi-imu-test ${TESTSOURCES})
#target_link_libraries(romi-imu-test romi rcom)


SET(RTIMU_SRCS
  vector.h
  vector.c
  quaternion.h
  quaternion.c
  RTIMULibDrive.cpp
  RTIMULib/RTFusion.cpp
  RTIMULib/RTFusionRTQF.cpp
  RTIMULib/RTMath.cpp
  RTIMULib/RTFusionKalman4.cpp
  RTIMULib/RTIMUAccelCal.cpp
  RTIMULib/RTIMUHal.cpp
  RTIMULib/RTIMUMagCal.cpp
  RTIMULib/RTIMUSettings.cpp
  RTIMULib/IMUDrivers/RTIMU.cpp
#    RTIMULib/IMUDrivers/RTIMUGD20M303DLHC.cpp
#    RTIMULib/IMUDrivers/RTIMUGD20HM303DLHC.cpp
#    RTIMULib/IMUDrivers/RTIMUGD20HM303D.cpp
#    RTIMULib/IMUDrivers/RTIMULSM9DS0.cpp
  RTIMULib/IMUDrivers/RTIMULSM9DS1.cpp
#    RTIMULib/IMUDrivers/RTIMUMPU9150.cpp
#    RTIMULib/IMUDrivers/RTIMUMPU9250.cpp
#    RTIMULib/IMUDrivers/RTIMUBMX055.cpp
#    RTIMULib/IMUDrivers/RTIMUBNO055.cpp
  RTIMULib/IMUDrivers/RTIMUNull.cpp
#    RTIMULib/IMUDrivers/RTPressure.cpp
#    RTIMULib/IMUDrivers/RTPressureBMP180.cpp
#    RTIMULib/IMUDrivers/RTPressureLPS25H.cpp
#    RTIMULib/IMUDrivers/RTPressureMS5611.cpp
#    RTIMULib/IMUDrivers/RTPressureMS5637.cpp
#    RTIMULib/IMUDrivers/RTHumidity.cpp
#    RTIMULib/IMUDrivers/RTHumidityHTS221.cpp
#    RTIMULib/IMUDrivers/RTHumidityHTU21D.cpp
)

add_executable(RTIMULibDrive ${RTIMU_SRCS})
target_include_directories(RTIMULibDrive PRIVATE RTIMULib)
target_link_libraries(RTIMULibDrive romi rcom)
