#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock_cnccontroller.h"
#include "mock_clock.h"
#include "CNC.h"

#include <rcom/Linux.h>

#include "util/ClockAccessor.h"
#include "data_provider/RomiDeviceData.h"
#include "data_provider/SoftwareVersion.h"
#include "session/Session.h"
#include "data_provider/Gps.h"
#include "data_provider/GpsLocationProvider.h"

using namespace std;
using namespace testing;
using namespace romi;

class cnc_tests : public ::testing::Test
{
protected:
        
        int32_t position[3] =  {0, 0, 0};
        
        const double xmin[3] =  {0, 0, 0};
        const double xmax[3] =  {0.5, 0.5, 0};
        const double vmax[3] =  {0.1, 0.1, 0.01};
        const double amax[3] =  {0.2, 0.2, 0.2};
        const double scale[3] = {40000, 40000, 100000};
        const double slice_interval = 0.020;
        const double max_slice_interval = 4.0;
        const romi::AxisIndex homing[3] = {romi::kAxisX, romi::kAxisY, romi::kNoAxis};
        
        CNCRange range;
        CNCSettings settings;
        MockCNCController controller;
        
	cnc_tests()
                : range(xmin, xmax),
                  settings(range, vmax, amax, scale, 0.005, slice_interval,
                           max_slice_interval, homing, kHomingDefault),
                  controller(),
                  linux(),
                  romiDeviceData("CNC", "0001"),
                  softwareVersion(),
                  gps(),
                  locationPrivider(),
                  session_directory("./session-directory"),
                  observation_id("observation_id"),
                  mockClock_(std::make_shared<romi::MockClock>()) {
                locationPrivider  = std::make_unique<GpsLocationProvider>(gps);
        }

	~cnc_tests() override = default;

	void SetUp() override {
                position[0] = 0;
                position[1] = 0;
                position[2] = 0;
                romi::ClockAccessor::SetInstance(mockClock_);
                std::string date_time("01012025");
                EXPECT_CALL(*mockClock_, datetime_compact_string)
                                .Times(AtLeast(1))
                                .WillRepeatedly(Return(date_time));
        }

	void TearDown() override {
                romi::ClockAccessor::SetInstance(nullptr);
	}

    void HomingTestsSetUp() {
        EXPECT_CALL(controller, get_position(NotNull()))
                .WillRepeatedly(DoAll(SetArrayArgument<0>(position, position+3),
                                      Return(true)));
        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .WillRepeatedly(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));
        EXPECT_CALL(controller, enable())
                .WillRepeatedly(Return(true));
        EXPECT_CALL(controller, synchronize(_))
                .WillRepeatedly(Return(true));
        EXPECT_CALL(controller, move(_,_,_,_))
                .WillRepeatedly(Return(true));
    }

        void DefaultSetUp() {
                EXPECT_CALL(controller, homing())
                        .WillRepeatedly(Return(true));
                HomingTestsSetUp();
        }
        
public:
        
        bool get_position(int32_t *p) {
                for (int i = 0; i < 3; i++)
                        p[i] = position[i];
                return true;
        }
        
        bool move(int16_t millis, int16_t x, int16_t y, int16_t z) {
                r_debug("move(dt=%d, dx=%d, dy=%d, dz=%d)", millis, x, y, z);
                position[0] += x;
                position[1] += y;
                position[2] += z;
                return true;
        }

        rcom::Linux linux;
        RomiDeviceData romiDeviceData;
        SoftwareVersion softwareVersion;
        romi::Gps gps;
        std::unique_ptr<ILocationProvider> locationPrivider;
        const std::string session_directory;
        const std::string observation_id;
        std::shared_ptr<romi::MockClock> mockClock_;
};

TEST_F(cnc_tests, constructor_calls_set_homing_axes)
{
        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);

        CNC cnc(controller, settings, session);
}

TEST_F(cnc_tests, constructor_throws_exception_when_set_homing_axes_fails)
{
        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(false));
        
        try {
                romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
                session.start(observation_id);
                CNC cnc(controller, settings, session);
                FAIL() << "Excpected a runtime error";
                
        } catch (std::runtime_error& e) {
                // OK
        }
}

TEST_F(cnc_tests, pause_activity_calls_controller_1)
{
        DefaultSetUp();
        EXPECT_CALL(controller, pause_activity())
                .Times(1)
                .WillOnce(Return(true));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.pause_activity();
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, pause_activity_calls_controller_2)
{
        DefaultSetUp();
        EXPECT_CALL(controller, pause_activity())
                .Times(1)
                .WillOnce(Return(false));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.pause_activity();
        ASSERT_EQ(success, false);
}

TEST_F(cnc_tests, continue_activity_calls_controller_1)
{
        DefaultSetUp();
        EXPECT_CALL(controller, continue_activity())
                .Times(1)
                .WillOnce(Return(true));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.continue_activity();
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, continue_activity_calls_controller_2)
{
        DefaultSetUp();
        EXPECT_CALL(controller, continue_activity())
                .Times(1)
                .WillOnce(Return(false));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.continue_activity();
        ASSERT_EQ(success, false);
}

TEST_F(cnc_tests, reset_calls_controller_1)
{
        DefaultSetUp();
        EXPECT_CALL(controller, reset_activity())
                .Times(1)
                .WillOnce(Return(true));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.reset_activity();
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, reset_calls_controller_2)
{
        DefaultSetUp();
        EXPECT_CALL(controller, reset_activity())
                .Times(1)
                .WillOnce(Return(false));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.reset_activity();
        ASSERT_EQ(success, false);
}

TEST_F(cnc_tests, constructor_copies_range)
{
        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);

        CNCRange range;
        cnc.get_range(range);

        ASSERT_EQ(range.xmin(), xmin[0]);
        ASSERT_EQ(range.xmax(), xmax[0]);
        ASSERT_EQ(range.ymin(), xmin[1]);
        ASSERT_EQ(range.ymax(), xmax[1]);
        ASSERT_EQ(range.zmin(), xmin[2]);
        ASSERT_EQ(range.zmax(), xmax[2]);
}

TEST_F(cnc_tests, moveto_returns_error_when_speed_is_invalid)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        
        ASSERT_EQ(cnc.moveto(0.1, 0.0, 0.0, 1.1), false);
        ASSERT_EQ(cnc.moveto(0.1, 0.0, 0.0, -0.1), false);
}

TEST_F(cnc_tests, moveto_returns_error_when_position_is_invalid)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        ASSERT_EQ(cnc.moveto(range.xmin()-0.1, 0.0, 0.0, 0.1), false);
        ASSERT_EQ(cnc.moveto(range.xmax()+0.1, 0.0, 0.0, 0.1), false);
        ASSERT_EQ(cnc.moveto(0.0, range.ymin()-0.1, 0.0, 0.1), false);
        ASSERT_EQ(cnc.moveto(0.0, range.ymax()+0.1, 0.0, 0.1), false);
        ASSERT_EQ(cnc.moveto(0.0, 0.0, range.zmin()-0.1, 0.1), false);
        ASSERT_EQ(cnc.moveto(0.0, 0.0, range.zmax()+0.1, 0.1), false);
}

TEST_F(cnc_tests, returns_false_when_get_position_fails)
{
        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .Times(1)
                .WillOnce(Return(false));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, false);        
}

TEST_F(cnc_tests, returns_false_when_moveto_fails)
{
        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .WillRepeatedly(Invoke(this, &cnc_tests::get_position));
        EXPECT_CALL(controller, move(_,_,_,_))
                .Times(1)
                .WillOnce(Return(false));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, false);        
}

TEST_F(cnc_tests, returns_false_when_synchronize_fails)
{
        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .WillRepeatedly(Invoke(this, &cnc_tests::get_position));
        EXPECT_CALL(controller, move(_,_,_,_))
                .WillRepeatedly(Return(true));
        EXPECT_CALL(controller, synchronize(_))
                .Times(1)
                .WillOnce(Return(false));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, false);        
}

TEST_F(cnc_tests, test_cnc_moveto)
{
        InSequence seq;

        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));
        EXPECT_CALL(controller, get_position(_))
                .WillRepeatedly(Invoke(this, &cnc_tests::get_position));
        EXPECT_CALL(controller, move(_,_,_,_))
                .WillRepeatedly(Invoke(this, &cnc_tests::move));
        EXPECT_CALL(controller, synchronize(_))
                .Times(1)
                .WillOnce(Return(true));

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);
        bool success = cnc.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
        ASSERT_EQ(position[0], 4000);        
}

TEST_F(cnc_tests, test_cnc_moveto_2)
{
        InSequence seq;

        EXPECT_CALL(controller, set_homing_axes(_,_,_))
                .Times(1)
                .WillOnce(Return(true));
        EXPECT_CALL(controller, spindle(0))
                .WillOnce(Return(true));
        for (int i = 0; i < 2; i++) {
                EXPECT_CALL(controller, get_position(_))
                        .WillRepeatedly(Invoke(this, &cnc_tests::get_position));
                EXPECT_CALL(controller, move(_,_,_,_))
                        .WillRepeatedly(Invoke(this, &cnc_tests::move));
                EXPECT_CALL(controller, synchronize(_))
                        .Times(1)
                        .WillOnce(Return(true));
        }

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start(observation_id);
        CNC cnc(controller, settings, session);

        bool success = cnc.moveto(0.1, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
        
        success = cnc.moveto(0.0, 0.0, 0.0, 0.3);
        ASSERT_EQ(success, true);
        
        ASSERT_EQ(position[0], 0);        
}

TEST_F(cnc_tests, test_cnc_travel_empty_path)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_empty");
        CNC cnc(controller, settings, session);

        Path path;
        bool success = cnc.travel(path, 0.3);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_square)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_square");
        CNC cnc(controller, settings, session);

        Path path;
        v3 p0(0.1, 0.0, 0.0);
        v3 p1(0.1, 0.1, 0.0);
        v3 p2(0.0, 0.1, 0.0);
        v3 p3(0.0, 0.0, 0.0);
        path.push_back(p0);
        path.push_back(p1);
        path.push_back(p2);
        path.push_back(p3);

        bool success = cnc.travel(path, 0.3);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_square_fast)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_fast");
        CNC cnc(controller, settings, session);

        Path path;
        v3 p0(0.1, 0.0, 0.0);
        v3 p1(0.1, 0.1, 0.0);
        v3 p2(0.0, 0.1, 0.0);
        v3 p3(0.0, 0.0, 0.0);
        path.push_back(p0);
        path.push_back(p1);
        path.push_back(p2);
        path.push_back(p3);

        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_snake)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_snake");
        CNC cnc(controller, settings, session);

        Path path;
        int N = 10;
        for (int i = 1; i <= N; i++) {
                v3 p0(i * 0.01, (i-1) * 0.01, 0.0);
                path.push_back(p0);
                v3 p1(i * 0.01, i * 0.01, 0.0);
                path.push_back(p1);
        }
        
        v3 p(0.0, 0.0, 0.0);
        path.push_back(p);

        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_snake_2)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_snake_2");
        CNC cnc(controller, settings, session);

        int N = 11;
        Path path;
        double x = 0.0;
        double y = 0.0;
        for (int i = 1; i <= N; i++) {
                int n = N + 1 - i;
                double len = 0.001 * n;
                v3 p0(x + len, y, 0.0);
                path.push_back(p0);
                v3 p1(x + len, y + len, 0.0);
                path.push_back(p1);
                x += len;
                y += len;
        }
        
        v3 p(0.0, 0.0, 0.0);
        path.push_back(p);

        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_round_trip)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_round_trip");
        CNC cnc(controller, settings, session);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_collinear)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_round_collinear");
        CNC cnc(controller, settings, session);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.2, 0.0, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_large_displacement)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_large_displacement");
        CNC cnc(controller, settings, session);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.1, 0.07, 0.0));
        path.push_back(v3(0.2, 0.07, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_small_displacement)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_small_displacement");
        CNC cnc(controller, settings, session);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.1, 0.04, 0.0));
        path.push_back(v3(0.2, 0.04, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_tiny_displacement)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData, softwareVersion, std::move(locationPrivider));
        session.start("travel_tiny_displacement");
        CNC cnc(controller, settings, session);
        //cnc.set_file_cabinet(&debug_session);

        Path path;
        path.push_back(v3(0.0, 0.0, 0.0));
        path.push_back(v3(0.1, 0.0, 0.0));
        path.push_back(v3(0.1, 0.005, 0.0));
        path.push_back(v3(0.2, 0.005, 0.0));
        path.push_back(v3(0.0, 0.0, 0.0));
        
        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, test_cnc_travel_zigzag)
{
        DefaultSetUp();

        romi::Session session(linux, session_directory, romiDeviceData,
                              softwareVersion, std::move(locationPrivider));
        session.start("travel_zigzag");
        CNC cnc(controller, settings, session);

        Path path;
        v3 p(0.0, 0.0, 0.0);
        
        for (int i = 1; i <= 3; i++) {
                p.y(p.y() + 0.01);
                path.push_back(p);
                
                p.x(p.x() + 0.1);
                path.push_back(p);
                
                p.y(p.y() + 0.01);
                path.push_back(p);

                p.x(p.x() - 0.1);
                path.push_back(p);
        }
        
        path.push_back(v3(0.0, 0.0, 0.0));

        bool success = cnc.travel(path, 1.0);
        ASSERT_EQ(success, true);
}

TEST_F(cnc_tests, power_up_calls_homing_after_construct)
{
    // Arrange
    HomingTestsSetUp();
    EXPECT_CALL(controller, homing())
            .WillOnce(Return(true));

    romi::Session session(linux, session_directory, romiDeviceData,
                          softwareVersion, std::move(locationPrivider));
    session.start("travel_zigzag");
    CNC cnc(controller, settings, session);

    // Act
    auto actual = cnc.power_up();

    // Assert
    ASSERT_EQ(actual, true);
}

TEST_F(cnc_tests, power_up_calls_homing_only_once_when_no_movement)
{
    // Arrange
    HomingTestsSetUp();
    EXPECT_CALL(controller, homing())
            .WillOnce(Return(true));

    romi::Session session(linux, session_directory, romiDeviceData,
                          softwareVersion, std::move(locationPrivider));
    session.start("no_movement");
    CNC cnc(controller, settings, session);

    // Act
    auto actual = cnc.power_up();
    actual = cnc.power_up();
    actual = cnc.power_up();

    // Assert
    ASSERT_EQ(actual, true);
}

TEST_F(cnc_tests, power_up_calls_homing_after_moveto)
{
    // Arrange
    HomingTestsSetUp();

    EXPECT_CALL(controller, homing())
    .Times(2)
    .WillRepeatedly(Return(true));

    romi::Session session(linux, session_directory, romiDeviceData,
                          softwareVersion, std::move(locationPrivider));
    session.start("homing_test");
    CNC cnc(controller, settings, session);

    // Act
    auto actual = cnc.power_up();
    actual = cnc.moveto(1, 1, 1, 0.1);

    actual = cnc.power_up();

    // Assert
    ASSERT_EQ(actual, true);
}

// TEST_F(cnc_tests, power_up_calls_homing_after_moveat)
// {
//     // Arrange
//     HomingTestsSetUp();

//     EXPECT_CALL(controller, homing())
//             .Times(2)
//             .WillRepeatedly(Return(true));
//     EXPECT_CALL(controller, moveat(_,_,_))
//             .WillOnce(Return(true));

//     romi::Session session(linux, session_directory, romiDeviceData,
//                           softwareVersion, std::move(locationPrivider));
//     session.start("homing_test");
//     CNC cnc(controller, settings, session);

//     // Act
//     auto actual = cnc.power_up();
//     actual = cnc.moveat(1, 1, 1);
//     actual = cnc.power_up();

//     // Assert
//     ASSERT_EQ(actual, true);
// }

TEST_F(cnc_tests, power_up_calls_homing_after_spindle)
{
    // Arrange
    HomingTestsSetUp();

    EXPECT_CALL(controller, homing())
            .Times(2)
            .WillRepeatedly(Return(true));
    EXPECT_CALL(controller, spindle(1.0))
            .WillOnce(Return(true));

    romi::Session session(linux, session_directory, romiDeviceData,
                          softwareVersion, std::move(locationPrivider));
    session.start("homing_test");
    CNC cnc(controller, settings, session);

    // Act
    auto actual = cnc.power_up();
    actual = cnc.spindle(1.0);
    actual = cnc.power_up();

    // Assert
    ASSERT_EQ(actual, true);
}

TEST_F(cnc_tests, power_up_calls_homing_after_travel)
{
    // Arrange
    HomingTestsSetUp();

    EXPECT_CALL(controller, homing())
            .Times(2)
            .WillRepeatedly(Return(true));

    romi::Session session(linux, session_directory, romiDeviceData,
                          softwareVersion, std::move(locationPrivider));
    session.start("homing_test");
    CNC cnc(controller, settings, session);

    // Act
    auto actual = cnc.power_up();
    Path travel_path;
    actual = cnc.travel(travel_path, 1.0);
    actual = cnc.power_up();

    // Assert
    ASSERT_EQ(actual, true);
}
