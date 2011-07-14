#include <cmath>
#include <limits>

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
    mgr.beginManaging(&integer);

    integer.set(1);
    CPPUNIT_ASSERT(42 == integer);

    mgr.flip();
    CPPUNIT_ASSERT(1 == integer);

    mgr.stopManaging(&integer);
}

void BufferedUnitTests::test_flipping_after_single_change_to_Buffered_float()
{
    sim_mob::Buffered<float> floater(3.14159f);

    sim_mob::BufferedDataManager mgr;
    mgr.beginManaging(&floater);

    floater.set(2.71828f);
    CPPUNIT_ASSERT(3.14159f == floater);

    mgr.flip();
    CPPUNIT_ASSERT(2.71828f == floater);

    mgr.stopManaging(&floater);
}

void BufferedUnitTests::test_flipping_after_multiple_changes()
{
    sim_mob::Buffered_uint32 integer(42);

    sim_mob::BufferedDataManager mgr;
    mgr.beginManaging(&integer);

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

    mgr.stopManaging(&integer);
}

void BufferedUnitTests::test_multiple_changes_back_to_original_value()
{
    sim_mob::Buffered_uint32 integer(42);

    sim_mob::BufferedDataManager mgr;
    mgr.beginManaging(&integer);

    ++integer;
    CPPUNIT_ASSERT(42 == integer);
    integer.set(42);
    CPPUNIT_ASSERT(42 == integer);

    mgr.flip();
    CPPUNIT_ASSERT(42 == integer);

    mgr.stopManaging(&integer);
}

namespace
{
    enum Color { green, amber, red };

    class BufferedColor : public sim_mob::Buffered<Color>
    {
    public:
        BufferedColor(enum Color color)
          : sim_mob::Buffered<Color>(color)
        {
        }
    };
}

void BufferedUnitTests::test_flipping_single_change_to_Buffered_enum()
{
    BufferedColor color(red);

    sim_mob::BufferedDataManager mgr;
    mgr.beginManaging(&color);

    CPPUNIT_ASSERT(red == color);
    color.set(green);
    CPPUNIT_ASSERT(red == color);

    mgr.flip();
    CPPUNIT_ASSERT(green == color);

    mgr.stopManaging(&color);
}

namespace
{
    class DataManager : public sim_mob::BufferedDataManager
    {
    public:
        // No need to define the ctor and dtor.

        size_t managed_data_count() const { return managedData.size(); }
    };
}

void BufferedUnitTests::test_BufferedDataManager_with_several_Buffered_T_objects()
{
    sim_mob::Buffered_uint32 integer(42);
    sim_mob::Buffered<float> floater(3.14159f);
    BufferedColor color(red);

    DataManager mgr;
    mgr.beginManaging(&integer);
    mgr.beginManaging(&floater);
    mgr.beginManaging(&color);
    CPPUNIT_ASSERT(3 == mgr.managed_data_count());

    integer++;
    CPPUNIT_ASSERT(42 == integer);
    floater.set(2.71828f);
    CPPUNIT_ASSERT(3.14159f == floater);
    // color is not changed.
    CPPUNIT_ASSERT(red == color);

    mgr.flip();
    CPPUNIT_ASSERT(43 == integer);
    CPPUNIT_ASSERT(2.71828f == floater);
    CPPUNIT_ASSERT(red == color);

    mgr.stopManaging(&integer);
    mgr.stopManaging(&floater);
    mgr.stopManaging(&color);
    CPPUNIT_ASSERT(0 == mgr.managed_data_count());
}

void BufferedUnitTests::test_migrating_to_another_BufferedDataManager()
{
    sim_mob::Buffered_uint32 integer(42);
    sim_mob::Buffered<float> floater(3.14159f);
    BufferedColor color(red);

    DataManager mgr1;
    mgr1.beginManaging(&integer);
    mgr1.beginManaging(&floater);
    mgr1.beginManaging(&color);
    CPPUNIT_ASSERT(3 == mgr1.managed_data_count());

    integer++;
    CPPUNIT_ASSERT(42 == integer);
    floater.set(2.71828f);
    CPPUNIT_ASSERT(3.14159f == floater);
    color.set(green);
    CPPUNIT_ASSERT(red == color);

    DataManager mgr2;
    mgr1.stopManaging(&floater);
    mgr2.beginManaging(&floater);
    CPPUNIT_ASSERT(2 == mgr1.managed_data_count());
    CPPUNIT_ASSERT(1 == mgr2.managed_data_count());

    mgr1.flip();
    CPPUNIT_ASSERT(43 == integer);
    // floater is now managed by mgr2 which hasn't call its flip() yet.  So floater should
    // still retain its current value.
    CPPUNIT_ASSERT(3.14159f == floater);
    CPPUNIT_ASSERT(green == color);

    mgr2.flip();
    CPPUNIT_ASSERT(43 == integer);
    CPPUNIT_ASSERT(2.71828f == floater);
    CPPUNIT_ASSERT(green == color);

    mgr1.stopManaging(&integer);
    mgr2.stopManaging(&floater);
    mgr1.stopManaging(&color);
    CPPUNIT_ASSERT(0 == mgr1.managed_data_count());
    CPPUNIT_ASSERT(0 == mgr2.managed_data_count());
}

namespace
{
    class GlobalDataManager : public sim_mob::BufferedDataManager
    {
    public:
        static GlobalDataManager& singleton();

    private:
        friend void BufferedUnitTests::test_Agent_with_Buffered_T_objects();

        GlobalDataManager();

        static GlobalDataManager* instance_;
    };

    GlobalDataManager* GlobalDataManager::instance_ = 0;

    inline GlobalDataManager::GlobalDataManager()
    {
    }

    GlobalDataManager& GlobalDataManager::singleton()
    {
        if (0 == instance_)
        {
            instance_ = new GlobalDataManager;
        }
        return *instance_;
    }

    // Our agent is a bus.
    class Bus
    {
    public:
        Bus();
        ~Bus();

        void update(unsigned int frame_number);

        sim_mob::Buffered_uint32 const & passengers_count() const;
        sim_mob::Buffered<float> const & gas_tank_level() const;
        BufferedColor const & color() const;

    private:
        sim_mob::Buffered_uint32 passengers_count_;
        sim_mob::Buffered<float> gas_tank_level_;   // between 0.0 (empty) and 1.0 (full).
        BufferedColor color_;   // why is this Bus carrying a traffic signal?  Haha.
    };

    Bus::Bus()
      : passengers_count_(7)  // Our bus was born with 7 passengers on board.
      , gas_tank_level_(0.5)  // Our bus starts life with a half tank; the maintenance team
                              // forgot to fill it up.
      , color_(red)
    {
        // The bus delegates the task of managing its Buffered<T> objects to the global
        // data manager.
        GlobalDataManager & mgr = GlobalDataManager::singleton();
        mgr.beginManaging(&passengers_count_);
        mgr.beginManaging(&gas_tank_level_);
        mgr.beginManaging(&color_);
    }

    Bus::~Bus()
    {
        GlobalDataManager & mgr = GlobalDataManager::singleton();
        mgr.stopManaging(&passengers_count_);
        mgr.stopManaging(&gas_tank_level_);
        mgr.stopManaging(&color_);
    }

    void Bus::update(unsigned int frame_number)
    {
        CPPUNIT_ASSERT(1 == frame_number || 2 == frame_number);

        if (1 == frame_number)
        {
            // One passenger alighted in the 1st frame.
            passengers_count_--;
            // Our bus is a fuel-gusher; OPEC would love this model.
            gas_tank_level_.set(gas_tank_level_ - 0.1);
            // The color changes to amber in the 1st frame.
            color_.set(amber);
        }
        else if (2 == frame_number)
        {
            // 2 passengers boarded in the 2nd frame.
            passengers_count_ += 2;
            // It must be a steep hill climb in the 2nd frame; our bus drank even more fuel.
            gas_tank_level_.set(gas_tank_level_ - 0.2);
            // The color changes to green in the 2nd frame.
            color_.set(green);
        }
    }

    inline sim_mob::Buffered_uint32 const & Bus::passengers_count() const
    {
        return passengers_count_;
    }
    inline sim_mob::Buffered<float> const & Bus::gas_tank_level() const
    {
        return gas_tank_level_;
    }
    inline BufferedColor const & Bus::color() const
    {
        return color_;
    }
}

namespace
{
    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < std::numeric_limits<float>::epsilon());
    }
}

void BufferedUnitTests::test_Agent_with_Buffered_T_objects()
{
    Bus bus;

    CPPUNIT_ASSERT(bus.passengers_count() == 7);
    CPPUNIT_ASSERT(bus.gas_tank_level() == 0.5);
    CPPUNIT_ASSERT(bus.color() == red);

    bus.update(1);
    GlobalDataManager::singleton().flip();
    CPPUNIT_ASSERT(bus.passengers_count() == 7 - 1);
#if 0
    // Inexperienced programmers think that == works for the floating types,
    // they forget that floating types do not have accurate representations.
    // I am one of those programmers.  So I am leaving this here to remind myself.
    // I wonder why == works in test_Buffered_float_constructor() and in
    // test_flipping_after_single_change_to_Buffered_float().
    CPPUNIT_ASSERT(bus.gas_tank_level() == 0.5 - 0.1);
#else
    CPPUNIT_ASSERT(is_equal(bus.gas_tank_level(), 0.5 - 0.1));
#endif
    CPPUNIT_ASSERT(bus.color() == amber);

    bus.update(2);
    GlobalDataManager::singleton().flip();
    CPPUNIT_ASSERT(bus.passengers_count() == 7 - 1 + 2);
#if 0
    CPPUNIT_ASSERT(bus.gas_tank_level() == 0.5 - 0.1 - 0.1);
#else
    CPPUNIT_ASSERT(is_equal(bus.gas_tank_level(), 0.5 - 0.1 - 0.2));
#endif
    CPPUNIT_ASSERT(bus.color() == green);

    if (GlobalDataManager::instance_)
    {
        delete GlobalDataManager::instance_;
        GlobalDataManager::instance_ = 0;
    }
}

void BufferedUnitTests::test_Buffered_T_reference_count()
{
    sim_mob::Buffered_uint32 * integer = new sim_mob::Buffered_uint32;

    DataManager mgr;
    mgr.beginManaging(integer);
    CPPUNIT_ASSERT(1 == mgr.managed_data_count());

    mgr.stopManaging(integer);
    CPPUNIT_ASSERT(0 == mgr.managed_data_count());
    // delete must come after stopManaging().  If the lines are swapped, this test will fail.
    delete integer;
}

void BufferedUnitTests::test_BufferedDataManager_stopManaging()
{
    sim_mob::Buffered_uint32 integer(42);
    sim_mob::Buffered<float> floater(3.14159f);

    DataManager mgr1;
    mgr1.beginManaging(&integer);
    mgr1.beginManaging(&floater);
    CPPUNIT_ASSERT(2 == mgr1.managed_data_count());

    // We have a 2nd data-manager to connivingly test that stopManaging() on a Buffered<T>
    // object that it was not managing will not break anything. 
    DataManager mgr2;
    CPPUNIT_ASSERT(0 == mgr2.managed_data_count());
    mgr2.stopManaging(&floater);
    CPPUNIT_ASSERT(2 == mgr1.managed_data_count());
    CPPUNIT_ASSERT(0 == mgr2.managed_data_count());

    integer++;
    CPPUNIT_ASSERT(42 == integer);
    floater.set(2.71828f);
    CPPUNIT_ASSERT(3.14159f == floater);

    mgr1.flip();
    CPPUNIT_ASSERT(43 == integer);
    CPPUNIT_ASSERT(2.71828f == floater);

    mgr1.stopManaging(&integer);
    mgr1.stopManaging(&floater);
    CPPUNIT_ASSERT(0 == mgr1.managed_data_count());
    CPPUNIT_ASSERT(0 == mgr2.managed_data_count());
}

}
