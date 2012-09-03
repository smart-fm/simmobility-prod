/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for nullptr (either the class we created, or the standard library one).
 * \author Seth N. Hetu
 */
class NullPtrUnitTests : public CppUnit::TestFixture
{
public:
	///Test equality
	void test_Equality();

	///Test negative equality
	void test_NEquality();


private:
    CPPUNIT_TEST_SUITE(NullPtrUnitTests);
        CPPUNIT_TEST(test_Equality);
        CPPUNIT_TEST(test_NEquality);
    CPPUNIT_TEST_SUITE_END();
};

}
