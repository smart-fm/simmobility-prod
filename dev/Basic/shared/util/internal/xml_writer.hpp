#ifndef INCLUDE_UTIL_XML_WRITER_HPP
 #error Do not include util/internal/xml_writer.hpp directly; instead, include util/XmlWriter.hpp
#else //INCLUDE_UTIL_XML_WRITER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <boost/noncopyable.hpp>

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
 * If you want your class to be xml-serializable, you should create two public functions called "write_xml()"
 * and "get_id()" with the required functionality.
 *
 * See: geospatial/Serialize.hpp for some examples.
 */
class XmlWriter : private boost::noncopyable {
public:
	///Create a new XmlWriter which will send output to "outFile". Writes the header.
	XmlWriter(std::ostream& outFile);

public:
	///Helper: escape illegal XML characters into a new, clean string.
	static std::string EscapeXML(const std::string& src);

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
	//How many spaces is a tab?
	const static int TabSize = 4;

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


///////////////////////////////////////////////////////////////////
// Template implementation
// Note that these must go into the header file, due to the nature of templates.
///////////////////////////////////////////////////////////////////


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



#endif //INCLUDE_UTIL_XML_WRITER_HPP
