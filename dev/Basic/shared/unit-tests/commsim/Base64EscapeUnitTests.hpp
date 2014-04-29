//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for decoding base-64 (with our special considerations).
 * \author Seth N. Hetu
 */
class Base64EscapeUnitTests : public CppUnit::TestFixture
{
public:
	///Test strings that are expected to pass.
	void test_Base64Escape_normal_strings();

private:
#ifndef SIMMOB_DISABLE_MPI
    CPPUNIT_TEST_SUITE(Base64EscapeUnitTests);
      CPPUNIT_TEST(test_Base64Escape_normal_strings);
    CPPUNIT_TEST_SUITE_END();
#endif
};

}
