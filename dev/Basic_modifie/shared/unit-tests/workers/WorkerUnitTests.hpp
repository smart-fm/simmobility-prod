//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for Workers. Note that *all* of these tests have the potential to deadlock, so
 *   you should have a way of manually killing the test program if necessary.
 *
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

	///Test agents with different start times.
	void test_AgentStartTimes();

	///Tests agent/buffered behavior at sub-micro-tick resolutions, and across multiple ticks.
	void test_UpdatePhases();

	//Long-running, multiple granularity test with a time-tick dependent value added each time
	// (to avoid accidentally correct answers).
	void test_MultiGroupInteraction();


private:
    CPPUNIT_TEST_SUITE(WorkerUnitTests);
		CPPUNIT_TEST(test_SimpleWorkers);
        CPPUNIT_TEST(test_MultipleGranularities);
        CPPUNIT_TEST(test_OddGranularities);
        CPPUNIT_TEST(test_AgentStartTimes);
        CPPUNIT_TEST(test_UpdatePhases);
        CPPUNIT_TEST(test_MultiGroupInteraction);
    CPPUNIT_TEST_SUITE_END();
};

}
