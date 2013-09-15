//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#ifndef INCLUDE_UTIL_XML_WRITER_HPP
 #error Do not include util/internal/prop_forward.hpp directly; instead, include util/XmlWriter.hpp
#else //INCLUDE_UTIL_XML_WRITER_HPP


namespace sim_mob {
namespace xml {



/**
 * Serialize with a namer and expand option. Non-container types should never have to override these,
 * and in general most necessary STL containers have already had them overridden. If you are implementing
 * your own container class (or serializing a complex entity like vector<pair<vector<string>,string>>)
 * then you should put all of your functionality into this function, and have defaults specified in
 * the variants which don't provide a namer or an expander. See XmlWriter.hpp and its included files
 * for examples of how we did this for STL container classes.
 *
 * Note that these namer/expander versions of write_xml will dispatch up to the basci write_xml
 * with no additional arguments *if* the namer and expander are empty. So you should never have to
 * override these functions for your own classes; just override the one that takes no namer/expander.
 */
template <class T>
void write_xml(XmlWriter& wr, const T& it, const namer& name, const expander& expand) {
	//This provides a little protection against improperly formatted stings; since we're already using
	//  extremely loose type checking (via strings), we can only prevent so much.
	if (!(name.isEmpty() && expand.isEmpty())) {
		throw std::runtime_error("Can't dispatch write_xml to a non-leaf node; make sure your namers line up!");
	}

	//A null namer is essentially *no* namer, so remove it.
	write_xml(wr, it);
}

///Write with a namer, no expander. See also: xml_writer() with name and expander.
template <class T>
void write_xml(XmlWriter& wr, const T& it, const namer& name) {
	//This provides a little protection against improperly formatted stings; since we're already using
	//  extremely loose type checking (via strings), we can only prevent so much.
	if (!(name.isEmpty())) {
		throw std::runtime_error("Can't dispatch write_xml to a non-leaf node; make sure your namers line up!");
	}

	//A null namer is essentially *no* namer, so remove it.
	write_xml(wr, it);
}

///Write with an expander, no namer. See also: xml_writer() with name and expander.
template <class T>
void write_xml(XmlWriter& wr, const T& it, const expander& expand) {
	//This provides a little protection against improperly formatted stings; since we're already using
	//  extremely loose type checking (via strings), we can only prevent so much.
	if (!(expand.isEmpty())) {
		throw std::runtime_error("Can't dispatch write_xml to a non-leaf node; make sure your namers line up!");
	}

	//A null namer is essentially *no* namer, so remove it.
	write_xml(wr, it);
}

}}

#endif //INCLUDE_UTIL_XML_WRITER_HPP
