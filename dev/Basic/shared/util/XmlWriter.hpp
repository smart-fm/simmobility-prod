/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

/**
 * \file XmlWriter.hpp
 *
 * Provides functionality for writing XML files in as simple a manner as possible.
 */


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
 * TODO: Can we move that functionality into a static function of the xml_writer class?
 *
 * If you want your class to be xml-serializable, you should create a public function called "write_xml()"
 * with the required functionality. See geospatial/Serialize.hpp for some examples.
 */
class xml_writer : private boost::noncopyable {
public:
	///Create a new xml_writer which will send output to "outFile"
	xml_writer(std::ostream& outFile) :
		outFile(&outFile), tabCount(0), sealedAttr(false)
	{
		header();
	}

	///Write an attribute on the current property.
	void attr(const std::string& key, const std::string& val);

	/*void prop(const std::string& key) {
		seal_attrs();
		(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<"/>" <<std::endl;
	}*/

	///Write a property (key-value pair).
	template <class T>
	void prop(const std::string& key, const T& val);

private:
	///Helper class: scope-lock used for remembering the current state of the xml_writer.
	///Saves and restores the state using RAII
	///NOTE: This isn't working, and I don't have time to figure out if it's because of optimizations in GCC.
	/*class state_lock : private boost::noncopyable {
	public:
		state_lock(xml_writer& write) : write(write), state(std::make_pair(write.tabCount, write.sealedAttr)) {}
		~state_lock() {
			write.tabCount=state.first; write.sealedAttr=state.second;
		}
	private:
		xml_writer& write;
		std::pair<int, bool> state;
	};*/

	//Make an XML writer with a different tab count. (TODO: this is sloppy)
	/*xml_writer(std::ostream& outFile, int tabCount) :
		outFile(&outFile), tabCount(tabCount), sealedAttr(false)
	{}*/

	///Write this file's header. This will be called first, as it prints the "<?xml..." tag.
	void header();

	//Seal attributes; this appends a ">" to the current property and sets the sealed flag.
	//Has no effect on the very first element (due to the <?xml... closing itself).
	void seal_attrs();

	//Write a simple property (e.g., a primitive). Relies on operator<< for printing.
	template <class T>
	void write_simple_prop(const std::string& key, const T& val);

private:
	std::ostream* outFile;
	int tabCount;     //For indentation
	bool sealedAttr;  //Are we done with writing attributes?
	//int propsWritten; //How many properties have we written with this xml_writer?
};

}}  //End namespace sim_mob::xml




///////////////////////////////////////////////////////////////////
// Template implementation
///////////////////////////////////////////////////////////////////


namespace { //TODO: Move this into its own implementation file, later.
const int TabSize = 4;
} //End un-named namespace

void sim_mob::xml::xml_writer::attr(const std::string& key, const std::string& val)
{
	if (sealedAttr) { throw std::runtime_error("Can't write attribute; a property has already been written"); }
	(*outFile) <<" " <<key <<"=\"" <<val <<"\"";
}


void sim_mob::xml::xml_writer::header()
{
	(*outFile) <<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" <<std::endl;
	(*outFile) <<std::endl;
}

void sim_mob::xml::xml_writer::seal_attrs() {
	if (!sealedAttr) {
		if (tabCount>0) {
			(*outFile) <<">" <<std::endl;
		}
		sealedAttr = true;
	}
}

template <class T>
void sim_mob::xml::xml_writer::write_simple_prop(const std::string& key, const T& val)
{
	seal_attrs();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<">" <<val <<"</" <<key <<">" <<std::endl;
}


//This function does all the heavy lifting; dispatching as appropriate.
template <class T>
void sim_mob::xml::xml_writer::prop(const std::string& key, const T& val)
{
	seal_attrs();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key;

	//Track if we wrote at least one property.
	bool wroteProp = false;

	//Lock the state
	std::pair<int, bool> state = std::make_pair(tabCount, sealedAttr);
	tabCount++;
	sealedAttr = false;

	//Recurse
	write_xml(*this, val);
	wroteProp = sealedAttr;

	//Restore state
	tabCount = state.first;
	sealedAttr = state.second;

	//Close tab.
	if (wroteProp) {
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

}}
