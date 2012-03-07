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
	CPPUNIT_ASSERT_MESSAGE("Exact FixedDelayed retrieval failed (1).", res==42);

	//Do a quick check starting from zero.
	FixedDelayed<int> y(100);
	y.delay(42);
	y.update(100);
	res = y.can_sense() ? y.sense() : 0;
	CPPUNIT_ASSERT_MESSAGE("Exact FixedDelayed retrieval failed (2).", res==42);
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


void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_sanity_checks()
{
	//Updating backwards
	FixedDelayed<int> x(10);
	x.update(100);
	try {
		x.update(50);
		CPPUNIT_FAIL("Sanity check failed: updating backwards.");
	} catch (std::exception& ex) {}

	//Setting the delay greater than the maximum
	FixedDelayed<int> y(100);
	try {
		y.set_delay(101);
		CPPUNIT_FAIL("Sanity check failed: allowed to exceep maximum.");
	} catch (std::exception& ex) {}
}


namespace {
struct DelStruct { //Sets flag to "true" if delete called.
	explicit DelStruct(int& refCount) : refCount(refCount) {
		refCount = 1;
	}
	DelStruct(const DelStruct& copy) : refCount(copy.refCount) {
		refCount++; //Needed for vector<> to not mess up our results.
	}
	~DelStruct() {
		refCount--;
	}
	int& refCount;
};
} //End anon namespace
void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_false_delete()
{
	{
	//Destructor called inappropriately on a pointer
	int obj1Refs = 0;
	int obj2Refs = 0;
	DelStruct* o1 = new DelStruct(obj1Refs);
	CPPUNIT_ASSERT_MESSAGE("DelStruct not initializing properly", (obj1Refs==1)&&(obj2Refs==0));
	DelStruct* o2 = new DelStruct(obj2Refs);
	{
		FixedDelayed<DelStruct*> z(10, false);
		z.update(100);
		z.delay(o1);
		z.update(105);
		z.delay(o2);
		z.update(200);
		CPPUNIT_ASSERT_MESSAGE("Unmanaged type deleted (1).", obj1Refs==1);
	}
	CPPUNIT_ASSERT_MESSAGE("Unmanaged type deleted (2).", obj2Refs==1);

	//Avoid leaking memory
	delete o1;
	delete o2;
	CPPUNIT_ASSERT_MESSAGE("Destructor not called when it was supposed to", (obj1Refs==0)&&(obj2Refs==0));
	}

	//Destructor called inappropriately on a non-pointer
	{
	int obj1Refs = 0;
	int obj2Refs = 0;
	DelStruct o1(obj1Refs);
	DelStruct o2(obj2Refs);
	{
		FixedDelayed<DelStruct> z(10, true);
		z.update(100);
		z.delay(o1);
		z.update(105);
		z.delay(o2);
		z.update(200);
		CPPUNIT_ASSERT_MESSAGE("Managed value type deleted (1).", obj1Refs==1);
	}
	CPPUNIT_ASSERT_MESSAGE("Managed value type deleted (2).", obj2Refs==1);
	}
}

void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_skipped_delete()
{
	{
	//Destructor called inappropriately on a pointer
	int obj1Refs = 0;
	int obj2Refs = 0;
	DelStruct* o1 = new DelStruct(obj1Refs);
	DelStruct* o2 = new DelStruct(obj2Refs);
	{
		FixedDelayed<DelStruct*> z(10, true);
		z.update(100);
		z.delay(o1);
		z.update(105);
		z.delay(o2);
		z.update(200);
		CPPUNIT_ASSERT_MESSAGE("Managed type leaked (1).", obj1Refs==0);
	}
	CPPUNIT_ASSERT_MESSAGE("Managed type leaked (2).", obj2Refs==0);  //Fails
	}

	//Ensure our copy semantics work; this will matter for our lists.
	int obj1Refs = 0;
	{
	DelStruct o1(obj1Refs);
	{
		DelStruct o2 = o1;
		CPPUNIT_ASSERT_MESSAGE("Copy semantics error in DelStruct (1)", obj1Refs==2);
	}
	CPPUNIT_ASSERT_MESSAGE("Copy semantics error in DelStruct (2)", obj1Refs==1);
	}
	CPPUNIT_ASSERT_MESSAGE("Copy semantics error in DelStruct (3)", obj1Refs==0);
}




