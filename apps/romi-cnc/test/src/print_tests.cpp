#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "json.hpp"
#include "SmoothPath.h"
#include "print.h"

using namespace std;
using namespace testing;
using namespace romi;

class print_tests : public ::testing::Test
{
protected:

        double vmax[3] = { 1.0, 1.0, 0.0};
        double amax[3] = { 1.0, 1.0, 1.0};
        double deviation = 0.01;
        double period = 0.100;
        double maxlen = 32.0;
        
	print_tests() {
	}

	~print_tests() override = default;

	void SetUp() override {
	}

	void TearDown() override {
	}
};

TEST_F(print_tests, test_valid_json_without_slices)
{
        // Arrange
        v3 start_position(0, 0, 0);
        SmoothPath script(start_position);
        script.moveto(0.10, 0.00, 0.0, 1.0);
        script.moveto(0.00, 0.00, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);

        rcom::MemBuffer text;
        try {
                print(script, text, false);
                nlohmann::json json = nlohmann::json::parse(text.tostring());
                
        } catch (nlohmann::json::exception& je) {
                printf("Generated JSON:\n%s", text.tostring().c_str());
                r_err("Parsing failed: %s", je.what());
                FAIL() << "Failed to parse the JSON";
        }

}

TEST_F(print_tests, test_valid_json_with_slices)
{
        // Arrange
        v3 start_position(0, 0, 0);
        SmoothPath script(start_position);
        script.moveto(0.10, 0.00, 0.0, 1.0);
        script.moveto(0.00, 0.00, 0.0, 1.0);
        script.convert(vmax, amax, deviation, period, maxlen);

        rcom::MemBuffer text;
        try {
                print(script, text, true);
                nlohmann::json json = nlohmann::json::parse(text.tostring());
                
        } catch (nlohmann::json::exception& je) {
                printf("Generated JSON:\n%s", text.tostring().c_str());
                r_err("Parsing failed: %s", je.what());
                FAIL() << "Failed to parse the JSON";
        }
}
