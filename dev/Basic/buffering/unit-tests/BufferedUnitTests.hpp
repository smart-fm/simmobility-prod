#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for the double-buffered data types in Basic/buffering.
 */
class BufferedUnitTests : public CppUnit::TestFixture
{
public:
    /**
     * Tests the Buffered<float> constructor.
     *
     * The Buffered<float> constructor should initialize current_ to 0.0.
     * This test confirms that the value is indeed 0.0 via get() and
     * the float conversion operator.
     */
    void test_default_Buffered_float_constructor();

private:
    CPPUNIT_TEST_SUITE(BufferedUnitTests);
        CPPUNIT_TEST(test_default_Buffered_float_constructor);
    CPPUNIT_TEST_SUITE_END();
};

}
