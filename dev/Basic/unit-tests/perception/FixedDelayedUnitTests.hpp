/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for the FixedDelayed class
 * \author Seth N. Hetu
 */
class FixedDelayedUnitTests : public CppUnit::TestFixture
{
public:
	///Test simple setting and retrieving
	void test_FixedDelayed_simple_set_get();

	///Test retrieving a value too early.
	void test_FixedDelayed_bad_retrieve();





private:
    CPPUNIT_TEST_SUITE(FixedDelayedUnitTests);
        CPPUNIT_TEST(test_FixedDelayed_simple_set_get);
        CPPUNIT_TEST(test_FixedDelayed_bad_retrieve);
    CPPUNIT_TEST_SUITE_END();
};

}
