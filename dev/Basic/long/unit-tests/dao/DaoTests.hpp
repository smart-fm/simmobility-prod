//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   DaoTests.hpp
 * Unit Tests for DAO classes in long term
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * \author: Gishara Premarathne <gishara@smart.mit.edu>
 * Created on May 7, 2013, 5:22 PM
 */
#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests {

class DaoTests : public CppUnit::TestFixture{
        
public:
	/** Runs all tests inside of this class. */
	void testAll();

private:
	CPPUNIT_TEST_SUITE(DaoTests);
		CPPUNIT_TEST(testAll);
	CPPUNIT_TEST_SUITE_END();

};
}


