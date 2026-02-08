cmake_minimum_required(VERSION 3.10)
project(libromi)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libcamera REQUIRED libcamera>=0.3)

set(SOURCES
  include/v3.h
  src/v3.cpp
  
  include/api/Axis.h
  include/api/ActivityHelper.h
  include/api/CameraSettings.h
  include/api/CNCRange.h
  include/api/DeviceData.h
  include/api/DummyLocationProvider.h
  include/api/Gps.h
  include/api/GpsLocationProvider.h
  include/api/IAxis.h
  include/api/ICNC.h
  include/api/ICamera.h
  include/api/ICameraSettings.h
  include/api/IGPS.h
  include/api/ILocationProvider.h
  include/api/IDeviceData.h
  include/api/IMetaFolder.h
  include/api/ISession.h
  include/api/Image.h
  include/api/JsonFieldNames.h
  include/api/MetaFolder.h
  include/api/Path.h
  include/api/Range.h
  include/api/Session.h
  
  src/api/Axis.cpp
  src/api/ActivityHelper.cpp
  src/api/CameraSettings.cpp
  src/api/CNCRange.cpp
  src/api/DeviceData.cpp
  src/api/DummyLocationProvider.cpp
  src/api/Gps.cpp
  src/api/GpsLocationProvider.cpp        
  src/api/Image.cpp
  src/api/MetaFolder.cpp
  src/api/Path.cpp
  src/api/Range.cpp
  src/api/Session.cpp
  
  include/configuration/GetOpt.h

  src/configuration/LocalConfig.cpp
  src/configuration/GetOpt.cpp
  src/configuration/RomiOptions.cpp

  include/util/Clock.h
  include/util/ClockAccessor.h
  include/util/DataLog.h
  include/util/DataLogAccessor.h
  include/util/FileUtils.h
  include/util/GpioStatusIndicator.h
  include/util/Gpio.h
  include/util/IDataLog.h
  include/util/ImageIO.h
  include/util/Logger.h
  include/util/LogWriter.h
  include/util/RomiSerialLog.h
  include/util/ISubprocess.h
  include/util/Subprocess.h
  include/util/StringUtils.h
  include/util/stb_image.h
  include/util/stb_image_write.h
  include/util/stb_includes.h
  
  src/util/Clock.cpp
  src/util/ClockAccessor.cpp
  src/util/DataLogAccessor.cpp
  src/util/DataLog.cpp
  src/util/GpioStatusIndicator.cpp
  src/util/Gpio.cpp
  src/util/FileUtils.cpp
  src/util/ImageIO.cpp
  src/util/Logger.cpp
  src/util/LogWriter.cpp
  src/util/RomiSerialLog.cpp
  src/util/StringUtils.cpp
  
  include/rpc/MethodsActivity.h
  include/rpc/MethodsCNC.h
  include/rpc/MethodsCamera.h
  include/rpc/MethodsConfig.h
  include/rpc/MethodsDataLog.h
  include/rpc/MethodsPowerDevice.h
  include/rpc/RemoteCamera.h
  include/rpc/RemoteCNC.h
  include/rpc/RemoteConfig.h
  include/rpc/RemoteDataLog.h

  src/rpc/RemoteCamera.cpp
  src/rpc/RemoteCNC.cpp
  src/rpc/RemoteConfig.cpp
  src/rpc/RemoteDataLog.cpp
)

add_library(romi SHARED ${SOURCES})
set_target_properties(romi PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(romi
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ${libcamera_INCLUDE_DIRS}
        )

target_link_libraries(romi
        romiserial
        rcom
        ${libcamera_LINK_LIBRARIES}
        )

#if(BUILD_TESTS)
#    add_subdirectory(test/mock)
#    add_subdirectory(test)
#endif()

add_subdirectory(examples)

