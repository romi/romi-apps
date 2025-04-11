
cmake_minimum_required(VERSION 3.10)
project(romi-apps)

############################################################

set(COMMON_COMPILATION_FLAGS "-Wall -Wpedantic -Werror -Wconversion -Wswitch-default -Wswitch-enum -Winit-self -Waddress -Wlogical-op -Wpointer-arith -Wformat=2 -fpic")

set(CMAKE_CXX_FLAGS "${COMMON_COMPILATION_FLAGS} -Wzero-as-null-pointer-constant ${PROJECT_SANITISE_FLAGS}")
set(CMAKE_C_FLAGS "${COMMON_COMPILATION_FLAGS} ${PROJECT_SANITISE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROJECT_SANITISE_FLAGS}")

output_compile_flags()

############################################################

add_subdirectory(romi-cnc)
add_subdirectory(romi-camera)
add_subdirectory(romi-config)
