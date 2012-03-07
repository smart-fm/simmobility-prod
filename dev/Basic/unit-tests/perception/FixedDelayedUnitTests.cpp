/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <cmath>
#include <limits>

#include "FixedDelayedUnitTests.hpp"

#include "perception/FixedDelayed.hpp"

using namespace sim_mob;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::FixedDelayedUnitTests);

void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_simple_set_get()
{
	//Create a FixedDelayed Type and wait until well past its sensation delay
	FixedDelayed<int> x(100); //Delay 100ms
	x.delay(42, 200); //Sensed at 200ms. Will be visible at 300ms
	int res = x.sense(350, 0);
	CPPUNIT_ASSERT_MESSAGE("Simple FixedDelayed retrieval failed.", res==42);
}

void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_bad_retrieve()
{
	//Create a FixedDelayed Type, retrieve it too early.
	FixedDelayed<int> x(100); //Delay 100ms
	x.delay(42, 200); //Sensed at 200ms. Will be visible at 300ms
	try {
		x.sense(250, 0);
		CPPUNIT_FAIL("FixedDelayed returned a result before it was ready.");
	} catch (std::exception& ex) {}
}





