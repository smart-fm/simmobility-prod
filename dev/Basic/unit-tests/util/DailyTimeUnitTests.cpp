/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <cmath>
#include <limits>

#include "util/DailyTime.hpp"

#include "DailyTimeUnitTests.hpp"

using namespace sim_mob;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::DailyTimeUnitTests);

void unit_tests::DailyTimeUnitTests::test_invalid_DailyTime_constructor()
{
	try {
		DailyTime a("ABCDEFG");
		CPPUNIT_FAIL("Nonsensical input test failed.");
	} catch (std::exception& ex) { }
}


