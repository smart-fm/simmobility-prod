/*
 * ModelTest.hpp
 *
 *  Created on: Aug 26, 2014
 *      Author: gishara
 */

#pragma once

#include "model/DeveloperModel.hpp"
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace sim_mob;
using namespace sim_mob::long_term;

namespace unit_tests {

class ModelTest : public CppUnit::TestFixture{

public:
	void setUp();

private:
	DeveloperModel *developerModel;

	CPPUNIT_TEST_SUITE(ModelTest);
	CPPUNIT_TEST_SUITE_END();

};
}
