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
	///One-time setup
	virtual void setUp();

	///Test strings that are expected to pass.
	void test_Base64Escape_1();
	void test_Base64Escape_2();
	void test_Base64Escape_3();
	void test_Base64Escape_4();
	void test_Base64Escape_5();
	void test_Base64Escape_6();
	void test_Base64Escape_7();
	void test_Base64Escape_8();
	void test_Base64Escape_9();

	//Test actual object data from RoadRunner.
	void test_Base64Escape_roadrunner_1();

private:
#ifndef SIMMOB_DISABLE_MPI
    CPPUNIT_TEST_SUITE(Base64EscapeUnitTests);
      CPPUNIT_TEST(test_Base64Escape_1);
      CPPUNIT_TEST(test_Base64Escape_2);
      CPPUNIT_TEST(test_Base64Escape_3);
      CPPUNIT_TEST(test_Base64Escape_4);
      CPPUNIT_TEST(test_Base64Escape_5);
      CPPUNIT_TEST(test_Base64Escape_6);
      CPPUNIT_TEST(test_Base64Escape_7);
      CPPUNIT_TEST(test_Base64Escape_8);
      CPPUNIT_TEST(test_Base64Escape_9);
      CPPUNIT_TEST(test_Base64Escape_roadrunner_1);
    CPPUNIT_TEST_SUITE_END();
#endif
};

}
