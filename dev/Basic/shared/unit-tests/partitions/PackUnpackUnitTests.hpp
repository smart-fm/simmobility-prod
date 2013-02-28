/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "conf/settings/DisableMPI.h"

#include "util/LangHelpers.hpp"
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for packing/unpacking of MPI data.
 * \author Seth N. Hetu
 */
class PackUnpackUnitTests : public CppUnit::TestFixture
{
public:
	///Test simple setting and retrieving
	void test_PackUnpack_simple_set_get() CHECK_MPI_THROW ;

	//Slightly more complicated template processing.
	void test_PackUnpack_fixed_delayed() CHECK_MPI_THROW ;

	//Ensure serialization chaining works.
	void test_PackUnpack_fixed_delayed_dpoint() CHECK_MPI_THROW ;

	//Check serialization of the dynamic vector class.
	void test_PackUnpack_dynamic_vector() CHECK_MPI_THROW ;
	void test_PackUnpack_dynamic_vector2() CHECK_MPI_THROW ;




private:
#ifndef SIMMOB_DISABLE_MPI
    CPPUNIT_TEST_SUITE(PackUnpackUnitTests);
      CPPUNIT_TEST(test_PackUnpack_simple_set_get);
      CPPUNIT_TEST(test_PackUnpack_fixed_delayed);
      CPPUNIT_TEST(test_PackUnpack_fixed_delayed_dpoint);
      CPPUNIT_TEST(test_PackUnpack_dynamic_vector);
      CPPUNIT_TEST(test_PackUnpack_dynamic_vector2);
    CPPUNIT_TEST_SUITE_END();
#endif
};

}
