/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * \file XmlWriter.hpp
 *
 * Provides functionality for writing XML files in as simple a manner as possible.
 */

#include <vector>
#include <string>

namespace sim_mob {
namespace xml {

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
		outFile(&outFile), tabCount(0), sealedAttr(false)
	{
		header();
	}

private:
	///More detailed constructor, for use internally. Does NOT write a header.
	/*XmlWriter(std::ostream& outFile, int tabCount, bool sealedAttr, const std::string& currProp) :
		outFile(&outFile), tabCount(tabCount), sealedAttr(sealedAttr), currProp(currProp)
	{}*/

public:

	///Write an attribute on the current property.
	void attr(const std::string& key, const std::string& val);

	///Write a property (key-value pair).
	template <class T>
	void prop(const std::string& key, const T& val);

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

	//Seal attributes; this appends a ">" to the current property and sets the sealed flag.
	//Has no effect on the very first element (due to the <?xml... closing itself).
	void seal_attrs();

	//Write a simple property (e.g., a primitive). Relies on operator<< for printing.
	template <class T>
	void write_simple_prop(const std::string& key, const T& val);

private:
	std::ostream* const outFile;
	int tabCount;          //For indentation
	std::vector<std::string> propStack;  //The property that we are currently writing is at the back of the vector.
	bool sealedAttr;       //Are we done with writing attributes?
};

}}  //End namespace sim_mob::xml




///////////////////////////////////////////////////////////////////
// Template implementation
///////////////////////////////////////////////////////////////////


namespace { //TODO: Move this into its own implementation file, later.
const int TabSize = 4;
} //End un-named namespace

void sim_mob::xml::XmlWriter::attr(const std::string& key, const std::string& val)
{
	if (sealedAttr) { throw std::runtime_error("Can't write attribute; a property has already been written"); }
	(*outFile) <<" " <<key <<"=\"" <<val <<"\"";
}


void sim_mob::xml::XmlWriter::header()
{
	(*outFile) <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" <<std::endl;
	(*outFile) <<std::endl;
}

void sim_mob::xml::XmlWriter::seal_attrs() {
	if (!sealedAttr) {
		if (tabCount>0) {
			(*outFile) <<">" <<std::endl;
		}
		sealedAttr = true;
	}
}

template <class T>
void sim_mob::xml::XmlWriter::write_simple_prop(const std::string& key, const T& val)
{
	seal_attrs();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<">" <<val <<"</" <<key <<">" <<std::endl;
}


//This function does all the heavy lifting; dispatching as appropriate.
template <class T>
void sim_mob::xml::XmlWriter::prop(const std::string& key, const T& val)
{
	//Recurse
	prop_begin(key);
	write_xml(*this, val);
	prop_end();
}


void sim_mob::xml::XmlWriter::prop_begin(const std::string& key)
{
	seal_attrs();
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

namespace {
//Helper: Attempt to convert a noun to its singular form.
//TODO: This is quite heuristical; can we rely on a shared library?
std::string SingularNoun(const std::string& noun) {
	//Too small to matter?
	if (noun.size()<3) { return noun; }

	//Already singular?
	if (noun[noun.size()-1]!='s') { return noun; }

	//"es" plural noun?
	if (noun[noun.size()-2]=='e') {
		return noun.substr(0, noun.size()-2);
	} else {
		return noun.substr(0, noun.size()-1);
	}
}

///Write a generic list/map/whatever via iterators
template <class IterType>
void write_value_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end)
{
	std::string singular = SingularNoun(write.curr_prop());
	for (; begin!=end; begin++) {
		write.prop(singular, *begin);
	}
}
template <class IterType>
void write_pointer_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end)
{
	std::string singular = SingularNoun(write.curr_prop());
	for (; begin!=end; begin++) {
		write.prop(singular, **begin);
	}
}
} //End anon namespace

namespace sim_mob { //Function specializations require an explicit namespace wrapping.
namespace xml {

///Attempts to write a vector of "items" as "item", "item", ... etc.
template <class T>
void write_xml(XmlWriter& write, const std::vector<T>& vec)
{
	write_value_array(write, vec.begin(), vec.end());
}
template <class T>
void write_xml(XmlWriter& write, const std::vector<T*>& vec)
{
	write_pointer_array(write, vec.begin(), vec.end());
}

///Attempts to write a set of "items" as "item", "item", ... etc.
template <class T>
void write_xml(XmlWriter& write, const std::set<T>& vec)
{
	write_value_array(write, vec.begin(), vec.end());
}
template <class T>
void write_xml(XmlWriter& write, const std::set<T*>& vec)
{
	write_pointer_array(write, vec.begin(), vec.end());
}

template <>
void XmlWriter::prop(const std::string& key, const std::string& val)
{
	write_simple_prop(key, val);
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
