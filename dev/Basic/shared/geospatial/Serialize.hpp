/* Copyright Singapore-MIT Alliance for Research and Technology */

/**
 * \file Serialize.hpp

 * This file contains a set of serialization functions (boost::serialize) for the Geospatial road
 *   network classes which are compatible with boost::xml.
 * Be careful when including this file, as it has a large number of dependencies coupled with template
 *   expansion.
 *
 * \note
 * In terms of justification for why we put all of these functions outside of their respective classes,
 * there are three. First, I don't want tiny classes like Point2D and Node having boost::xml as a
 * dependency. Second, no "friends" are needed if the proper accessors are provided. Third, a friend
 * class would have been needed anyway in boost::serialization::access, so we might as well remove
 * these functions. The downside is that changes to the RoadNetwork classes will force Serialize.hpp to
 * recompile, but that is mostly unavoidable (and the classes are mostly stable now anyway). If performance
 * really becomes a problem, we can force the template into a separate cpp file (although it's messy).
 *
 * \note
 * These serialization functions are used for *writing* only; they are untested and unintended for reading.
 *
 * \author Seth N. Hetu
 */


#pragma once

#include <iostream>
#include <stdexcept>
#include <boost/noncopyable.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>

#include "geospatial/RoadNetwork.hpp"

namespace sim_mob {
namespace xml {

namespace { //TODO: Move this into its own implementation file, later.
const int TabSize = 4;
} //End un-named namespace


/**
 * Class which allows for simple recursive writing of an XML file.
 * The recursive descent through external methods is based somewhat on boost::serialization,
 * but is far more narrow in focus (writing XML only).
 */
class xml_writer : private boost::noncopyable {
public:
	xml_writer(std::ostream& outFile, int tabCount=0) : outFile(&outFile), tabCount(tabCount), sealedAttr(false) {}

	void header() {
		(*outFile) <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" <<std::endl;
		(*outFile) <<std::endl;
	}

	void attr(const std::string& key, const std::string& val) {
		if (sealedAttr) { throw std::runtime_error("Can't write attribute; a property has already been written"); }
		(*outFile) <<" " <<key <<"=\"" <<val <<"\"";
	}

	void prop(const std::string& key) {
		seal_attrs();
		(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<"/>" <<std::endl;
	}

	template <class T>
	void prop(const std::string& key, const T& val);

private:
	void seal_attrs() {
		if (!sealedAttr) {
			if (tabCount>0) {
				(*outFile) <<">" <<std::endl;
			}
			sealedAttr = true;
		}
	}

	template <class T>
	void write_simple_prop(const std::string& key, const T& val) {
		seal_attrs();
		(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<">" <<val <<"</" <<key <<">" <<std::endl;
	}

private:
	std::ostream* outFile;
	int tabCount;    //For indentation
	bool sealedAttr; //Are we done with writing attributes?
};


template <>
void xml_writer::prop(const std::string& key, const std::string& val)
{
	write_simple_prop(key, val);
}

template <>
void xml_writer::prop(const std::string& key, const int& val)
{
	write_simple_prop(key, val);
}

template <>
void xml_writer::prop(const std::string& key, const unsigned int& val)
{
	write_simple_prop(key, val);
}

template <>
void xml_writer::prop(const std::string& key, const size_t& val)
{
	write_simple_prop(key, val);
}

template <>
void xml_writer::prop(const std::string& key, const bool& val)
{
	write_simple_prop(key, val);
}

template <class T>
void xml_writer::prop(const std::string& key, const T& val)
{
	seal_attrs();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key;

	//Recurse. Pass a copy of the xml_writer to make this easier.
	xml_writer next_writer(*outFile, tabCount+1);
	write_xml(next_writer, val);
	next_writer.seal_attrs(); //Just in case this element is empty.

	//Close tab.
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"</" <<key <<">" <<std::endl;
}




//TODO: Serialization functions go here.
void write_xml(xml_writer& write, const sim_mob::RoadNetwork& rn) {
	//TODO: These go into lightweight containers later.
	//TODO: Or, we can do something like "prop_start", "prop_end" that allows us to fake simple containers.
	write.attr("xmlns:geo", "http://www.smart.mit.edu/geo");
	write.attr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	write.attr("xsi:schemaLocation", "http://www.smart.mit.edu/geo file:/home/vahid/Desktop/geo8/geo10.xsd");
	write.prop("my_prop", rn.drivingSide);
}

void write_xml(xml_writer& write, const sim_mob::DRIVING_SIDE& temp) {
	write.prop("internal_prop");
}

}}
