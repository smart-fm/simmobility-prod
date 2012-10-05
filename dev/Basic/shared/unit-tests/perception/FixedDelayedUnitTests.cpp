/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <cmath>
#include <limits>
#include <string>
#include <sstream>

#include "FixedDelayedUnitTests.hpp"

#include "perception/FixedDelayed.hpp"

//Just adding this to make sure linking works w/ the template function (it should).
#include "util/DynamicVector.hpp"

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


namespace {
//Simple class for testing
class MyPoint {
public:
	MyPoint(int x, int y) : x(x), y(y) {}
	int getX() { return x; }
	int getY() { return y; }
private:
	int x;
	int y;
};
} //End anon namespace
void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_class_store()
{
	FixedDelayed<MyPoint*> store(10);
	store.update(1);
	store.delay(new MyPoint(1,2));
	store.update(2);
	store.delay(new MyPoint(3,4));
	store.update(3);
	store.delay(new MyPoint(5,6));
	store.update(10);
	CPPUNIT_ASSERT_MESSAGE("Storing classes failed (1).", !store.can_sense());

	//Sensible checks
	store.update(11);
	CPPUNIT_ASSERT_MESSAGE("Storing classes failed (2).", store.can_sense() && (store.sense()->getX()==1) && (store.sense()->getY()==2));
	store.update(12);
	CPPUNIT_ASSERT_MESSAGE("Storing classes failed (3).", store.can_sense() && (store.sense()->getX()==3) && (store.sense()->getY()==4));
	store.update(13);
	CPPUNIT_ASSERT_MESSAGE("Storing classes failed (4).", store.can_sense() && (store.sense()->getX()==5) && (store.sense()->getY()==6));

	//Make sure this value persists as the last known value.
	store.update(1000);
	CPPUNIT_ASSERT_MESSAGE("Storing classes failed (5).", store.can_sense() && (store.sense()->getX()==5) && (store.sense()->getY()==6));
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
	//The default constructor is only used so that our list<> templates compile correctly (it's magic that it worked before this).
	//  Actually, this should never be used.
	DelStruct() : refCount(nullptr) {}

	explicit DelStruct(int& rc) : refCount(&rc) {
		(*refCount) = 1;
	}
	DelStruct(const DelStruct& copy) : refCount(copy.refCount) {
		if (refCount) {
			(*refCount)++; //Needed for vector<> to not mess up our results.
		}
	}
	~DelStruct() {
		if (refCount) {
			(*refCount)--;
		}
	}

private:
	//Check the value externally.
	int* refCount;
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


namespace {
//Represent a series of views
struct View {
	uint32_t time;
	int value;
};
const View Sequence1[] = {{0, 100}, {100, 98}, {200, 102}, {300, 97}, {400, 96}, {500, 1}, {600, 103}, {700, 95}, {800, 92}, {900, 99}};
const View Sequence2[] = {{0, 199}, {100, 2}, {200, 100}, /*{300, 175},*/ {400, 210}, {500, 185}, {600, 111}, {700, 122}, {800, 221}};

//Store all.
void StoreAll(FixedDelayed<int>& store, const View seq[], size_t sz) {
	for (size_t i=0; i<sz; i++) {
		store.update(seq[i].time);
		store.delay(seq[i].value);
	}
}

//Make a fail message
void MakeFail(const std::string& str1, int testID, const std::string& str2) {
	std::stringstream msg;
	msg <<str1 <<testID <<str2;
	CPPUNIT_FAIL(msg.str().c_str());
}

//Check a point
void CheckPoint(FixedDelayed<int>& store, int& testID, uint32_t timeMS, int expected, int next, const std::string& failMsg) {
	store.update(timeMS);
	if (   ((expected<0)  && store.can_sense())
		|| ((expected>=0) && (!store.can_sense() || (store.sense()!=expected)))  ) {
		MakeFail(failMsg + " (", testID, ").");
	}
	testID++;
	if (next >= 0) {
		store.delay(next);
	}
	if (   ((expected<0)  && store.can_sense())
		|| ((expected>=0) && (!store.can_sense() || (store.sense()!=expected)))  ) {
		MakeFail(failMsg + " (", testID, ").");
	}
	testID++;
}
} //End anon namespace
void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_comprehensive_sense()
{
	//Try to build a data flow similar to what Sim Mobility might see.
	// Use "StoreAll" to build the first 9 values easily.
	FixedDelayed<int> store(1000);
	StoreAll(store, Sequence1, sizeof(Sequence1)/sizeof(Sequence1[0]));

	//We now have 900ms of buffered data.
	CPPUNIT_ASSERT_MESSAGE("Comprehensive sense test failed (1).", !store.can_sense());
	store.update(1000);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive sense test failed (2).", store.can_sense() && (store.sense()==100));
	store.delay(105); //Value: {1000, 105}
	CPPUNIT_ASSERT_MESSAGE("Comprehensive sense test failed (3).", store.can_sense() && (store.sense()==100));

	//We now have 1000ms of buffered data
	for (unsigned int i=1001; i<=1099; i++) {
		store.update(i);
		if (!store.can_sense() || (store.sense()!=100)) {
			std::stringstream msg;
			msg <<"Comprehensive sense test failed (4+" <<i <<").";
			CPPUNIT_FAIL(msg.str().c_str());
		}
	}
	store.update(1100);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive sense test failed (5).", store.can_sense() && (store.sense()==98));
	store.delay(144); //Value: {1100, 144}
	CPPUNIT_ASSERT_MESSAGE("Comprehensive sense test failed (6).", store.can_sense() && (store.sense()==98));

	//Perform automatic checks for the remaining data.
	int i = 7;
	std::string msg = "Comprehensive sense test failed";
	CheckPoint(store, i, 1200, 102, 145, msg);
	CheckPoint(store, i, 1300, 97, 141, msg);
	CheckPoint(store, i, 1400, 96, 130, msg);
	CheckPoint(store, i, 1500, 1, 122, msg);   //Stop storing data after this test
	CheckPoint(store, i, 1600, 103, -1, msg);
	CheckPoint(store, i, 1700, 95, -1, msg);
	CheckPoint(store, i, 1800, 92, -1, msg);
	CheckPoint(store, i, 1900, 99, -1, msg);   //Last automatic value; now just wind down.
	CheckPoint(store, i, 2000, 105, -1, msg);
	CheckPoint(store, i, 2100, 144, -1, msg);
	CheckPoint(store, i, 2200, 145, -1, msg);
	CheckPoint(store, i, 2300, 141, -1, msg);
	CheckPoint(store, i, 2400, 130, -1, msg);

	//Last value. Double-check boundaries
	CheckPoint(store, i, 2499, 130, -1, msg);
	CheckPoint(store, i, 2500, 122, -1, msg);
	CheckPoint(store, i, 2501, 122, -1, msg);
	CheckPoint(store, i, 9999, 122, -1, msg);
}



void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_diminishing_reaction_time()
{
	//First, load and store up to the limit
	FixedDelayed<int> store(10);
	int i = 1;
	std::string msg = "Diminishing test failed";
	CheckPoint(store, i, 0, -1, 200, msg);
	CheckPoint(store, i, 2, -1, 300, msg);
	CheckPoint(store, i, 4, -1, 100, msg);
	CheckPoint(store, i, 6, -1, 900, msg);
	CheckPoint(store, i, 8, -1, 1000, msg);
	CheckPoint(store, i, 10, 200, 600, msg);

	//Now shorten the reaction time
	store.set_delay(6);
	CPPUNIT_ASSERT_MESSAGE("Diminishing test failed (13).", store.can_sense() && (store.sense()==100));

	//Shorten to an odd number
	store.set_delay(3);
	CPPUNIT_ASSERT_MESSAGE("Diminishing test failed (14).", store.can_sense() && (store.sense()==900));

	//Shorten to nothing
	store.set_delay(0);
	CPPUNIT_ASSERT_MESSAGE("Diminishing test failed (15).", store.can_sense() && (store.sense()==600));
}


void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_expanding_reaction_time()
{
	//Load up to a (decreased) limit
	FixedDelayed<int> store(10);
	store.set_delay(5);
	int i = 1;
	std::string msg = "Expanding test failed";
	CheckPoint(store, i, 0, -1, 199, msg);
	CheckPoint(store, i, 2, -1, 299, msg);
	CheckPoint(store, i, 4, -1, 399, msg);
	CheckPoint(store, i, 5, 199, -1, msg);
	CheckPoint(store, i, 6, 199, -1, msg);
	CheckPoint(store, i, 7, 299, -1, msg);

	//We are directly on a sense-able event. Increase reaction time by 1 and check.
	store.set_delay(6);
	CPPUNIT_ASSERT_MESSAGE("Expanding test failed (13).", store.can_sense() && (store.sense()==199));
	store.set_delay(7);
	CPPUNIT_ASSERT_MESSAGE("Expanding test failed (14).", store.can_sense() && (store.sense()==199));
	store.set_delay(5);
	CPPUNIT_ASSERT_MESSAGE("Expanding test failed (15).", store.can_sense() && (store.sense()==299));

	//Now update to one off from a sense-able event and try again.
	i = 16;
	CheckPoint(store, i, 8, 299, -1, msg);
	store.set_delay(6);
	CPPUNIT_ASSERT_MESSAGE("Expanding test failed (18).", store.can_sense() && (store.sense()==299));
	store.set_delay(7);
	CPPUNIT_ASSERT_MESSAGE("Expanding test failed (19).", store.can_sense() && (store.sense()==199));
	store.set_delay(5);
	CPPUNIT_ASSERT_MESSAGE("Expanding test failed (20).", store.can_sense() && (store.sense()==299));

	//Set back to max and check.
	store.set_delay(10);
	CPPUNIT_ASSERT_MESSAGE("Expanding test failed (21).", !store.can_sense());
}



void unit_tests::FixedDelayedUnitTests::test_FixedDelayed_comprehensive_variable_reaction()
{
	//Try to present data that an Agent might see approaching a traffic light
	//(e.g., perception improves, then degrades as it moves away)
	FixedDelayed<int> store(1000);
	store.set_delay(900);
	StoreAll(store, Sequence2, sizeof(Sequence2)/sizeof(Sequence2[0]));

	//We now have 0..800 added. Ensure we can't detect anything, then upgrade to 900.
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (1).", !store.can_sense());
	store.update(900);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (2).", store.can_sense()&&(store.sense()==199));
	store.delay(223); //Value: {900, 223}
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (3).", store.can_sense()&&(store.sense()==199));

	//Go back to 1000s delay, ensure we can't sense, and update.
	store.set_delay(1000);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (4).", !store.can_sense());
	store.update(1000);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (5).", store.can_sense()&&(store.sense()==199));
	store.delay(544); //Value: {1000, 544}
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (6).", store.can_sense()&&(store.sense()==199));

	//Flip back and forth between 900 and 1000 without updating.
	store.set_delay(900);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (7).", store.can_sense()&&(store.sense()==2));
	store.set_delay(1000);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (8).", store.can_sense()&&(store.sense()==199));
	store.set_delay(900);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (9).", store.can_sense()&&(store.sense()==2));
	store.set_delay(1000);
	CPPUNIT_ASSERT_MESSAGE("Comprehensive variable test failed (10).", store.can_sense()&&(store.sense()==199));

	//Add a few more values at a sensation of 900
	store.set_delay(900);
	int i = 11;
	std::string msg = "Comprehensive variable test failed";
	CheckPoint(store, i, 1100, 100, 399, msg);
	CheckPoint(store, i, 1200, 100, 656, msg);
	CheckPoint(store, i, 1300, 210, 787, msg); //"skipped" message
	CheckPoint(store, i, 1400, 185, 900, msg);
	CheckPoint(store, i, 1500, 111, 338, msg);

	//Approaching traffic light: sensation increases by 50 for the next few values
	store.set_delay(850);
	CheckPoint(store, i, 1600, 122, -1, msg);
	store.set_delay(800);
	CheckPoint(store, i, 1700, 223, 1001, msg);
	store.set_delay(750);
	CheckPoint(store, i, 1800, 544, 987, msg);
	store.set_delay(700);
	CheckPoint(store, i, 1900, 656, 634, msg);

	//Much closer: sensation increases by 200 for the next few values
	store.set_delay(500);
	CheckPoint(store, i, 2000, 338, 20, msg);
	store.set_delay(300);
	CheckPoint(store, i, 2100, 987, 20, msg);

	//Leaving the traffic light:  sensation decreases by 50 for the next few values
	store.set_delay(350);
	CheckPoint(store, i, 2200, 987, 20, msg);
	store.set_delay(400);
	CheckPoint(store, i, 2300, 634, -1, msg);
	store.set_delay(450);
	CheckPoint(store, i, 2400, 634, 20, msg);
	store.set_delay(500);
	CheckPoint(store, i, 2400, 634, -1, msg);

	//Moving further away: sensation decreases by 200 back to 900
	//NOTE: Decreasing sensation this quickly is probably not a good idea from a modelling perspective,
	//      since the user is now sensing data which is OLDER than that which he sensed one time tick prior.
	//      However, that is an error of *modelling*, and probably one that we don't need to handle in FixedDelayed<>.
	store.set_delay(700);
	CheckPoint(store, i, 2500, 987, 20, msg);
	store.set_delay(900);
	CheckPoint(store, i, 2600, 1001, -1, msg);

	//Check a few more points at 900, just to be sure
	CheckPoint(store, i, 2700, 987, 20, msg);
	CheckPoint(store, i, 2800, 634, 20, msg);
}




