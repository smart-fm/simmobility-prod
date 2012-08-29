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




private:
    CPPUNIT_TEST_SUITE(WorkerUnitTests);
        CPPUNIT_TEST(test_SimpleWorkers);
    CPPUNIT_TEST_SUITE_END();
};

}
