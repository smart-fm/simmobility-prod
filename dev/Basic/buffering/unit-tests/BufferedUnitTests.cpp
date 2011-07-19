#include <cmath>
#include <limits>

#include <Buffered.hpp>
#include <Buffered_uint32.hpp>
#include <BufferedDataManager.hpp>
#include <Vector2D.hpp>

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

        size_t managed_data_count() const { return managedData.size(); }

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
            instance_ = new GlobalDataManager();
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
    CPPUNIT_ASSERT(GlobalDataManager::singleton().managed_data_count()==3);

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


void BufferedUnitTests::test_BufferedDataManager_doubleBeginManage()
{
    sim_mob::Buffered_uint32 var(42);

    DataManager mgr1;
    mgr1.beginManaging(&var);
    mgr1.beginManaging(&var);
    CPPUNIT_ASSERT(1 == mgr1.managed_data_count());

    mgr1.stopManaging(&var);
    CPPUNIT_ASSERT(0 == mgr1.managed_data_count());
}

void BufferedUnitTests::test_BufferedDataManager_doubleStopManaging()
{
    sim_mob::Buffered_uint32 var(42);

    DataManager mgr1;
    mgr1.beginManaging(&var);
    CPPUNIT_ASSERT(1 == mgr1.managed_data_count());

    mgr1.stopManaging(&var);
    mgr1.stopManaging(&var);
    CPPUNIT_ASSERT(0 == mgr1.managed_data_count());
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

void BufferedUnitTests::test_the_Vector2D_class()
{
    // This is lazy: the better approach is to have a separate method of the BufferedUnitTests
    // class for each of the test-cases below.  If any of the test-cases fail, cppunit doesn't
    // provide any indication which test-case failed.

    {
        // Testing the default constructor.
        sim_mob::Vector2D origin;
        CPPUNIT_ASSERT(0.0f == origin.getX());
        CPPUNIT_ASSERT(0.0f == origin.getY());
    }

    {
        // Testing the constructor.
        sim_mob::Vector2D vec(3, 4);
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
    }

    {
        // Testing the copy constructor.
        sim_mob::Vector2D vec1(3, 4);
        sim_mob::Vector2D vec2(vec1);
        CPPUNIT_ASSERT(3.0f == vec2.getX());
        CPPUNIT_ASSERT(4.0f == vec2.getY());
    }

    {
        // Testing the copy assignment.
        sim_mob::Vector2D vec1(3, 4);
        sim_mob::Vector2D vec2(5, 12);
        CPPUNIT_ASSERT(5.0f == vec2.getX());
        CPPUNIT_ASSERT(12.0f == vec2.getY());
        vec2 = vec1;
        CPPUNIT_ASSERT(3.0f == vec2.getX());
        CPPUNIT_ASSERT(4.0f == vec2.getY());

        // Yet another assignment to the 0 vector (the origin).
        vec2 = sim_mob::Vector2D();
        CPPUNIT_ASSERT(0.0f == vec2.getX());
        CPPUNIT_ASSERT(0.0f == vec2.getY());
    }

    {
        // Testing setX() and setY().
        sim_mob::Vector2D vec(3, 4);
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
        vec.setX(5.0f);
        CPPUNIT_ASSERT(5.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
        vec.setY(12.0f);
        CPPUNIT_ASSERT(5.0f == vec.getX());
        CPPUNIT_ASSERT(12.0f == vec.getY());
    }

    {
        // Testing the += operator.
        sim_mob::Vector2D vec1(3, 4);
        CPPUNIT_ASSERT(3.0f == vec1.getX());
        CPPUNIT_ASSERT(4.0f == vec1.getY());
        sim_mob::Vector2D vec2(5, 12);
        vec1 += vec2;
        CPPUNIT_ASSERT(8.0f == vec1.getX());
        CPPUNIT_ASSERT(16.0f == vec1.getY());

        // Adding the 0 vector will not change anything.
        vec1 += sim_mob::Vector2D();
        CPPUNIT_ASSERT(8.0f == vec1.getX());
        CPPUNIT_ASSERT(16.0f == vec1.getY());

        // Adding itself.
        vec1 += vec1;
        CPPUNIT_ASSERT(2 * 8.0f == vec1.getX());
        CPPUNIT_ASSERT(2 * 16.0f == vec1.getY());

        // Adding the inverse should result in the 0 vector.
        vec1 += sim_mob::Vector2D(-16, -32);
        CPPUNIT_ASSERT(0.0f == vec1.getX());
        CPPUNIT_ASSERT(0.0f == vec1.getY());
    }

    {
        // Testing the -= operator.
        sim_mob::Vector2D vec1(3, 4);
        CPPUNIT_ASSERT(3.0f == vec1.getX());
        CPPUNIT_ASSERT(4.0f == vec1.getY());
        sim_mob::Vector2D vec2(5, 12);
        vec1 -= vec2;
        CPPUNIT_ASSERT(-2.0f == vec1.getX());
        CPPUNIT_ASSERT(-8.0f == vec1.getY());

        // Subtracting the 0 vector will not change anything.
        vec1 -= sim_mob::Vector2D();
        CPPUNIT_ASSERT(-2.0f == vec1.getX());
        CPPUNIT_ASSERT(-8.0f == vec1.getY());

        // Subtracting the inverse should result in the 0 vector.
        vec1 -= sim_mob::Vector2D(-2, -8);
        CPPUNIT_ASSERT(0.0f == vec1.getX());
        CPPUNIT_ASSERT(0.0f == vec1.getY());

        // Subtracting itself will also result in the 0 vector.
        vec1 = sim_mob::Vector2D(3, 4);
        vec1 -= vec1;
        CPPUNIT_ASSERT(0.0f == vec1.getX());
        CPPUNIT_ASSERT(0.0f == vec1.getY());
    }

    {
        // Testing the += and -= operators.
        sim_mob::Vector2D vec(3, 4);
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
        sim_mob::Vector2D delta(0.1, 0.3);
        vec += delta;
        CPPUNIT_ASSERT(3 + 0.1f == vec.getX());
        CPPUNIT_ASSERT(4 + 0.3f == vec.getY());
        vec -= delta;
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());

        // Reverse order: -= first before +=
        delta = sim_mob::Vector2D(3.14159f, 2.71828f);
        vec -= delta;
        CPPUNIT_ASSERT(3 - 3.14159f == vec.getX());
        CPPUNIT_ASSERT(4 - 2.71828f == vec.getY());
        vec += delta;
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
    }

    {
        // Testing the *= operator.
        sim_mob::Vector2D vec(3, 4);
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
        vec *= 2;
        CPPUNIT_ASSERT(6.0f == vec.getX());
        CPPUNIT_ASSERT(8.0f == vec.getY());

        // The scalar 1 shouldn't change the vector.
        vec *= 1;
        CPPUNIT_ASSERT(6.0f == vec.getX());
        CPPUNIT_ASSERT(8.0f == vec.getY());

        // The scalar 0 should shrink the vector to the 0 vector.
        vec *= 0;
        CPPUNIT_ASSERT(0.0f == vec.getX());
        CPPUNIT_ASSERT(0.0f == vec.getY());
    }

    {
        // Testing the /= operator.
        sim_mob::Vector2D vec(3, 4);
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
        vec /= 2;
        CPPUNIT_ASSERT(1.5f == vec.getX());
        CPPUNIT_ASSERT(2.0f == vec.getY());

        // The scalar 1 shouldn't change the vector.
        vec /= 1;
        CPPUNIT_ASSERT(1.5f == vec.getX());
        CPPUNIT_ASSERT(2.0f == vec.getY());
    }

    {
        // Testing the *= and /= operators.
        sim_mob::Vector2D vec(3, 4);
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
        vec *= 3.14159f;
        CPPUNIT_ASSERT(3 * 3.14159f == vec.getX());
        CPPUNIT_ASSERT(4 * 3.14159f == vec.getY());
        vec /= 3.14159f;
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());

        // Reverse order: /= first before *=
        vec /= 2.71828f;
        CPPUNIT_ASSERT(3 / 2.71828f == vec.getX());
        CPPUNIT_ASSERT(4 / 2.71828f == vec.getY());
        vec *= 2.71828f;
        CPPUNIT_ASSERT(3.0f == vec.getX());
        CPPUNIT_ASSERT(4.0f == vec.getY());
    }

    {
        // Testing the == and != operators.
        sim_mob::Vector2D vec1(2, 4);
        sim_mob::Vector2D vec2(vec1);
        CPPUNIT_ASSERT(vec1 == vec2);

        vec1 += sim_mob::Vector2D(0.1, -0.3);
        CPPUNIT_ASSERT(vec1 != vec2);

        vec2.setX(vec1.getX());
        vec2.setY(vec1.getY());
        CPPUNIT_ASSERT(vec1 == vec2);

        CPPUNIT_ASSERT(vec1 != sim_mob::Vector2D());

        vec1 -= vec2;
        CPPUNIT_ASSERT(vec1 == sim_mob::Vector2D());
    }

    {
        // Testing the + and - operators.
        const sim_mob::Vector2D vec1(3, 4);
        const sim_mob::Vector2D vec2(5, 12);
        sim_mob::Vector2D vec3;
        CPPUNIT_ASSERT(0.0f == vec3.getX());
        CPPUNIT_ASSERT(0.0f == vec3.getY());

        vec3 = vec1 + vec2;
        CPPUNIT_ASSERT(8.0f == vec3.getX());
        CPPUNIT_ASSERT(16.0f == vec3.getY());

        vec3 = vec1 + vec2 + sim_mob::Vector2D(3.14159f, 2.71828f);
        CPPUNIT_ASSERT(8 + 3.14159f == vec3.getX());
        CPPUNIT_ASSERT(16 + 2.71828f == vec3.getY());

        vec3 = vec1 - vec2;
        CPPUNIT_ASSERT(-2.0f == vec3.getX());
        CPPUNIT_ASSERT(-8.0f == vec3.getY());

        vec3 = vec1 + sim_mob::Vector2D(3.14159f, 2.71828f) - vec2;
        CPPUNIT_ASSERT(-2 + 3.14159f == vec3.getX());
        CPPUNIT_ASSERT(-8 + 2.71828f == vec3.getY());

        CPPUNIT_ASSERT((vec3 - vec3) == sim_mob::Vector2D());
    }

    {
        // Testing the * and / operators.
        const sim_mob::Vector2D vec1(3, 4);
        CPPUNIT_ASSERT((3.14159f * vec1) == sim_mob::Vector2D(3 * 3.14159f, 4 * 3.14159f));
        CPPUNIT_ASSERT((vec1 * 3.14159f) == sim_mob::Vector2D(3 * 3.14159f, 4 * 3.14159f));
        CPPUNIT_ASSERT((vec1 / 3.14159f) == sim_mob::Vector2D(3 / 3.14159f, 4 / 3.14159f));

        sim_mob::Vector2D vec2;
        CPPUNIT_ASSERT(0.0f == vec2.getX());
        CPPUNIT_ASSERT(0.0f == vec2.getY());
        // Hope that the compiler will not optimize the next line to "vec1 * 1.15572f"
        // (1.15572f being equal to 3.14159f / 2.71828f) but produce the pseudo-code
        //     sim_mob::Vector2D tmp = 3.14159f * vec1;
        //     vec2 = tmp / 2.71828f;
        vec2 = (3.14159f * vec1) / 2.71828f;
        CPPUNIT_ASSERT(3 * 3.14159f / 2.71828f == vec2.getX());
        CPPUNIT_ASSERT(4 * 3.14159f / 2.71828f == vec2.getY());
    }

    {
        // Testing length().
        sim_mob::Vector2D vec(3, 4);
        CPPUNIT_ASSERT(length(vec) == 5.0f);

        vec.setX(5);
        vec.setY(12);
        CPPUNIT_ASSERT(length(vec) == 13.0f);

        // The length of s*vec is s * length(vec)
        CPPUNIT_ASSERT(length(3.14159f * vec) == 3.14159f * 13.0f);
    }

    {
        // Testing normalize().
        sim_mob::Vector2D vec1(3, 4);
        CPPUNIT_ASSERT(normalize(vec1) == sim_mob::Vector2D(0.6f, 0.8f));

        vec1.setX(5);
        vec1.setY(12);
        sim_mob::Vector2D vec2 = normalize(vec1);
        CPPUNIT_ASSERT(5 / 13.0f == vec2.getX());
        CPPUNIT_ASSERT(12 / 13.0f == vec2.getY());
        CPPUNIT_ASSERT(length(vec2) == 1.0f);
    }

    {
        // Testing the unary - operator.
        const sim_mob::Vector2D vec1(3, 4);
        sim_mob::Vector2D vec2(-vec1);
        CPPUNIT_ASSERT(-3.0f == vec2.getX());
        CPPUNIT_ASSERT(-4.0f == vec2.getY());

        // The inverse of the 0 vector is itself.
        vec2 = -sim_mob::Vector2D();
        CPPUNIT_ASSERT(vec2 == sim_mob::Vector2D());
    }

    {
        // Testing the dot product.
        const sim_mob::Vector2D vec1(3, 4);
        const sim_mob::Vector2D vec2(5, 12);
        // The inner product remains the same when the order is reversed.
        CPPUNIT_ASSERT((vec1 * vec2) == (vec2 * vec1));

        // Another formula for the dot product is length(vec1) * length(vec2) * cos(theta)
        // where theta is the angle between the 2 vectors.
        float angle1 = atan2f(vec2.getY(), vec2.getX());
        float angle2 = atan2f(vec1.getY(), vec1.getX());
        float theta = angle2 - angle1;
        CPPUNIT_ASSERT((vec1 * vec2) == (length(vec1) * length(vec2) * cosf(theta)));

        // The dot product of a vector and its normal is 0.  The normal, the vector that
        // is perpendicular to vec(x, y), can be simply calcuated as vec(-y, x) or vec(y, -x).
        const sim_mob::Vector2D vec3(-vec1.getY(), vec1.getX());
        CPPUNIT_ASSERT((vec1 * vec3) == 0.0f);
        CPPUNIT_ASSERT((vec1 * -vec3) == 0.0f);

        // The dot product of 2 parallel vectors is the product of their lengths, negative if
        // the 2 vectors are in opposite directions.
        float len1 = length(vec1);
        CPPUNIT_ASSERT((vec1 * -vec1) == -(len1 * len1));
        const sim_mob::Vector2D vec4(vec1 / 2);
        CPPUNIT_ASSERT((vec1 * vec4) == (len1 * len1/2));
    }
}

}
