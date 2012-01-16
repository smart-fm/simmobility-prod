/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for the Daily Time class in Basic/util
 */
class DailyTimeUnitTests : public CppUnit::TestFixture
{
public:
    /**
     * Ensure nonsense isn't parsed
     *
     * If something which is obviously not a DailyTime is passed into the constructor,
     * the engine should throw an exception.
     */
    void test_invalid_DailyTime_constructor();



private:
    CPPUNIT_TEST_SUITE(DailyTimeUnitTests);
        CPPUNIT_TEST(test_invalid_DailyTime_constructor);
    CPPUNIT_TEST_SUITE_END();
};

}
