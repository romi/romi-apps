cmake_minimum_required(VERSION 3.10)
project(libromi)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libcamera REQUIRED libcamera>=0.3)

set(SOURCES
  include/v3.h
  src/v3.cpp
  
  include/api/ActivityHelper.h
  include/api/CameraSettings.h
  include/api/CNCRange.h
  include/api/DeviceData.h
  include/api/DummyLocationProvider.h
  include/api/Gps.h
  include/api/GpsLocationProvider.h
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
  include/util/FileUtils.h
  include/util/ImageIO.h
  include/util/Logger.h
  include/util/LogWriter.h
  include/util/RomiSerialLog.h
  include/util/StringUtils.h
  include/util/stb_image.h
  include/util/stb_image_write.h
  include/util/stb_includes.h
  
  src/util/Clock.cpp
  src/util/ClockAccessor.cpp
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
  include/rpc/MethodsPowerDevice.h
  include/rpc/RemoteConfig.h

  src/rpc/RemoteCamera.cpp
  src/rpc/RemoteCNC.cpp
  src/rpc/RemoteConfig.cpp

  
#        include/api/INavigation.h
#      
#        include/cv/ConnectedComponentsImage.h
#        include/cv/IImageCropper.h
#        include/cv/IPrincipalComponentAnalysis.h
#        include/cv/ImageCropper.h
#        include/cv/connected.h
#        include/oquam/ATDC.h
#        include/oquam/FakeCNCController.h
#        include/oquam/Helix.h
#        include/oquam/ICNCController.h
#        include/oquam/Oquam.h
#        include/oquam/OquamSettings.h
#        include/oquam/Section.h
#        include/oquam/SmoothPath.h
#        include/oquam/StepperController.h
#        include/oquam/StepperSettings.h
#        include/oquam/is_valid.h
#        include/oquam/plotter.h
#        include/oquam/print.h
#        include/rover/DifferentialSteering.h
#        include/rover/DoubleSteering.h
#        include/rover/EventsAndStates.h
#        include/rover/IDistanceMeasure.h
#        include/rover/IMUTrackFollower.h
#        include/rover/INavigationController.h
#        include/rover/ISpeedController.h
#        include/rover/ISteering.h
#        include/rover/ISteeringController.h
#        include/rover/ITrackFollower.h
#        include/rover/L1NavigationController.h
#        include/rover/LocationTracker.h
#        include/rover/ManualTrackFollower.h
#        include/rover/Navigation.h
#        include/rover/NavigationSettings.h
#        include/rover/PythonTrackFollower.h
#        include/rover/Rover.h
#        include/rover/RoverInterface.h
#        include/rover/RoverNotifications.h
#        include/rover/RoverOptions.h
#        include/rover/RoverScriptEngine.h
#        include/rover/RoverStateMachine.h
#        include/rover/SpeedController.h
#        include/rover/SpeedConverter.h
#        include/rover/SteeringController.h
#        include/rover/SteeringData.h
#        include/rover/StepperSteering.h
#        include/rover/WheelOdometry.h
#        include/rover/ZeroNavigationController.h
#        include/ui/CrystalDisplay.h
#        include/ui/IJoystickEventMapper.h
#        include/ui/IScriptEngine.h
#        include/ui/JoystickInputDevice.h
#        include/ui/LinuxJoystick.h
#        include/ui/Menu.h
#        include/ui/RemoteStateInputDevice.h
#        include/ui/ScriptMenu.h
#        include/ui/UIEventMapper.h
#
#        src/cv/ConnectedComponentsImage.cpp
#        src/cv/ImageCropper.cpp
#        src/cv/PrincipalComponentAnalysis.cpp
#        src/cv/Superpixels.cpp
#        src/oquam/ATDC.cpp
#        src/oquam/Helix.cpp
#        src/oquam/Oquam.cpp
#        src/oquam/OquamSettings.cpp
#        src/oquam/Section.cpp
#        src/oquam/SmoothPath.cpp
#        src/oquam/StepperController.cpp
#        src/oquam/StepperSettings.cpp
#        src/oquam/is_valid.cpp
#        src/oquam/plotter.cpp
#        src/oquam/print.cpp
#        src/rover/DifferentialSteering.cpp
#        src/rover/DoubleSteering.cpp
#        src/rover/EventsAndStates.cpp
#        src/rover/IMUTrackFollower.cpp
#        src/rover/L1NavigationController.cpp
#        src/rover/LocationTracker.cpp
#        src/rover/ManualTrackFollower.cpp
#        src/rover/Navigation.cpp
#        src/rover/NavigationSettings.cpp
#        src/rover/PythonTrackFollower.cpp
#        src/rover/Rover.cpp
#        src/rover/RoverInterface.cpp
#        src/rover/RoverOptions.cpp
#        src/rover/RoverScriptEngine.cpp
#        src/rover/RoverStateMachine.cpp
#        src/rover/SpeedController.cpp
#        src/rover/SpeedConverter.cpp
#        src/rover/SteeringController.cpp
#        src/rover/StepperSteering.cpp
#        src/rover/WheelOdometry.cpp
#        src/ui/CrystalDisplay.cpp
#        src/ui/JoystickInputDevice.cpp
#        src/ui/LinuxJoystick.cpp
#        src/ui/RemoteStateInputDevice.cpp
#        src/ui/ScriptMenu.cpp
#        src/ui/UIEventMapper.cpp
#        
#        include/IFileCabinet.h
#        include/IFolder.h
#        include/api/DataLog.h
#        include/api/DataLogAccessor.h
#        include/api/EventTimer.h
#        include/api/GimbalRange.h
#        include/api/IActivity.h
#        include/api/IConfigManager.h
#        include/api/IDataLog.h
#        include/api/IDisplay.h
#        include/api/IEventSource.h
#        include/api/IEventTimer.h
#        include/api/IGimbal.h
#        include/api/IImager.h
#        include/api/IInputDevice.h
#        include/api/IJoystick.h
#        include/api/IMotorDriver.h
#        include/api/INotifications.h
#        include/api/IPowerDevice.h
#        include/api/IRange.h
#        include/api/IWeeder.h
#        include/api/LocalConfig.h
#        include/api/Path.h
#        include/api/Range.h
#        include/api/StateMachine.h
#        include/battery_monitor/BatteryMonitor.h
#        include/battery_monitor/IBatteryMonitor.h
#        include/cablebot/Cablebot.h
#        include/cablebot/CablebotBase.h
#        include/cablebot/CablebotController.h
#        include/cablebot/CablebotProgram.h
#        include/cablebot/CablebotProgramIO.h
#        include/cablebot/CablebotProgramList.h
#        include/cablebot/CablebotProgramListAdaptor.h
#        include/cablebot/CablebotRunner.h
#        include/cablebot/FakeMotorController.h
#        include/cablebot/ICablebotProgram.h
#        include/cablebot/ICablebotProgramIO.h
#        include/cablebot/ICablebotProgramList.h
#        include/camera/CameraInfo.h
#        include/camera/CameraInfoIO.h
#        include/camera/CameraIntrinsics.h
#        include/camera/CameraSettings.h
#        include/camera/CameraWithConfig.h
#        include/camera/ExternalCamera.h
#        include/camera/FakeCamera.h
#        include/camera/FileCamera.h
#        include/camera/ICameraDistortion.h
#        include/camera/ICameraInfo.h
#        include/camera/ICameraInfoIO.h
#        include/camera/ICameraIntrinsics.h
#        include/camera/ICameraIntrinsics.h
#        include/camera/ICameraSettings.h
#        include/camera/Imager.h
#        include/camera/LibCamera.h
#        include/camera/SimpleRadialDistortion.h        
#        include/camera/USBCamera.h
#        include/configuration/ConfigurationProvider.h
#        include/configuration/IOptions.h
#        include/cv/cv.h
#        include/cv/slic.h
#        include/fake/FakeCNC.h
#        include/fake/FakeDisplay.h
#        include/fake/FakeInputDevice.h
#        include/fake/FakeMotorDriver.h
#        include/fake/FakeNavigation.h
#        include/fake/FakeNotifications.h
#        include/fake/FakeScriptEngine.h
#        include/fake/FakeWeeder.h
#        include/hal/BldcGimbal.h
#        include/hal/BldcGimbalI2C.h
#        include/hal/BrushMotorDriver.h
#        include/hal/CNCAndGimbal.h
#        include/hal/FakeGimbal.h
#        include/hal/FakeI2C.h
#        include/hal/ICameraMount.h
#        include/hal/II2C.h
#        include/hal/I2C.h
#        include/hal/ImagingDevice.h
#        include/rpc/CameraAdaptor.h
#        include/rpc/CameraMountAdaptor.h
#        include/rpc/ConfigAdaptor.h
#        include/rpc/MethodsActivity.h
#        include/rpc/MethodsCNC.h
#        include/rpc/MethodsCamera.h
#        include/rpc/MethodsCameraMount.h
#        include/rpc/MethodsNavigation.h
#        include/rpc/MethodsRover.h
#        include/rpc/MethodsWeeder.h
#        include/rpc/NavigationAdaptor.h
#        include/rpc/RcomLog.h
#        include/rpc/RemoteCNC.h
#        include/rpc/RemoteCamera.h
#        include/rpc/RemoteCameraMount.h
#        include/rpc/RemoteConfig.h
#        include/rpc/RemoteFunctionCallNames.h
#        include/rpc/RemoteNavigation.h
#        include/rpc/RemoteWeeder.h
#        include/rpc/WeederAdaptor.h
#        include/ui/ScriptList.h
#        include/util/AlarmClock.h
#        include/util/IAlarmClock.h
#        include/util/IAlarmClockListener.h
#        include/util/IClock.h
#        include/util/ILogWriter.h
#        include/util/ILogger.h
#        include/util/IThreadsafeQueue.h
#        include/util/LogWriter.h
#        include/util/Logger.h
#        include/util/ThreadsafeQueue.h
#        
#        src/api/ActivityHelper.cpp
#        src/api/CNCRange.cpp
#        src/api/DataLog.cpp
#        src/api/DataLogAccessor.cpp
#        src/api/EventTimer.cpp
#        src/api/GimbalRange.cpp
#        src/api/LocalConfig.cpp
#        src/api/Path.cpp
#        src/api/Range.cpp
#        src/battery_monitor/BatteryMonitor.cpp
#        src/cablebot/Cablebot.cpp
#        src/cablebot/CablebotBase.cpp
#        src/cablebot/CablebotController.cpp
#        src/cablebot/CablebotProgram.cpp
#        src/cablebot/CablebotProgramIO.cpp
#        src/cablebot/CablebotProgramList.cpp
#        src/cablebot/CablebotProgramListAdaptor.cpp
#        src/cablebot/CablebotRunner.cpp
#        src/cablebot/FakeMotorController.cpp
#        src/camera/CameraInfo.cpp
#        src/camera/CameraInfoIO.cpp
#        src/camera/CameraIntrinsics.cpp
#        src/camera/CameraSettings.cpp
#        src/camera/CameraWithConfig.cpp
#        src/camera/ExternalCamera.cpp
#        src/camera/FakeCamera.cpp
#        src/camera/FileCamera.cpp
#        src/camera/Imager.cpp
#        src/camera/LibCamera.cpp
#        src/camera/SimpleRadialDistortion.cpp
#        src/camera/USBCamera.cpp
#        src/camera/camera_v4l.cpp
#        src/camera/camera_v4l.h
#        src/configuration/ConfigurationProvider.cpp
#        src/cv/cv.cpp
#        src/cv/slic.cpp
#        src/fake/FakeScriptEngine.cpp
#        src/hal/BldcGimbal.cpp
#        src/hal/BldcGimbalI2C.cpp
#        src/hal/BrushMotorDriver.cpp
#        src/hal/CNCAndGimbal.cpp
#        src/hal/FakeGimbal.cpp
#        src/hal/FakeI2C.cpp
#        src/hal/ImagingDevice.cpp
#        src/hal/I2C.cpp
#        src/rpc/CameraAdaptor.cpp
#        src/rpc/CameraMountAdaptor.cpp
#        src/rpc/ConfigAdaptor.cpp
#        src/rpc/NavigationAdaptor.cpp
#        src/rpc/RemoteCNC.cpp
#        src/rpc/RemoteCamera.cpp
#        src/rpc/RemoteCameraMount.cpp
#        src/rpc/RemoteConfig.cpp
#        src/rpc/RemoteNavigation.cpp
#        src/rpc/RemoteWeeder.cpp
#        src/rpc/ScriptHubListener.cpp
#        src/rpc/WeederAdaptor.cpp
#        src/ui/ScriptList.cpp
#        src/util/AlarmClock.cpp
#        src/util/Clock.cpp
#        src/util/ClockAccessor.cpp
#        src/util/LogWriter.cpp
#        src/util/Logger.cpp
#
#        include/data_provider/CNCLocationProvider.h
#        include/data_provider/DummyLocationProvider.h
#        include/data_provider/CameraMountLocationProvider.h
#        include/data_provider/IOrientationProvider.h

#        include/data_provider/RoverIdentityProvider.h
#        include/debug_tools/debug_data_dumper.h
#        
#        src/data_provider/DummyLocationProvider.cpp
#        src/data_provider/CNCLocationProvider.cpp
#        src/data_provider/CameraMountLocationProvider.cpp
#        src/data_provider/Gps.cpp
#        src/data_provider/GpsLocationProvider.cpp        
#        src/data_provider/RomiDeviceData.cpp
#        src/data_provider/RoverIdentityProvider.cpp
#        src/data_provider/SoftwareVersion.cpp
#        src/debug_tools/debug_data_dumper.cpp
#        src/session/MetaFolder.cpp
#        src/session/Session.cpp
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

if(BUILD_TESTS)
    add_subdirectory(test/mock)
    add_subdirectory(test)
endif()


