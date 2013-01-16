/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * \file XmlWriter.hpp
 *
 * Provides functionality for writing XML files in as simple a manner as possible.
 */

#include <vector>
#include <string>
#include <sstream>
#include <map>

namespace sim_mob {
namespace xml {

/**
 * Template class used for specifying the name that elements in an STL container can take.
 * This class serves to address a particular problem: we provide helper write_xml() wrappers
 * for vectors, sets, etc., but what happens if the user has two different sets, and wants
 * to name the elements in each set differently? What if we also want to provide a sensible default,
 * such as "item" for vectors?
 *
 * To accomplish this, you can pass in a "naming", using syntax such as "naming<"item">"
 * or "naming<"roadSegment">". These should be chain-able (e.g., a naming for vectors of pairs)
 * ---indeed, that's the whole point.
 */
struct naming {
	naming(const std::string& name) : name(name) {}
	std::string name;
};



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
	template <class T>
	void prop(const std::string& key, const T& val, naming name);

	///Write a list (key-list-of-values)
	template <class T>
	void list(const std::string& plural, const std::string& singular, const T& val);

	///Write a property as using identifiers instead of values. This requires
	/// the required base type to have a corresponding get_id() override.
	template <class T>
	void ident(const std::string& key, const T& val);

	///Write an identifier that consists of a pair of values.
	template <class T, class U>
	void ident(const std::string& key, const T& first, const U& second);

	///Write a list of identifiers
	template <class T>
	void ident_list(const std::string& plural, const std::string& singular, const T& val);

	///Begin writing a property. This is used instead of prop() for classes which don't exist
	/// (i.e., containers within the XML file itself).
	void prop_begin(const std::string& key);

	///Pair with prop_begin()
	void prop_end();

	///Returns the property that is currently being written.
	std::string curr_prop() { return propStack.back(); }

private:
	///Write this file's header. This will be called first, as it prints the "<?xml..." tag.
	void header();

	//Write all pending newlines
	void write_newlines();

	//Seal attributes; this appends a ">" to the current property and sets the sealed flag.
	//Has no effect on the very first element (due to the <?xml... closing itself).
	void seal_attrs();

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


//Function prototypes: generally you should override these, and not the ones with the
// "naming" parameter.
template <class T>
void write_xml(XmlWriter&, const T&);
template <class T>
std::string get_id(const T&);

//The function write_xml with a naming parameter can be used to override naming
// on vectors, sets, etc.
template <class T>
void write_xml(XmlWriter&, const T&, naming name);

}}  //End namespace sim_mob::xml


//Helper namespace for writing lists.
namespace {
///Write a generic list/map/whatever via iterators
template <class IterType>
void write_value_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end)
{
	std::string singular = write.curr_prop();
	for (; begin!=end; begin++) {
		write.prop(singular, *begin);
	}
}
template <class IterType>
void write_pointer_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end, sim_mob::xml::naming name=sim_mob::xml::naming(""))
{
	std::string singular = name.name.empty() ? write.curr_prop() : name.name;
	for (; begin!=end; begin++) {
		write.prop(singular, **begin);
	}
}

///Same, but for identifiers.
template <class IterType>
void write_value_ident_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end)
{
	std::string singular = write.curr_prop();
	for (; begin!=end; begin++) {
		write.ident(singular, *begin);
	}
}
template <class IterType>
void write_pointer_ident_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end)
{
	std::string singular = write.curr_prop();
	for (; begin!=end; begin++) {
		write.ident(singular, **begin);
	}
}


///Attempts to write a vector of "items" as "item", "item", ... etc.
template <class T>
void write_xml_list(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec)
{
	write_value_array(write, vec.begin(), vec.end());
}
template <class T>
void write_xml_list(sim_mob::xml::XmlWriter& write, const std::vector<T*>& vec)
{
	write_pointer_array(write, vec.begin(), vec.end());
}

///Attempts to write a set of "items" as "item", "item", ... etc.
template <class T>
void write_xml_list(sim_mob::xml::XmlWriter& write, const std::set<T>& vec)
{
	write_value_array(write, vec.begin(), vec.end());
}
template <class T>
void write_xml_list(sim_mob::xml::XmlWriter& write, const std::set<T*>& vec)
{
	write_pointer_array(write, vec.begin(), vec.end());
}

///Write a list of identifiers.
template <class T>
void write_ident_list(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec)
{
	write_value_ident_array(write, vec.begin(), vec.end());
}
template <class T>
void write_ident_list(sim_mob::xml::XmlWriter& write, const std::vector<T*>& vec)
{
	write_pointer_ident_array(write, vec.begin(), vec.end());
}

///Same, but witha  set.
template <class T>
void write_ident_list(sim_mob::xml::XmlWriter& write, const std::set<T>& vec)
{
	write_value_ident_array(write, vec.begin(), vec.end());
}
template <class T>
void write_ident_list(sim_mob::xml::XmlWriter& write, const std::set<T*>& vec)
{
	write_pointer_ident_array(write, vec.begin(), vec.end());
}

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

void sim_mob::xml::XmlWriter::seal_attrs() {
	if (!sealedAttr) {
		if (tabCount>0) {
			(*outFile) <<">" <<std::endl;
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
void sim_mob::xml::XmlWriter::prop(const std::string& key, const T& val, naming name)
{
	//Recurse
	prop_begin(key);
	write_xml(*this, val, name);
	prop_end();
}


//Lists require a tiny bit of hacking
template <class T>
void sim_mob::xml::XmlWriter::list(const std::string& plural, const std::string& singular, const T& val)
{
	//Recurse
	prop_begin(plural);
	propStack.push_back(singular);
	write_xml_list(*this, val);
	propStack.pop_back();
	prop_end();
}


//Write an identifier
template <class T>
void sim_mob::xml::XmlWriter::ident(const std::string& key, const T& val)
{
	//Identifiers have no attributes or child properties, so they are relatively easy to serialize.
	seal_attrs();
	write_newlines();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<">";
	(*outFile) <<get_id(val) <<"</" <<key <<">" <<std::endl;
}

//Write a pair of identifiers
template <class T, class U>
void sim_mob::xml::XmlWriter::ident(const std::string& key, const T& first, const U& second)
{
	//Pairs of identifiers are relatively simple, but we are kind of breaking things down a bit.
	seal_attrs();
	write_newlines();

	//Pairs of identifiers require a little more scaffolding, but are not otherwise that complex.
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<"/>" <<std::endl;
	(*outFile) <<std::string((tabCount+1)*TabSize, ' ') <<"<first>" <<get_id(first) <<"</first>" <<std::endl;
	(*outFile) <<std::string((tabCount+1)*TabSize, ' ') <<"<second>" <<get_id(second) <<"</second>" <<std::endl;
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"</" <<key <<"/>" <<std::endl;
}

//Write a list of identifiers
template <class T>
void sim_mob::xml::XmlWriter::ident_list(const std::string& plural, const std::string& singular, const T& val)
{
	//Same as a regular ident(), but in list format.
	prop_begin(plural);
	propStack.push_back(singular);
	write_ident_list(*this, val);
	propStack.pop_back();
	prop_end();
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

void sim_mob::xml::XmlWriter::prop_end()
{
	//Restore indentation
	std::string key = propStack.back();
	propStack.pop_back();
	tabCount--;

	//Close tab.
	if (sealedAttr) { //Did we write at least one property?
		(*outFile) <<std::string(tabCount*TabSize, ' ') <<"</" <<key <<">" <<std::endl;
	} else {
		(*outFile) <<"/>" <<std::endl;
	}

	//Closing will "seal" this property, even if we haven't called seal_attrs();
	sealedAttr = true;
}



////////////////////////////////////////////////////////////////////
// Template specializations for primitive output.
// Add any basic types here (you can even add tpyes such as Point2D which
// have an output operator if you prefer).
////////////////////////////////////////////////////////////////////



namespace sim_mob { //Function specializations require an explicit namespace wrapping.
namespace xml {

//////////////////////////////////////////////////////////////////////
//Test function. Functions of this kind will replace the write_xml_list functions listed earlier.
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T*>& vec, naming name)
{
	write_pointer_array(write, vec.begin(), vec.end(), name);
}
//Two-parameter functions (for containers) pass *up*
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T*>& vec)
{
	write_pointer_array(write, vec.begin(), vec.end(), naming("item"));
}
//////////////////////////////////////////////////////////////////////



///Attempts to write a pair of "items" as "first", "second"
template <class T>
void write_xml(XmlWriter& write, const std::pair<T, T>& pr)
{
	write.prop("first", pr.first);
	write.prop("second", pr.second);
}
template <class T>
void write_xml(XmlWriter& write, const std::pair<T*, T*>& pr)
{
	write.prop("first", *pr.first);
	write.prop("second", *pr.second);
}
template <class T>
void write_xml(XmlWriter& write, const std::pair<const T*, const T*>& pr)
{
	write.prop("first", *pr.first);
	write.prop("second", *pr.second);
}
template <class T>
void write_xml(XmlWriter& write, const std::pair<T*, const T*>& pr)
{
	write.prop("first", *pr.first);
	write.prop("second", *pr.second);
}
template <class T>
void write_xml(XmlWriter& write, const std::pair<const T*, T*>& pr)
{
	write.prop("first", *pr.first);
	write.prop("second", *pr.second);
}

///Flatten a map into a vector of "item" pairs.
///TODO: This is somewhat inefficient; we can probably create a "write_xml" function for
//       std::maps too. Unfortunately, we need to handle the four combinations of <T, U>,
//       <T*, U>, <T, U*>, <T*, U*>, which gets messy.
//       We can get around this by declaring a "write_xml" function for T* that attempts to just
//       pass through to "write_xml" for T, but this will only work if we *definitely* do not
//       need to do any magic with pointers. We'll know if this is possible once we generate a
//       successful XML output file, so fo rnow flatten_map will work.
template <class T, class U>
std::vector< std::pair<T, U> > flatten_map(const std::map<T, U>& map)
{
	std::vector< std::pair<T, U> > res;
	for (typename std::map<T, U>::const_iterator it=map.begin(); it!=map.end(); it++) {
		res.push_back(std::make_pair(it->first, it->second));
	}
	return res;
}

//Generic stuff.

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
void XmlWriter::prop(const std::string& key, const size_t& val)
{
	write_simple_prop(key, val);
}

template <>
void XmlWriter::prop(const std::string& key, const bool& val)
{
	write_simple_prop(key, val);
}

}}
