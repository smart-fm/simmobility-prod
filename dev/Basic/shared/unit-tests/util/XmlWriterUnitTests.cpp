/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "XmlWriterUnitTests.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <ostream>

#include "util/XmlWriter.hpp"

using std::string;

CPPUNIT_TEST_SUITE_REGISTRATION(unit_tests::XmlWriterUnitTests);

void unit_tests::XmlWriterUnitTests::test_SimpleXML()
{
	std::ostringstream stream;
	{
	sim_mob::xml::XmlWriter write(stream);
    write.prop("name", string("John"));
    write.prop("age", 19);
    write.prop("male", true);
    write.prop("favorite_decimal", 3.5);
	}

	string expected =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<name>John</name>\n"
		"<age>19</age>\n"
		"<male>true</male>\n"
		"<favorite_decimal>3.5000</favorite_decimal>\n";

	CPPUNIT_ASSERT(stream.str()==expected);
}


void unit_tests::XmlWriterUnitTests::test_NestedXML()
{

}
