//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

	///Test retrieving a value directly on the target time
	void test_FixedDelayed_exact_retrieve();

	///Test a zero-waiting-time retrieve
	void test_FixedDelayed_zero_retrieve();

	///Several common consistency checks
	void test_FixedDelayed_sanity_checks();

	///Make sure we can store classes correctly.
	void test_FixedDelayed_class_store();

	///Ensure memory isn't wrongly deleted
	void test_FixedDelayed_false_delete();

	///Ensure memory isn't leaked
	void test_FixedDelayed_skipped_delete();

	///Do a comprehensive sense check; push back several values and read them in order.
	void test_FixedDelayed_comprehensive_sense();

	///Ensure that we can shrink our delayed value without causing invalid reads.
	void test_FixedDelayed_diminishing_reaction_time();

	///Ensure that we can recover back to the maximum after shrinking our reaction time.
	void test_FixedDelayed_expanding_reaction_time();

	///Perform a comprehensive test of variable reaction time.
	void test_FixedDelayed_comprehensive_variable_reaction();





private:
    CPPUNIT_TEST_SUITE(FixedDelayedUnitTests);
        CPPUNIT_TEST(test_FixedDelayed_simple_set_get);
        CPPUNIT_TEST(test_FixedDelayed_bad_retrieve);
        CPPUNIT_TEST(test_FixedDelayed_exact_retrieve);
        CPPUNIT_TEST(test_FixedDelayed_zero_retrieve);
        CPPUNIT_TEST(test_FixedDelayed_sanity_checks);
        CPPUNIT_TEST(test_FixedDelayed_class_store);
        CPPUNIT_TEST(test_FixedDelayed_false_delete);
        CPPUNIT_TEST(test_FixedDelayed_skipped_delete);
        CPPUNIT_TEST(test_FixedDelayed_comprehensive_sense);
        CPPUNIT_TEST(test_FixedDelayed_diminishing_reaction_time);
        CPPUNIT_TEST(test_FixedDelayed_expanding_reaction_time);
        CPPUNIT_TEST(test_FixedDelayed_comprehensive_variable_reaction);
    CPPUNIT_TEST_SUITE_END();
};

}
