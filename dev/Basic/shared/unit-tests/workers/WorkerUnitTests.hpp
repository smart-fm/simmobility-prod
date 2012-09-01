/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for Workers.
 * \author Seth N. Hetu
 */
class WorkerUnitTests : public CppUnit::TestFixture
{
public:
	///Test basic worker functionality
	void test_SimpleWorkers();

	///Test what happens when workers have different, interacting granularities.
	void test_MultipleGranularities();

	///Test agents scheduled for 3ms ticks where the simulation ends at 5ms (not 6)
	void test_OddGranularities();

	//TODO: Test agents with different start times.

	//TODO: Test each agent's update after each time tick.

	//TODO: Test long-running, multiple-granularity work groups (add +timeTick each time, since
	//      adding +1 can work even if synchronization isn't working right).




private:
    CPPUNIT_TEST_SUITE(WorkerUnitTests);
		CPPUNIT_TEST(test_SimpleWorkers);
        CPPUNIT_TEST(test_MultipleGranularities);
        CPPUNIT_TEST(test_OddGranularities);
    CPPUNIT_TEST_SUITE_END();
};

}
