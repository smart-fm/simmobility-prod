/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "PackUnpackUnitTests.hpp"

#ifndef SIMMOB_DISABLE_MPI

#include <cmath>
#include <limits>
#include <string>
#include <sstream>

#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"

using namespace sim_mob;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::PackUnpackUnitTests);

void unit_tests::PackUnpackUnitTests::test_PackUnpack_simple_set_get()
{





	CPPUNIT_ASSERT_MESSAGE("Simple FixedDelayed retrieval failed.", 2==42);
}



#endif //SIMMOB_DISABLE_MPI
