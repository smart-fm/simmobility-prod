/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "XmlWriterUnitTests.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <map>
#include <ostream>

#include <boost/lexical_cast.hpp>

#include "util/XmlWriter.hpp"

using std::string;
using std::vector;
using std::map;
using std::pair;

namespace {

//Replace all occurrences of "old" with "new" in a string.
void str_replace_all(string& source, const string& oldStr, const string& newStr) {
	size_t nextPos = 0;
	while((nextPos = source.find(oldStr, nextPos)) != string::npos) {
		source.replace(nextPos, oldStr.length(), newStr);
		nextPos += newStr.length();
	}
}

} //End unnamed namespace


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

    bool const operator<(const Person &other) const {
        return name<other.name || ((name==other.name) && (age<other.age));
    }
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
ERASE_GET_ID(Person);
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


	CPPUNIT_ASSERT(stream.str()==expected);
}



//Some more simple structs.
struct Vehicle {
	Vehicle(const string& vehClass, int length) : vehClass(vehClass), length(length) {}
	string vehClass;
	int length;
};
struct Vehicle2 {
	Vehicle2(const string& vehClass) : vehClass(vehClass) {}
	string vehClass;
};

//Write these structs with the "vehClass" as an attribute.
namespace sim_mob {
namespace xml {
template <>
void write_xml(XmlWriter& write, const Vehicle& veh)
{
	write.attr("v_class", veh.vehClass);
	write.prop("length", veh.length);
}
template <>
void write_xml(XmlWriter& write, const Vehicle2& veh)
{
	write.attr("v_class", veh.vehClass);
}
ERASE_GET_ID(Vehicle);
ERASE_GET_ID(Vehicle2);
}} //End namespace sim_mob::xml

void unit_tests::XmlWriterUnitTests::test_AttributesXML()
{
	//Prepare our data
	vector<Vehicle> first;
	vector<Vehicle2> second;
	first.push_back(Vehicle("car", 10));
	first.push_back(Vehicle("taxi", 20));
	first.push_back(Vehicle("car", 30));
	second.push_back(Vehicle2("car"));
	second.push_back(Vehicle2("boat"));
	second.push_back(Vehicle2("jet"));

	//Write our data
	std::ostringstream stream;
	{
	sim_mob::xml::XmlWriter write(stream);
	write.prop("Predefined", first);
	write.prop("Runtime", second);
	}

	string expected =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<Predefined>\n"
		"    <item v_class=\"car\">\n"
		"        <length>10</length>\n"
		"    </item>\n"
		"    <item v_class=\"taxi\">\n"
		"        <length>20</length>\n"
		"    </item>\n"
		"    <item v_class=\"car\">\n"
		"        <length>30</length>\n"
		"    </item>\n"
		"</Predefined>\n"
		"<Runtime>\n"
		"    <item v_class=\"car\"/>\n"
		"    <item v_class=\"boat\"/>\n"
		"    <item v_class=\"jet\"/>\n"
		"</Runtime>\n";

	CPPUNIT_ASSERT(stream.str()==expected);
}


void unit_tests::XmlWriterUnitTests::test_NamersXML()
{
	//Our generic data
	vector<Vehicle> cars;
	vector< pair<Vehicle2,string> > taxis;
	map<int, string> lookup;
	map< pair<Person,int>, pair<string,string> > complex;
	cars.push_back(Vehicle("car", 20));
	cars.push_back(Vehicle("taxi", 19));
	cars.push_back(Vehicle("car", 18));
	taxis.push_back(std::make_pair(Vehicle2("taxi"), "Jurong"));
	taxis.push_back(std::make_pair(Vehicle2("taxi"), "Orchard"));
	taxis.push_back(std::make_pair(Vehicle2("taxi"), "Marina Bay"));
	lookup[1] = "one";
	lookup[5] = "five";
	lookup[33] = "thirty-three";
	complex[pair<Person,int>(Person("Sam",10), 13)] = std::make_pair("Traffic Light","red");
	complex[pair<Person,int>(Person("Alex",-100), 99)] = std::make_pair("Intersection","congested");

	//Use a pseudo-template string to make matching multiple results easier.
	const string templateStr =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<Cars>\n"
		"    <#{CARS_ITEM} v_class=\"car\">\n"
		"        <length>20</length>\n"
		"    </#{CARS_ITEM}>\n"
		"    <#{CARS_ITEM} v_class=\"taxi\">\n"
		"        <length>19</length>\n"
		"    </#{CARS_ITEM}>\n"
		"    <#{CARS_ITEM} v_class=\"car\">\n"
		"        <length>18</length>\n"
		"    </#{CARS_ITEM}>\n"
		"</Cars>\n"
		"<Taxis>\n"
		"    <#{TAXIS_ITEM}>\n"
		"        <#{TAXIS_ITEM_FIRST} v_class=\"taxi\"/>\n"
		"        <#{TAXIS_ITEM_SECOND}>Jurong</second>\n"
		"    </#{TAXIS_ITEM}>\n"
		"    <#{TAXIS_ITEM}>\n"
		"        <#{TAXIS_ITEM_FIRST} v_class=\"taxi\"/>\n"
		"        <#{TAXIS_ITEM_SECOND}>Orchard</second>\n"
		"    </#{TAXIS_ITEM}>\n"
		"    <#{TAXIS_ITEM}>\n"
		"        <#{TAXIS_ITEM_FIRST} v_class=\"taxi\"/>\n"
		"        <#{TAXIS_ITEM_SECOND}>Marina Bay</second>\n"
		"    </#{TAXIS_ITEM}>\n"
		"</Taxis>\n"
		"<Lookup>\n"
		"    <#{LOOKUP_ITEM}>\n"
		"        <#{LOOKUP_ITEM_KEY}>1</key>\n"
		"        <#{LOOKUP_ITEM_VALUE}>one</value>\n"
		"    </#{LOOKUP_ITEM}>\n"
		"    <#{LOOKUP_ITEM}>\n"
		"        <#{LOOKUP_ITEM_KEY}>5</key>\n"
		"        <#{LOOKUP_ITEM_VALUE}>five</value>\n"
		"    </#{LOOKUP_ITEM}>\n"
		"    <#{LOOKUP_ITEM}>\n"
		"        <#{LOOKUP_ITEM_KEY}>33</key>\n"
		"        <#{LOOKUP_ITEM_VALUE}>thirty-three</value>\n"
		"    </#{LOOKUP_ITEM}>\n"
		"</Lookup>\n"
		"<Complex>\n"
		"    <#{COMPLEX_ITEM}>\n"
		"        <#{COMPLEX_ITEM_KEY}>\n"
		"            <#{COMPLEX_ITEM_KEY_FIRST}>\n"
		"                <Name>Alex</Name>\n"
		"                <AGE>-100</AGE>\n"
		"            </#{COMPLEX_ITEM_KEY_FIRST}>\n"
		"            <#{COMPLEX_ITEM_KEY_SECOND}>99</#{COMPLEX_ITEM_KEY_SECOND}>\n"
		"        </#{COMPLEX_ITEM_KEY}>\n"
		"        <#{COMPLEX_ITEM_VALUE}>\n"
		"            <#{COMPLEX_ITEM_VALUE_FIRST}>Intersection</#{COMPLEX_ITEM_VALUE_FIRST}>\n"
		"            <#{COMPLEX_ITEM_VALUE_SECOND}>congested</#{COMPLEX_ITEM_VALUE_SECOND}>\n"
		"        </#{COMPLEX_ITEM_VALUE}>\n"
		"    </#{COMPLEX_ITEM}>\n"
		"    <#{COMPLEX_ITEM}>\n"
		"        <#{COMPLEX_ITEM_KEY}>\n"
		"            <#{COMPLEX_ITEM_KEY_FIRST}>\n"
		"                <Name>Sam</Name>\n"
		"                <AGE>10</AGE>\n"
		"            </#{COMPLEX_ITEM_KEY_FIRST}>\n"
		"            <#{COMPLEX_ITEM_KEY_SECOND}>13</#{COMPLEX_ITEM_KEY_SECOND}>\n"
		"        </#{COMPLEX_ITEM_KEY}>\n"
		"        <#{COMPLEX_ITEM_VALUE}>\n"
		"            <#{COMPLEX_ITEM_VALUE_FIRST}>Traffic Light</#{COMPLEX_ITEM_VALUE_FIRST}>\n"
		"            <#{COMPLEX_ITEM_VALUE_SECOND}>red</#{COMPLEX_ITEM_VALUE_SECOND}>\n"
		"        </#{COMPLEX_ITEM_VALUE}>\n"
		"    </#{COMPLEX_ITEM}>\n"
		"</Complex>\n";

	//First test: default labels.
	{
	//Write
	std::ostringstream stream;
	{
	sim_mob::xml::XmlWriter write(stream);
	write.prop("Cars", cars);
	write.prop("Taxis", taxis);
	write.prop("Lookup", lookup);
	write.prop("Complex", complex);
	}

	//Substitute
	string expected(templateStr);
	str_replace_all(expected, "#{CARS_ITEM}", "item");
	str_replace_all(expected, "#{TAXIS_ITEM}", "item");
	str_replace_all(expected, "#{TAXIS_ITEM_FIRST}", "first");
	str_replace_all(expected, "#{TAXIS_ITEM_SECOND}", "second");
	str_replace_all(expected, "#{LOOKUP_ITEM}", "item");
	str_replace_all(expected, "#{LOOKUP_ITEM_KEY}", "key");
	str_replace_all(expected, "#{LOOKUP_ITEM_VALUE}", "value");
	str_replace_all(expected, "#{COMPLEX_ITEM}", "item");
	str_replace_all(expected, "#{COMPLEX_ITEM_KEY}", "key");
	str_replace_all(expected, "#{COMPLEX_ITEM_KEY_FIRST}", "first");
	str_replace_all(expected, "#{COMPLEX_ITEM_KEY_SECOND}", "second");
	str_replace_all(expected, "#{COMPLEX_ITEM_VALUE}", "value");
	str_replace_all(expected, "#{COMPLEX_ITEM_VALUE_FIRST}", "first");
	str_replace_all(expected, "#{COMPLEX_ITEM_VALUE_SECOND}", "second");

	//Check
	CPPUNIT_ASSERT(stream.str()==expected);
	}
}




