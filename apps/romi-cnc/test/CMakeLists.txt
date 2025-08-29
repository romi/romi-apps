cmake_minimum_required(VERSION 3.10)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/mock
  ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(SRCS_OQUAM
        src/tests_main.cpp
        src/section_tests.cpp
        src/helix_tests.cpp
        src/print_tests.cpp
        src/script_tests.cpp
        #src/cnc_tests.cpp
        src/cncrange_tests.cpp
        src/path_tests.cpp
        #src/steppersettings_tests.cpp
        #src/cncadaptor_tests.cpp
  )
add_executable(romi_cnc_unit_tests ${SRCS_OQUAM})
target_link_libraries(romi_cnc_unit_tests cnc romi gtest gmock)

add_test(
    NAME romi_cnc_unit_tests
    COMMAND romi_cnc_unit_tests
)

##

Message (${CMAKE_BINARY_DIR})

if(BUILD_COVERAGE)
    SETUP_TARGET_FOR_COVERAGE_LCOV(
            NAME romi_cnc_unit_tests_coverage
            EXECUTABLE ctest -V ${n_cores} -R "libromi"
            EXCLUDE '*wrapper*' "src/Linux.cpp" "include/*" '${CMAKE_BINARY_DIR}/libr/*'
            DEPENDENCIES    romi_cnc_unit_tests
    )
endif()

