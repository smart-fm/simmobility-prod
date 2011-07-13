#include <Buffered.hpp>

#include "BufferedUnitTests.hpp"

namespace unit_tests
{

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::BufferedUnitTests);

void BufferedUnitTests::test_default_Buffered_float_constructor()
{
    sim_mob::Buffered<float> floater;
    CPPUNIT_ASSERT(0.0f == floater);
    CPPUNIT_ASSERT(0.0f == floater.get());
}

}
