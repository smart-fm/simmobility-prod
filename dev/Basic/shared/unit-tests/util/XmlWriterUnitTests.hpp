/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

namespace unit_tests
{

/**
 * Unit Tests for the XmlWriter class, used for serializing items to Xml
 * \author Seth N. Hetu
 */
class XmlWriterUnitTests : public CppUnit::TestFixture
{
public:
	///Very basic test.
	void test_SimpleXML();

	///Multiple, nested structures.
	void test_NestedXML();

	///Test attributes at different nesting levels.
	void test_AttributesXML();

	///Test default and custom namers for various combinations.
	void test_NamersXML();


private:
    CPPUNIT_TEST_SUITE(XmlWriterUnitTests);
        CPPUNIT_TEST(test_SimpleXML);
        CPPUNIT_TEST(test_NestedXML);
        CPPUNIT_TEST(test_AttributesXML);
        CPPUNIT_TEST(test_NamersXML);
    CPPUNIT_TEST_SUITE_END();
};

}
