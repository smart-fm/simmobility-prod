#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for the double-buffered data types in Basic/buffering.
 */
class BufferedUnitTests : public CppUnit::TestFixture
{
public:
    /**
     * Tests the default Buffered_uint32 constructor.
     *
     * The default Buffered_uint32 constructor should initialize current_ to 0.
     * This test confirms that the value is indeed 0 via get() and the
     * convert-to-uint32 operator.
     */
    void test_default_Buffered_uint32_constructor();

    /**
     * Tests the default Buffered<float> constructor.
     *
     * The default Buffered<float> constructor should initialize current_ to 0.0.
     * This test confirms that the value is indeed 0.0 via get() and the
     * convert-to-float operator.
     */
    void test_default_Buffered_float_constructor();

    /**
     * Tests the Buffered_uint32 constructor.
     *
     * This test confirms that the Buffered_uint32 constructor initializes current_ correctly.
     */
    void test_Buffered_uint32_constructor();

    /**
     * Tests the Buffered<float> constructor.
     *
     * This test confirms that the Buffered<float> constructor initializes current_ correctly.
     */
    void test_Buffered_float_constructor();

    /**
     * Tests that the BufferedDataManager is able to flip the Buffered_uint32 class correctly.
     *
     * This test confirms that the Buffered_uint32 object retains its current value until
     * its data-manager calls its flip() method, at which time its current value is
     * properly updated.
     */
    void test_flipping_after_single_change_to_Buffered_uint32();

    /**
     * Tests that the BufferedDataManager is able to flip the Buffered<float> class.
     *
     * This test confirms that the Buffered<float> object retains its current value until
     * its data-manager calls its flip() method, at which time its current value is
     * properly updated.
     */
    void test_flipping_after_single_change_to_Buffered_float();

    /**
     * Shows that changing Buffered<T> object multiple times doesn't affect its current value.
     *
     * This test confirms that the Buffered<T> object retains its current value, no matter
     * how many times its next value is changed, until its data-manager calls its flip()
     * method, at which time its current value is properly updated.
     *
     * The test also confirms that the flip() method sets its current value to the latest
     * next value at the time flip() was called.
     *
     * This test tests the pre- and post- increment and decrement operators as well as the
     * += and -= operators of the Buffered_uint32 class.
     */
    void test_flipping_after_multiple_changes();

    /**
     * Shows that flip() sets the current value to the latest next value.
     *
     * This test confirms that the flip() method sets its current value to the latest
     * next value at the time flip() was called.  Even if the Buffered<T> object was
     * changed multiple times, even to the point that it was reset to its original
     * value, the flip() uses the last value of next_ to set the current_ value.
     */
    void test_multiple_changes_back_to_original_value();

    /**
     * Shows that enum can also be double-buffered.
     *
     * This test confirms that Buffered<enum> works as well as any Buffered<T> types.
     */
    void test_flipping_single_change_to_Buffered_enum();

    /**
     * Tests the BufferedDataManager flipping all of its managed data.
     *
     * This test confirms that the BufferedDataManager will flip all Buffered<T> objects
     * that it is managing, even if some of the objects have not changed their values.
     */
    void test_BufferedDataManager_with_several_Buffered_T_objects();

    /**
     * Tests the migration of Buffered<T> objects from one BufferedDataManager to another manager.
     *
     * This test confirms that beginManaging() and stopManaging() work correctly.
     */
    void test_migrating_to_another_BufferedDataManager();

    /**
     * Tests an Agent class with Buffered<T> objects that are managed by a BufferedDataManager.
     *
     * This test confirms that an Agent can own several Buffered<T> objects and delegates the
     * task of flipping its Buffered<T> objects to a BufferedDataManager.  The agent changes
     * the values of its Buffered<T> objects in its update() method and the BufferedDataManager's
     * flip() method is called after each call to update().  This test calls update() and flip()
     * twice.
     */
    void test_Agent_with_Buffered_T_objects();

    /**
     * Tests that a Buffered_T object must not be managed by any BufferedDataManager when it dies.
     *
     * This test confirms that a Buffered_T object must not be managed by any BufferedDataManager
     * when its destructor is called.
     */
    void test_Buffered_T_reference_count();


    /**
     * Tests that managing a variable twice won't lead to an error.
     */
    void test_BufferedDataManager_doubleBeginManage();


    /**
     * Tests that unmanaging a variable twice won't lead to an error.
     */
    void test_BufferedDataManager_doubleStopManaging();


    /**
     * Tests BufferedDataManager::stopManaging() works correctly.
     *
     * This test confirms that BufferedDataManager::stopManaging() works even if the argument
     * is a Buffered<T> object that it is not managing.
     */
    void test_BufferedDataManager_stopManaging();

private:
    CPPUNIT_TEST_SUITE(BufferedUnitTests);
        CPPUNIT_TEST(test_default_Buffered_uint32_constructor);
        CPPUNIT_TEST(test_default_Buffered_float_constructor);
        CPPUNIT_TEST(test_Buffered_uint32_constructor);
        CPPUNIT_TEST(test_Buffered_float_constructor);
        CPPUNIT_TEST(test_flipping_after_single_change_to_Buffered_uint32);
        CPPUNIT_TEST(test_flipping_after_single_change_to_Buffered_float);
        CPPUNIT_TEST(test_flipping_after_multiple_changes);
        CPPUNIT_TEST(test_multiple_changes_back_to_original_value);
        CPPUNIT_TEST(test_flipping_single_change_to_Buffered_enum);
        CPPUNIT_TEST(test_BufferedDataManager_with_several_Buffered_T_objects);
        CPPUNIT_TEST(test_migrating_to_another_BufferedDataManager);
        CPPUNIT_TEST(test_Agent_with_Buffered_T_objects);
        CPPUNIT_TEST(test_Buffered_T_reference_count);
        CPPUNIT_TEST(test_BufferedDataManager_doubleBeginManage);
        CPPUNIT_TEST(test_BufferedDataManager_doubleStopManaging);
        CPPUNIT_TEST(test_BufferedDataManager_stopManaging);
    CPPUNIT_TEST_SUITE_END();
};

}
