//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for the Daily Time class in Basic/util
 * \author Seth N. Hetu
 */
class DailyTimeUnitTests : public CppUnit::TestFixture
{
public:
	///Test the default DailyTime constructor.
	///The default sets itself to 00:00
	void test_default_DailyTime_constructor();

	///Test the time_t DailyTime constructor.
	void test_time_t_DailyTime_constructor();

    ///Ensure nonsense isn't parsed
    ///If something which is obviously not a DailyTime is passed into the constructor,
	///the engine should throw an exception.
    void test_invalid_DailyTime_constructor();

    ///Ensure optional seconds can be parsed.
    ///Time values can be written with or without a trailing :SS value.
    void test_optional_seconds_DailyTime_constructor();

    ///Ensure hours/minutes are mandatory
    ///The format of HH::MM is required for clarity.
    void test_nonoptional_hoursminutes_DailyTime_constructor();

    ///Check time comparison: fine-grained
    ///Time comparison should operate on the seconds granularity level.
    void test_DailyTime_comparison_fine();

    ///Check time comparison: coarser grain
    ///Time comparison on the order of hours and minutes should work too.
    void test_DailyTime_comparison_coarse();

    ///Check parsing fractions
    ///Partial seconds should work (with fractions)
    void test_DailyTime_comparison_superfine();




private:
    CPPUNIT_TEST_SUITE(DailyTimeUnitTests);
        CPPUNIT_TEST(test_invalid_DailyTime_constructor);
        CPPUNIT_TEST(test_time_t_DailyTime_constructor);
        CPPUNIT_TEST(test_optional_seconds_DailyTime_constructor);
        CPPUNIT_TEST(test_default_DailyTime_constructor);
        CPPUNIT_TEST(test_nonoptional_hoursminutes_DailyTime_constructor);
        CPPUNIT_TEST(test_DailyTime_comparison_fine);
        CPPUNIT_TEST(test_DailyTime_comparison_coarse);
        CPPUNIT_TEST(test_DailyTime_comparison_superfine);
    CPPUNIT_TEST_SUITE_END();
};

}
