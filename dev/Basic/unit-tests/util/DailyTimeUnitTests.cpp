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


void unit_tests::DailyTimeUnitTests::test_time_t_DailyTime_constructor()
{
	//TODO: Not yet implemented.
}


void unit_tests::DailyTimeUnitTests::test_optional_seconds_DailyTime_constructor()
{
	DailyTime a("08:30:00");
	DailyTime b("08:30");
	CPPUNIT_ASSERT_MESSAGE("Optional seconds test failed.", a.isEqual(b));
}


void unit_tests::DailyTimeUnitTests::test_default_DailyTime_constructor()
{
	DailyTime a("00:00:00");
	DailyTime b;
	CPPUNIT_ASSERT_MESSAGE("Default constructor test failed.", a.isEqual(b));
}

void unit_tests::DailyTimeUnitTests::test_nonoptional_hoursminutes_DailyTime_constructor()
{
	//No fractions
	try {
		DailyTime a("08");
		CPPUNIT_FAIL("Non-optional seconds test failed.");
	} catch (std::exception& ex) { }

	//With fractions
	try {
		DailyTime a("08.5");
		CPPUNIT_FAIL("Mandatory minutes test failed.");
	} catch (std::exception& ex) { }
	try {
		DailyTime b("08:30.5");
		CPPUNIT_FAIL("Mandatory minutes test failed.");
	} catch (std::exception& ex) { }

}

void unit_tests::DailyTimeUnitTests::test_DailyTime_comparison_fine()
{
	DailyTime a("08:30:00");
	DailyTime b("08:30:01");
	CPPUNIT_ASSERT_MESSAGE("Single second after test failed: equality.", !a.isEqual(b));
	CPPUNIT_ASSERT_MESSAGE("Single second after test failed: inequality.", a.isBefore(b));
	CPPUNIT_ASSERT_MESSAGE("Single second after test failed: inequality.", !a.isAfter(b));
	CPPUNIT_ASSERT_MESSAGE("Single second after test failed: inequality.", !b.isBefore(a));
	CPPUNIT_ASSERT_MESSAGE("Single second after test failed: inequality.", b.isAfter(a));
}

void unit_tests::DailyTimeUnitTests::test_DailyTime_comparison_coarse()
{
	DailyTime a("09:30:00");
	DailyTime b("08:30:00");
	CPPUNIT_ASSERT_MESSAGE("Hour before test failed: equality.", !a.isEqual(b));
	CPPUNIT_ASSERT_MESSAGE("Hour before test failed: inequality.", !a.isBefore(b));
	CPPUNIT_ASSERT_MESSAGE("Hour before test failed: inequality.", a.isAfter(b));
	CPPUNIT_ASSERT_MESSAGE("Hour before test failed: inequality.", b.isBefore(a));
	CPPUNIT_ASSERT_MESSAGE("Hour before test failed: inequality.", !b.isAfter(a));
}

void unit_tests::DailyTimeUnitTests::test_DailyTime_comparison_superfine()
{
	{
	DailyTime a("08:30:00.5");
	DailyTime b("08:30:00");
	CPPUNIT_ASSERT_MESSAGE("Half-second before test failed: equality.", !a.isEqual(b));
	CPPUNIT_ASSERT_MESSAGE("Half-second before test failed: inequality.", !a.isBefore(b));
	CPPUNIT_ASSERT_MESSAGE("Half-second before test failed: inequality.", a.isAfter(b));
	CPPUNIT_ASSERT_MESSAGE("Half-second before test failed: inequality.", b.isBefore(a));
	CPPUNIT_ASSERT_MESSAGE("Half-second before test failed: inequality.", !b.isAfter(a));
	}

	{
	DailyTime a("08:30:00.1");
	DailyTime b("08:30:00.3");
	CPPUNIT_ASSERT_MESSAGE("Sub-second before test failed: equality.", !a.isEqual(b));
	CPPUNIT_ASSERT_MESSAGE("Sub-second before test failed: inequality.", a.isBefore(b));
	CPPUNIT_ASSERT_MESSAGE("Sub-second before test failed: inequality.", !a.isAfter(b));
	CPPUNIT_ASSERT_MESSAGE("Sub-second before test failed: inequality.", !b.isBefore(a));
	CPPUNIT_ASSERT_MESSAGE("Sub-second before test failed: inequality.", b.isAfter(a));
	}
}




