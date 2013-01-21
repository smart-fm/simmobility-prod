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
#include <stdexcept>
#include <map>

#include "util/LangHelpers.hpp"


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
 *
 * \note
 * I'm in the process of changing a "naming" to a "namer", which has three benefits:
 *    1) It can be nested (so pairs, vectors of maps, etc. are ok).
 *    2) It can specify "value" or "id" expansion.
 *    3) One can construct them with a series of static functions that can be nested.
 * I almost called it "expander", but then I figured that no-one would ever use it.
 * Better to call it "namer", since that's what most people will use it for.
 *
 * \todo
 * It's very easy to descent recursively into a value-type expansion loop. There are two solutions;
 *    1) Make the defaults for vectors/sets be value types.
 *    2) Have the pointer-dereference template functions track each pointer used and throw an
 *       exception if the same pointer is being used as a value type more than once.
 *  Two is preferable.
 */



//"Result" of matches. Left/right indicate the same thing they did in the previous code; both are optional.
struct res_set {
	std::string left;
	std::string right;
};


namespace {
//Count occurrences of "letter" in "src"
int letter_count(const std::string& src, char letter) {
	int res=0;
	for (std::string::const_iterator it=src.begin(); it!=src.end(); it++) {
		if (*it == letter) { res++; }
	}
	return res;
}

//Is this string complex? (Does it have <,> anywhere?)
bool is_complex(const std::string& src) {
	return (src.find('<')<src.size()) || (src.find(',')<src.size()) || (src.find('>')<src.size());
}

//Helper: Scan and break into left/right segments. Results are returned by reference into left and right.
void scan_pair(const std::string& nameStr, std::string& left, std::string&right) {
	std::stringstream newLeft;
	int depth=0; // nested in brackets?
	for (std::string::const_iterator it=nameStr.begin(); it!=nameStr.end(); it++) {
		//Only three characters matter here.
		if (*it==',') {
			if (depth==0) {
				//We found our comma; left will be set later, so set right to the remainder.
				right = std::string(it+1, nameStr.end());
				break;
			}
		} else if (*it=='<') {
			depth++;
		} else if (*it=='>') {
			depth--;
		}

		//Either way, append it to left.
		newLeft <<*it;
	}

	//Save left; right's already been saved.
	left = newLeft.str();
}
} //End anon namespace



///The base class of namer and expander, which provides shared functionality for both.
class prop_parser  {
protected:
	//Only allow construction via subclasses.
	explicit prop_parser(const std::string& str) {
		//Perform some sanity checks, then parse.
		sanity_check(str);
		parse(str);
	}

	//Container for results
	struct res_item {
		res_item() : leaf(true) {}
		std::string item;  //E.g., "item", or "<first,second>"
		bool leaf;         //True if "item" or "", not "<first,second>"
	};


public:
	std::string leftStr() const {
		return left.leaf ? left.item : "";
	}
	std::string rightStr() const {
		return right.leaf ? right.item : "";
	}
	bool leftIsLeaf() const {
		return left.leaf;
	}
	bool rightIsLeaf() const {
		return right.leaf;
	}

	//An object is "empty" if it can be safely discarded; e.g., it contains no information (string) whatsoever.
	bool isEmpty() const {
		return left.item.empty() && right.item.empty();
	}

private:
	void sanity_check(const std::string& str) {
		//We can handle spaces later; it just messes up our definition of "empty". For now it's not worth it.
		if (str.find(' ')<str.size()) { throw std::runtime_error("Bad namer/expander string: no spaces!"); }

		//Very simple sanity check: do our left and right brackets match up (and outnumber commas)?
		int countL = letter_count(str, '<');
		int countR = letter_count(str, '>');
		int countC = letter_count(str, ',');
		if (countL==countR && countC<=countL) { return; }
		throw std::runtime_error("Bad namer/expander string: brackets and commas don't match up.");
	}

	//Parse the current string, removing the outer layer of brackets and retrieving the "left" and "right"
	//  elements (including remaining components).
	void parse(const std::string& str) {
		//Nothing to parse?
		if (str.empty()) { return; }

		//Find the outermost brackets. These are balanced (see the constructor), and should be at the start/end of the string.
		size_t cropL = str.find('<');
		size_t cropR = str.rfind('>');
		if (cropL!=0 || cropR!=str.size()-1) {
			return;  //Silently fail.
		}

		//There are three cases to deal with, all of which generalize to the first case.
		//The difficulty is that there can be nested brackets too.
		//   <left,right>
		//   <left>
		//   <> (nothing)
		scan_pair(str.substr(1,str.size()-2), left.item, right.item);

		//Replace "*" with "" (optional padding, looks better)
		if (left.item=="*") { left.item=""; }
		if (right.item=="*") { right.item=""; }

		//Set "leaf" property.
		left.leaf = !is_complex(left.item);
		right.leaf = !is_complex(right.item);
	}

protected:
	//Our current results set.
	res_item left;
	res_item right;
};



//Try again, this time simply using strings.
class namer : public prop_parser {
public:
	explicit namer(const std::string& nameStr="") : prop_parser(nameStr) {}

	namer leftChild() {
		return left.leaf ? namer() : namer(left.item);
	}
	namer rightChild() {
		return right.leaf ? namer() : namer(right.item);
	}
};
class expander : public prop_parser {
public:
	explicit expander(const std::string& nameStr="") : prop_parser(nameStr) {}

	expander leftChild() const {
		return left.leaf ? expander() : expander(left.item);
	}
	expander rightChild() const {
		return right.leaf ? expander() : expander(right.item);
	}

	bool leftIsValue() const {
		return isValue(leftStr());
	}

	bool rightIsValue() const {
		return isValue(rightStr());
	}

private:
	bool isValue(const std::string& candidate) const {
		//Default is "value", so check for "id"
		//This is the only area where we're strict.
		if (candidate=="value") {
			return true;
		} else if (candidate=="id") {
			return false;
		} else if (!candidate.empty()) {
			throw std::runtime_error("expand-type must be either \"id\" or \"value\"");
		}
		return true;
	}
};

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

	///Write a property as using identifiers instead of values. This requires
	/// the required base type to have a corresponding get_id() override.
	//template <class T>
	//void ident(const std::string& key, const T& val);

	///Write a list of identifiers
	//template <class T>
	//void ident_list(const std::string& plural, const std::string& singular, const T& val);

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



///Same, but for identifiers.
/*template <class IterType>
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
}*/

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



//Write an identifier
/*template <class T>
void sim_mob::xml::XmlWriter::ident(const std::string& key, const T& val)
{
	//Identifiers have no attributes or child properties, so they are relatively easy to serialize.
	seal_attrs();
	write_newlines();
	(*outFile) <<std::string(tabCount*TabSize, ' ') <<"<" <<key <<">";
	(*outFile) <<get_id(val) <<"</" <<key <<">" <<std::endl;
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
}*/

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
	write.prop(key, val, name, expand, writeValue);
}
} //End un-named namespace


namespace sim_mob { //Function specializations require an explicit namespace wrapping.
namespace xml {



///Flatten a map into a vector of "item" pairs.
///TODO: This is somewhat inefficient; we can probably create a "write_xml" function for
//       std::maps too. Unfortunately, we need to handle the four combinations of <T, U>,
//       <T*, U>, <T, U*>, <T*, U*>, which gets messy.
//       We can get around this by declaring a "write_xml" function for T* that attempts to just
//       pass through to "write_xml" for T, but this will only work if we *definitely* do not
//       need to do any magic with pointers. We'll know if this is possible once we generate a
//       successful XML output file, so fo rnow flatten_map will work.
/*template <class T, class U>
std::vector< std::pair<T, U> > flatten_map(const std::map<T, U>& map)
{
	std::vector< std::pair<T, U> > res;
	for (typename std::map<T, U>::const_iterator it=map.begin(); it!=map.end(); it++) {
		res.push_back(std::make_pair(it->first, it->second));
	}
	return res;
}*/

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
void XmlWriter::prop(const std::string& key, const bool& val)
{
	write_simple_prop(key, val);
}

}}
