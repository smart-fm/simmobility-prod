#include <Buffered.hpp>
#include <Buffered_uint32.hpp>
#include <BufferedDataManager.hpp>

#include "BufferedUnitTests.hpp"

namespace unit_tests
{

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::BufferedUnitTests);

void BufferedUnitTests::test_default_Buffered_uint32_constructor()
{
    sim_mob::Buffered_uint32 integer;
    CPPUNIT_ASSERT(0 == integer);
    CPPUNIT_ASSERT(0 == integer.get());
}

void BufferedUnitTests::test_default_Buffered_float_constructor()
{
    sim_mob::Buffered<float> floater;
    CPPUNIT_ASSERT(0.0f == floater);
    CPPUNIT_ASSERT(0.0f == floater.get());
}

void BufferedUnitTests::test_Buffered_uint32_constructor()
{
    sim_mob::Buffered_uint32 integer(42);
    CPPUNIT_ASSERT(42 == integer);
    CPPUNIT_ASSERT(42 == integer.get());
}

void BufferedUnitTests::test_Buffered_float_constructor()
{
    sim_mob::Buffered<float> floater(3.14159f);
    CPPUNIT_ASSERT(3.14159f == floater);
    CPPUNIT_ASSERT(3.14159f == floater.get());
}

void BufferedUnitTests::test_flipping_after_single_change_to_Buffered_uint32()
{
    sim_mob::Buffered_uint32 integer(42);

    sim_mob::BufferedDataManager mgr;
    mgr.add(&integer);

    integer.set(1);
    CPPUNIT_ASSERT(42 == integer);

    mgr.flip();
    CPPUNIT_ASSERT(1 == integer);

    mgr.rem(&integer);
}

void BufferedUnitTests::test_flipping_after_single_change_to_Buffered_float()
{
    sim_mob::Buffered<float> floater(3.14159f);

    sim_mob::BufferedDataManager mgr;
    mgr.add(&floater);

    floater.set(2.71828f);
    CPPUNIT_ASSERT(3.14159f == floater);

    mgr.flip();
    CPPUNIT_ASSERT(2.71828f == floater);

    mgr.rem(&floater);
}

void BufferedUnitTests::test_flipping_after_multiple_changes()
{
    sim_mob::Buffered_uint32 integer(42);

    sim_mob::BufferedDataManager mgr;
    mgr.add(&integer);

    integer.set(100);
    CPPUNIT_ASSERT(42 == integer);
    ++integer;
    CPPUNIT_ASSERT(42 == integer);
    integer++;
    CPPUNIT_ASSERT(42 == integer);
    integer -= 3;
    CPPUNIT_ASSERT(42 == integer);
    integer--;
    CPPUNIT_ASSERT(42 == integer);
    --integer;
    CPPUNIT_ASSERT(42 == integer);
    integer += 6;
    CPPUNIT_ASSERT(42 == integer);

    mgr.flip();
    CPPUNIT_ASSERT(100 + 1 + 1 - 3 - 1 - 1 + 6 == integer);

    mgr.rem(&integer);
}

void BufferedUnitTests::test_multiple_changes_back_to_original_value()
{
    sim_mob::Buffered_uint32 integer(42);

    sim_mob::BufferedDataManager mgr;
    mgr.add(&integer);

    ++integer;
    CPPUNIT_ASSERT(42 == integer);
    integer.set(42);
    CPPUNIT_ASSERT(42 == integer);

    mgr.flip();
    CPPUNIT_ASSERT(42 == integer);

    mgr.rem(&integer);
}

namespace
{
    enum Color { green, amber, red };

    class BufferedColor : public sim_mob::Buffered<Color>
    {
    public:
        BufferedColor(enum Color color)
          : Buffered<Color>(color)
        {
        }
    };
}

void BufferedUnitTests::test_flipping_single_change_to_Buffered_enum()
{
    BufferedColor color(red);

    sim_mob::BufferedDataManager mgr;
    mgr.add(&color);

    CPPUNIT_ASSERT(red == color);
    color.set(green);
    CPPUNIT_ASSERT(red == color);

    mgr.flip();
    CPPUNIT_ASSERT(green == color);

    mgr.rem(&color);
}

}
