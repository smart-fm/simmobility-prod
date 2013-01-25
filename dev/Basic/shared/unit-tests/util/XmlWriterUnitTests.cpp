/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "XmlWriterUnitTests.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <ostream>

#include <boost/lexical_cast.hpp>

#include "util/XmlWriter.hpp"

using std::string;
using std::vector;


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


//Simple container struct.
struct Person {
	Person(const string& name, int age) : name(name), age(age) {}
	string name;
	int age;
};

//Rules for writing this struct.
namespace sim_mob {
namespace xml {
template <>
void write_xml(XmlWriter& write, const Person& per)
{
	write.prop("Name", per.name);
	write.prop("AGE", per.age);
}
template <>
std::string get_id(const Person& pers)
{
	return pers.name + "%" + boost::lexical_cast<string>(pers.age);
}
}} //End namespace sim_mob::xml

void unit_tests::XmlWriterUnitTests::test_NestedXML()
{
	//Prepare our data
	vector<string> empty;
	vector<string> items;
	vector<Person> objs;
	items.push_back("Legal characters: abcde");
	items.push_back("Illegal characters: \"'<>&\"'<>&<<<>");
	items.push_back("Numbers: 01234");
	objs.push_back(Person("John", 19));
	objs.push_back(Person("Laura", 33));
	objs.push_back(Person("Nancy", 25));

	//Write our data
	std::ostringstream stream;
	{
	sim_mob::xml::XmlWriter write(stream);
	write.prop_begin("root_node");
	write.prop("Items", items);
	write.prop("Nothing", empty);
	write.prop("Objects", objs);
    write.prop_end();
	}

	string expected =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<root_node>\n"
		"    <Items>\n"
		"        <item>Legal characters: abcde</item>\n"
		"        <item>Illegal characters: &quot;&apos;&lt;&gt;&amp;&quot;&apos;&lt;&gt;&amp;&lt;&lt;&lt;&gt;</item>\n"
		"        <item>Numbers: 01234</item>\n"
		"    </Items>\n"
		"    <Nothing/>\n"
		"    <Objects>\n"
		"        <item>\n"
		"            <Name>John</Name>\n"
		"            <AGE>19</AGE>\n"
		"        </item>\n"
		"        <item>\n"
		"            <Name>Laura</Name>\n"
		"            <AGE>33</AGE>\n"
		"        </item>\n"
		"        <item>\n"
		"            <Name>Nancy</Name>\n"
		"            <AGE>25</AGE>\n"
		"        </item>\n"
		"    </Objects>\n"
		"</root_node>\n";

	std::cout <<stream.str() <<std::endl;
	std::cout <<expected <<std::endl;


	CPPUNIT_ASSERT(stream.str()==expected);
}
