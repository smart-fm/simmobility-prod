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
	FixedDelayed<int> x(100);

	x.update(200);
	x.delay(42); //Sensed at 200ms. Will be visible at 300ms

	x.update(350);
	int res = x.can_sense() ? x.sense() : 0;
	CPPUNIT_ASSERT_MESSAGE("Simple FixedDelayed retrieval failed.", res==42);
}

void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_bad_retrieve()
{
	//Create a FixedDelayed Type, retrieve it too early.
	FixedDelayed<int> x(100);

	x.update(200);
	x.delay(42); //Sensed at 200ms. Will be visible at 300ms
	try {
		x.update(250);
		x.sense();
		CPPUNIT_FAIL("FixedDelayed returned a result before it was ready.");
	} catch (std::exception& ex) {}
}


void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_exact_retrieve()
{
	//Create a FixedDelayed Type and sense it exactly on the boundary
	FixedDelayed<int> x(100);

	x.update(200);
	x.delay(42);

	x.update(300);
	int res = x.can_sense() ? x.sense() : 0;
	CPPUNIT_ASSERT_MESSAGE("Exact FixedDelayed retrieval failed.", res==42);
}


void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_zero_retrieve()
{
	//Create a FixedDelayed Type with NO delayed value
	FixedDelayed<int> x(0);

	x.update(200);
	x.delay(42);

	//This value should be immediately retrievable.
	int res = x.can_sense() ? x.sense() : 0;
	CPPUNIT_ASSERT_MESSAGE("Zero-wait FixedDelayed retrieval failed (1).", res==42);

	//Double-check that calling update twice doesn't cause unexpected behavior.
	x.update(200);
	res = x.can_sense() ? x.sense() : 0;
	CPPUNIT_ASSERT_MESSAGE("Zero-wait FixedDelayed retrieval failed (2).", res==42);
}








