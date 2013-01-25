/* Copyright Singapore-MIT Alliance for Research and Technology */

//NOTE: Do *not* replace the following ifdef guard wtih a "pragma once" ---we use this defined file to
//      ensure that users don't include items in the "internal" folder directly.
#ifndef INCLUDE_UTIL_XML_WRITER_HPP
#define INCLUDE_UTIL_XML_WRITER_HPP

/**
 * \file XmlWriter.hpp
 *
 * Provides functionality for writing XML files in as simple a manner as possible.
 */

//NOTE: There is a bug regarding default values; see XmlWriterUnitTests.cpp for details.
//TODO: To fix this, we need to modify the library so that *all* items have namers and expanders, and
//      the write_xml() function for vectors simply checks if "namer.currLeaf()" is empty.
//      Users will be somewhat hidden from the complexities of this scheme, as all "namers() and "expanders()"
//      passed in to value-types will be empty (properties have already been printed, and get_id() would have
//      been dispatched if it was given)  ---and if they're writing container-type functions they will
//      appreciate the simplicity of only having to override one write_xml function instead of four.


#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>

#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>


//Get our namer definitions.
#include "internal/namer.hpp"


///Create a specialization of get_id() for a given "classname" that calls "function" and
///  uses boost::lexical_cast to return a string of that result.
///#defines are usually evil, but we can slim things down a bit by using them for get_id()
#ifndef SPECIALIZE_GET_ID
#define SPECIALIZE_GET_ID(classname, function)  template <> \
	std::string get_id(const classname& item) { \
		return boost::lexical_cast<std::string>(item.function()); }
#endif

///Create a specialization of get_id() that throws an error at runtime. Used to effectively
///  "erase" an item from the "<id>" expander.
#ifndef ERASE_GET_ID
#define ERASE_GET_ID(classname)  template <> \
	std::string get_id(const classname& item) { \
		throw std::runtime_error("Invalid get_id template for class " #classname); }
#endif


namespace sim_mob {
namespace xml {

//Forward-declare our class.
class XmlWriter;

//Function prototypes: generally you should override these, and not the ones with the
// "naming" parameter.

/**
 * Generic function for writing an item of type T (usually a class). You must provide
 * a specialization to this function if you want your class to be serializable.
 * E.g., template <> void write_xml(XmlWriter& write, const MyClass& item) { --do something-- }
 */
template <class T>
void write_xml(XmlWriter&, const T&);

/**
 * Generic function for retrieving the ID of an item of type T (usually a class). You must provide
 * a specialization to this function if you want your class to be serializable, EVEN IF YOU NEVER
 * SAVE IT BY ID.
 * E.g., template <> std::string get_id(const MyClass& item) { --do something-- }
 */
template <class T>
std::string get_id(const T&);


}} //End namespace sim_mob::xml


//Add some useful over-rides that forward write_xml
#include "internal/prop_forward.hpp"


//We need this hack to get around a cyclical dependency (the alternative is to prototype
//  *all* our template overrides, and it seems likely that someone will forget to do this).
namespace {
template <class T>
//This function calls write.prop(<other_args>);
void dispatch_write_xml_request(sim_mob::xml::XmlWriter& write, const std::string& key, const T& val, sim_mob::xml::namer name, sim_mob::xml::expander expand, bool writeValue);
} //End un-named namespace.



//We need overrides listed here (before XmlWriter) since functions cannot have partial template specializations.
//If you do not understand the difference between partial specialization and overriding template functions,
//then you should not be editing the XmlWriter.
#include "internal/overrides.hpp"


//Now include the actual class definition.
#include "internal/xml_writer.hpp"
#include "internal/xml_writer_imp.hpp"


//Actually define our hack function here.
namespace {
template <class T>
void dispatch_write_xml_request(sim_mob::xml::XmlWriter& write, const std::string& key, const T& val, sim_mob::xml::namer name, sim_mob::xml::expander expand, bool writeValue)
{
	//Try to be a little careful of types here, or our simple_prop property may not work right.
	//TODO: This should really be handled properly by write.prop()'s partial specialization...
	if (writeValue && name.isEmpty() && expand.isEmpty()) {
		write.prop(key, val);
	} else if (writeValue && expand.isEmpty()) {
		write.prop(key, val, name);
	} else if (name.isEmpty()) {
		write.prop(key, val, expand, writeValue);
	} else {
		write.prop(key, val, name, expand, writeValue);
	}
}
} //End un-named namespace


//And finally, no template hack would be complete without workarounds
#include "internal/workarounds.hpp"



#endif //INCLUDE_UTIL_XML_WRITER_HPP
