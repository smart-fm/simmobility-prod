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

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>

#include <boost/noncopyable.hpp>

//Internal includes
#include "internal/namer.hpp"


namespace sim_mob {
namespace xml {


//Forward-declare our class.
class XmlWriter;

//Function prototypes: generally you should override these, and not the ones with the
// "naming" parameter.
template <class T>
void write_xml(XmlWriter&, const T&);
template <class T>
std::string get_id(const T&);

//The function write_xml with a naming parameter can be used to override naming
// on vectors, sets, etc. Users should not override this one unless they are implementing their own containers.
//NOTE: The call to "prop()" will make sure you *always* get both name and expand. If either is empty,
//      that is when it is appropriate it use customizable defaults (e.g., "item" for vectors).
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

//Similarly, dispatch up for primitives on other partial function signatures.
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
}}  //End namespace sim_mob::xml


//We need this hack to get around a cyclical dependency (the alternative is to prototype
//  *all* our template overrides, and it seems likely that someone will forget to do this).
namespace {
template <class T>
//This function calls write.prop(<other_args>);
void dispatch_write_xml_request(sim_mob::xml::XmlWriter& write, const std::string& key, const T& val, sim_mob::xml::namer name, sim_mob::xml::expander expand, bool writeValue);
} //End un-named namespace.



//////////////////////////////////////////////////////////////////////////////////////////////
// Prototypes are required for template overrides (since XmlWriter needs to find them later).
//////////////////////////////////////////////////////////////////////////////////////////////

namespace sim_mob {
namespace xml {

//////////////////////////////////////////////////////////////////////
// get_id for const and non-const pointer types
//////////////////////////////////////////////////////////////////////
template <class T>
std::string get_id(const T* ptr)
{
	return get_id(*ptr);
}
template <class T>
std::string get_id(T* ptr)
{
	return get_id(*ptr);
}


//////////////////////////////////////////////////////////////////////
// get_id failure cases on container classes.
//////////////////////////////////////////////////////////////////////
template <class T>
std::string get_id(const std::set<T>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}
template <class T>
std::string get_id(const std::vector<T>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}
template <class T, class U>
std::string get_id(const std::pair<T, U>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}
template <class T, class U>
std::string get_id(const std::map<T, U>& temp)
{
	throw std::runtime_error("Cannot call get_id() on STL containers.");
}


//////////////////////////////////////////////////////////////////////
// get_id() failure cases for primitive tpyes.
//////////////////////////////////////////////////////////////////////
template <>
std::string get_id(const std::string& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <>
std::string get_id(const int& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <>
std::string get_id(const unsigned int& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <>
std::string get_id(const long& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <>
std::string get_id(const unsigned long& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <>
std::string get_id(const double& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}

template <>
std::string get_id(const bool& temp)
{
	throw std::runtime_error("Cannot call get_id() on primitive types.");
}



//////////////////////////////////////////////////////////////////////
// Xml writers for const pointer types
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr, namer name, expander expand)
{
	write_xml(write, *ptr, name, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr, namer name)
{
	write_xml(write, *ptr, name);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr, expander expand)
{
	write_xml(write, *ptr, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const T* ptr)
{
	write_xml(write, *ptr);
}


//////////////////////////////////////////////////////////////////////
// Xml writers for non-const pointer types
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr, namer name, expander expand)
{
	write_xml(write, *ptr, name, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr, namer name)
{
	write_xml(write, *ptr, name);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr, expander expand)
{
	write_xml(write, *ptr, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, T* ptr)
{
	write_xml(write, *ptr);
}


//////////////////////////////////////////////////////////////////////
// Xml writers for pair<T,U>
//////////////////////////////////////////////////////////////////////
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr, namer name, expander expand)
{
	//Propagate; use the left/right children for this.
	dispatch_write_xml_request(write, name.leftStr(), pr.first, name.leftChild(), expand.leftChild(), expand.leftIsValue());
	dispatch_write_xml_request(write, name.rightStr(), pr.second, name.rightChild(), expand.rightChild(), expand.rightIsValue());
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr, namer name)
{
	write_xml(write, pr, name, expander());
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr, expander expand)
{
	write_xml(write, pr, namer("<first,second>"), expand);
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T, U>& pr)
{
	write_xml(write, pr, namer("<first,second>"), expander());
}


//////////////////////////////////////////////////////////////////////
// Xml writers for set<T,U>
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec, namer name, expander expand)
{
	//Print each item as a separate property.
	for (typename std::set<T>::const_iterator it=vec.begin(); it!=vec.end(); it++) {
		dispatch_write_xml_request(write, name.leftStr(), *it, name.rightChild(), expand.rightChild(), expand.leftIsValue());
	}
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec, namer name)
{
	write_xml(write, vec, name, expander());
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec, expander expand)
{
	write_xml(write, vec, namer("<item>"), expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T>& vec)
{
	write_xml(write, vec, namer("<item>"), expander());
}


//////////////////////////////////////////////////////////////////////
// Xml writers for vector<T,U>
//////////////////////////////////////////////////////////////////////
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec, namer name, expander expand)
{
	//Print each item as a separate property.
	for (typename std::vector<T>::const_iterator it=vec.begin(); it!=vec.end(); it++) {
		dispatch_write_xml_request(write, name.leftStr(), *it, name.rightChild(), expand.rightChild(), expand.leftIsValue());
	}
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec, namer name)
{
	write_xml(write, vec, name, expander());
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec, expander expand)
{
	write_xml(write, vec, namer("<item>"), expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec)
{
	write_xml(write, vec, namer("<item>"), expander());
}


//////////////////////////////////////////////////////////////////////
// Xml writers for map<T,U>
//////////////////////////////////////////////////////////////////////
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items, namer name, expander expand)
{
	//Print each item as a separate property.
	for (typename std::map<T,U>::const_iterator it=items.begin(); it!=items.end(); it++) {
		dispatch_write_xml_request(write, name.leftStr(), *it, name.rightChild(), expand.rightChild(), expand.leftIsValue());
	}
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items, namer name)
{
	write_xml(write, items, name, expander());
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items, expander expand)
{
	write_xml(write, items, namer("<item,<key,value>>"), expand);
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::map<T, U>& items)
{
	write_xml(write, items, namer("<item,<key,value>>"), expander());
}





/**
 * Class which allows for simple recursive writing of an XML file.
 * The recursive descent through external methods is based somewhat on boost::serialization,
 * but is far more narrow in focus (writing XML only).
 *
 * You can use sim_mob::BoostSaveXML() in geospatial/xmlWriter/boostXmlWriter.hpp to avoid the
 * intricacies of this file.
 *
 * TODO: Can we move that functionality into a static function of the XmlWriter class?
 *
 * If you want your class to be xml-serializable, you should create a public function called "write_xml()"
 * with the required functionality. See geospatial/Serialize.hpp for some examples.
 *
 */
class XmlWriter : private boost::noncopyable {
public:
	///Create a new XmlWriter which will send output to "outFile". Writes the header.
	XmlWriter(std::ostream& outFile) :
		outFile(&outFile), tabCount(0), newlines(0), attPrefix(" "), sealedAttr(false)
	{
		header();
	}

public:
	///Inform the serializer to write an extra newline before the next property or identifier.
	///Only affects the very next item.
	void endl();

	///Specify a string that will appear before all attributes in this property.
	void attr_prefix(const std::string& pre);

	///Write an attribute on the current property.
	void attr(const std::string& key, const std::string& val);

	///Write a simple property (key-value pair).
	template <class T>
	void prop(const std::string& key, const T& val);

	///Write a slightly more complex property.
	///If "writeValue" is false, reduce the "val" argument to an integer ID (don't print its full expansion).
	template <class T>
	void prop(const std::string& key, const T& val, namer name, expander expand=expander(""), bool writeValue=true);

	//Expander-only version.
	template <class T>
	void prop(const std::string& key, const T& val, expander expand, bool writeValue=true) { prop(key, val, namer(""), expand, writeValue); }

	///Begin writing a property. This is used instead of prop() for classes which don't exist
	/// (i.e., containers within the XML file itself).
	void prop_begin(const std::string& key);

	///Pair with prop_begin()
	void prop_end() { prop_end(false); } //Java-style dispatch.

	///Returns the property that is currently being written.
	std::string curr_prop() { return propStack.back(); }

private:
	//The actual prop_end function. (We don't allow public-function users to ignor tabs, since there's no legitimate reason to do so.)
	void prop_end(bool ignoreTabs);

	///Write this file's header. This will be called first, as it prints the "<?xml..." tag.
	void header();

	//Write all pending newlines
	void write_newlines();

	//Seal attributes; this appends a ">" to the current property and sets the sealed flag.
	//Has no effect on the very first element (due to the <?xml... closing itself).
	void seal_attrs(bool appendNewline=true);

	//Write a simple property (e.g., a primitive). Relies on operator<< for printing.
	template <class T>
	void write_simple_prop(const std::string& key, const T& val);

private:
	std::ostream* const outFile;
	int tabCount;          //For indentation
	int newlines;          //How many newlines to write next.
	std::string attPrefix; //The current prefix for all attributes of this property.
	std::vector<std::string> propStack;  //The property that we are currently writing is at the back of the vector.
	bool sealedAttr;       //Are we done with writing attributes?
};




}}  //End namespace sim_mob::xml


//Helper namespace for writing lists.
namespace {


//Helper: Escape illegal symbols inside XML elements/attributes
std::string escape_xml(const std::string& src) {
	//Shortcut: no characters to replace?
	if (src.find_first_of("\"'<>&")==std::string::npos) { return src; }

	std::stringstream res;
	for (std::string::const_iterator it=src.begin(); it!=src.end(); it++) {
		     if (*it=='"')  { res <<"&quot;"; }
		else if (*it=='\'') { res <<"&apos;"; }
		else if (*it=='<')  { res <<"&lt;"; }
		else if (*it=='>')  { res <<"&gt;"; }
		else if (*it=='&')  { res <<"&amp;"; }
		else                { res <<*it; }
	}
	return res.str();
}
} //End anon namespace


///////////////////////////////////////////////////////////////////
// Template implementation
///////////////////////////////////////////////////////////////////




namespace { //TODO: Move this into its own implementation file, later.
const int TabSize = 4;
} //End un-named namespace

void sim_mob::xml::XmlWriter::attr(const std::string& key, const std::string& val)
{
	if (sealedAttr) { throw std::runtime_error("Can't write attribute; a property has already been written"); }
	(*outFile) <<attPrefix <<key <<"=\"" <<escape_xml(val) <<"\"";
}

void sim_mob::xml::XmlWriter::endl()
{
	newlines++;
}

void sim_mob::xml::XmlWriter::attr_prefix(const std::string& pre)
{
	attPrefix = pre;
}

void sim_mob::xml::XmlWriter::header()
{
	(*outFile) <<"<?xml version=\"1.0\" encoding=\"utf-8\"?>" <<std::endl;
}

void sim_mob::xml::XmlWriter::write_newlines()
{
	if (newlines>0) {
		(*outFile) <<std::string(newlines, '\n');
		newlines = 0;
	}
}

void sim_mob::xml::XmlWriter::seal_attrs(bool appendNewline) {
	if (!sealedAttr) {
		if (tabCount>0) {
			(*outFile) <<">";
			if (appendNewline) {
				(*outFile) <<std::endl;
			}
		}
		sealedAttr = true;

		//Attribute prefixes won't apply to the next property, and can't apply to this one any more (no more attributes).
		attPrefix = " ";
	}
}

template <class T>
void sim_mob::xml::XmlWriter::write_simple_prop(const std::string& key, const T& val)
{
	seal_attrs();
	write_newlines();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<">" <<val <<"</" <<key <<">" <<std::endl;
}


//These functions do all the heavy lifting; dispatching as appropriate.
template <class T>
void sim_mob::xml::XmlWriter::prop(const std::string& key, const T& val)
{
	//Recurse
	prop_begin(key);
	write_xml(*this, val);
	prop_end();
}


template <class T>
void sim_mob::xml::XmlWriter::prop(const std::string& key, const T& val, namer name, expander expand, bool writeValue)
{
	//Begin.
	//TODO: If Exp_ID, then a full "prop_begin()" and "prop_end()" are wasteful
	//      (since we have no reason to save state information). We can fix this by
	//      delegating the entire function to prop() (listed directly above) if Exp_Value,
	//      and putting ident()'s code in the "else" block, but right now it doesn't matter
	//      as long as it works.
	prop_begin(key);

	//Recurse
	bool ignoreTabs = false;
	if (!writeValue) {
		seal_attrs(false); //Adds the ">"
		(*outFile) <<get_id(val);
		ignoreTabs = true;
	} else {
		//In this case we simply dispatch. However, we must be careful to discard any un-used "name" or
		// "expand" classes, since value-type versions of write_xml() shouldn't have to catch these
		// properties, and array-types use empty parameters to dispatch defaults (so it's necessary).
		//In the end, it's a bit complicated, but fortunately it's all internal to this class.
		if (!name.isEmpty() && !expand.isEmpty()) {
			write_xml(*this, val, name, expand);
		} else if (!name.isEmpty()) {
			write_xml(*this, val, name);
		} else if (!expand.isEmpty()) {
			write_xml(*this, val, expand);
		} else {
			write_xml(*this, val);
		}
	}

	//End
	prop_end(ignoreTabs);
}



void sim_mob::xml::XmlWriter::prop_begin(const std::string& key)
{
	seal_attrs();
	write_newlines();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key;

	//Next indentation/sealed level.
	tabCount++;
	sealedAttr = false;
	propStack.push_back(key);
}

void sim_mob::xml::XmlWriter::prop_end(bool ignoreTabs)
{
	//Restore indentation
	std::string key = propStack.back();
	propStack.pop_back();
	tabCount--;

	//Close tab.
	if (sealedAttr) { //Did we write at least one property?
		if (!ignoreTabs) {
			(*outFile) <<std::string(tabCount*TabSize, ' ');
		}
		(*outFile) <<"</" <<key <<">" <<std::endl;
	} else {
		(*outFile) <<"/>" <<std::endl;
	}

	//Closing will "seal" this property, even if we haven't called seal_attrs();
	sealedAttr = true;
}



////////////////////////////////////////////////////////////////////
// Template specializations for primitive output.
// NOTE: Some of these are not specializations, but rather overrides! Grr.....
// Add any basic types here (you can even add tpyes such as Point2D which
// have an output operator if you prefer).
////////////////////////////////////////////////////////////////////


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


namespace sim_mob { //Function specializations require an explicit namespace wrapping.
namespace xml {



//////////////////////////////////////////////////////////////////////
// Xml writers for certain primitives.
//////////////////////////////////////////////////////////////////////

template <>
void XmlWriter::prop(const std::string& key, const std::string& val)
{
	write_simple_prop(key, escape_xml(val));
}

template <>
void XmlWriter::prop(const std::string& key, const int& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const unsigned int& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const long& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const unsigned long& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const double& val)
{
	std::streamsize prec = outFile->precision();
	outFile->precision(4);
	outFile->setf(std::ios::fixed);
	write_simple_prop(key, val);
	outFile->precision(prec);
	outFile->unsetf(std::ios::fixed);
}

template <>
void XmlWriter::prop(const std::string& key, const bool& val)
{
	write_simple_prop(key, val?"true":"false");
}


//////////////////////////////////////////////////////////////////////
// TODO: These are a problem (needed for compilation, but shouldn't ever be used).
//       For now we just do this.
//////////////////////////////////////////////////////////////////////

template <>
void write_xml(sim_mob::xml::XmlWriter& write, const std::string& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <>
void write_xml(sim_mob::xml::XmlWriter& write, const int& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <>
void write_xml(sim_mob::xml::XmlWriter& write, const unsigned int& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <>
void write_xml(sim_mob::xml::XmlWriter& write, const long& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <>
void write_xml(sim_mob::xml::XmlWriter& write, const unsigned long& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <>
void write_xml(sim_mob::xml::XmlWriter& write, const double& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}

template <>
void write_xml(sim_mob::xml::XmlWriter& write, const bool& temp)
{
	throw std::runtime_error("write_xml() was somehow accidentally called with a primitive type.");
}


}}


#endif //INCLUDE_UTIL_XML_WRITER_HPP
