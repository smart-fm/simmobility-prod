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
 */

//class namer_iterator;
//class XmlWriter;


//"Result" of matches. Left/right indicate the same thing they did in the previous code; both are optional.
struct res_set {
	std::string left;
	std::string right;
};


namespace {
//Count occurrances of "letter" in "src"
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
class expander : private prop_parser {
public:
	explicit expander(const std::string& nameStr="") : prop_parser(nameStr) {}

	expander leftChild() const {
		return left.leaf ? expander() : expander(left.item);
	}
	expander rightChild() const {
		return right.leaf ? expadner() : expander(right.item);
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
	void prop(const std::string& key, const T& val, namer name, expander expand=expander(""));

	//Expander-only version.
	template <class T>
	void prop(const std::string& key, const T& val, expander expand) { prop(key, val, namer(""), expand); }

	///Write a property as using identifiers instead of values. This requires
	/// the required base type to have a corresponding get_id() override.
	template <class T>
	void ident(const std::string& key, const T& val);

	///Write a list of identifiers
	template <class T>
	void ident_list(const std::string& plural, const std::string& singular, const T& val);

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

}}  //End namespace sim_mob::xml


//Helper namespace for writing lists.
namespace {
///Write a generic list/map/whatever via iterators
template <class IterType>
void write_value_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end, sim_mob::xml::namer name, sim_mob::xml::expander expand)
{
	//name.advance();
	for (; begin!=end; begin++) {
		write.prop(name.leftStr(), *begin, name, expand);
	}
	//name.unadvance();
}
template <class IterType>
void write_pointer_array(sim_mob::xml::XmlWriter& write, IterType begin, IterType end, sim_mob::xml::namer name, sim_mob::xml::expander expand)
{
	//TODO: I'm missing something here:
	//  1) We need to pass expand.leftStr(), so that we get "value" types instead of IDs (OR we can go back
	//     to the old "ident()" function (no).
	//  2) Actually, we might want to add a default function argument to prop; e.g., a default "expand()"
	//     option, which is just empty. This actually makes the most sense.
	//Then, we'd do something like this:
	//     write.prop(name.leftStr(), **begin, name.rightChild(), expand.rightChild(), expand.leftStr());
	//It's kind of messy, but it should work.
	//We also need to be *very* careful to dispatch back UP to prop() without the name/expand args
	//   if both name and expand are empty(). This should (in theory) allow our default arguments
	//   for lists, maps, and pointers to work fine.
	//The only problem would be something like "<item, <first, second>>" with "" as the expander.
	//   In this case, we need can add a default value-type argument. But what about:
	//   "" for the namer and "<value, <id, id>>" for the expander? We want to turn "" into "<item, <first,second>>"
	//   for the namer.... so perhaps we'll need prop(name,expand), which checks (internally) and dispatches to either:
	//   write_xml()
	//   write_xml(namer)
	//   write_xml(expander)
	//   write_xml(namer, expander)
	//This means we'd need all four functions for each type, but let's consider:
	//   1) Basic types could have a catch-all template which passes up to write_xml(), unless the args
	//      are non-empty (we already do this for the 4th function).
	//   2) vector<>,map<> and other container types will want to control 1,2, and 3, dispatching to
	//      4, regardless (with defaults). It's a tiny extra inconvenience, but they'll want the control,
	//      so it's not a burden (and if they forget, (1) will catch the error at runtime.)
	//   3) This strategy basically allows us to pass empty namers/expanders into prop() on a whim.
	//      write_xml() requires strictly the right namers/expanders, but that is only called internally.
	//      So, calling .prop("", "<value>") will work (even though the namer is empty).
	//Ok, this will probably work.

	//name.advance();
	for (; begin!=end; begin++) {
		write.prop(name.leftStr(), **begin, name.rightChild(), expand.rightChild());
	}
	//name.unadvance();
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
void sim_mob::xml::XmlWriter::prop(const std::string& key, const T& val, namer name, expander expand)
{
	//Begin.
	//TODO: If Exp_ID, then a full "prop_begin()" and "prop_end()" are wasteful
	//      (since we have no reason to save state information). We can fix this by
	//      delegating the entire function to prop() (listed directly above) if Exp_Value,
	//      and putting ident()'s code in the "else" block, but right now it doesn't matter
	//      as long as it works.
	prop_begin(key);

	//Advance the current namer one level deeper.
	name.parse();
	expand.parse();

	//Recurse
	bool ignoreTabs = false;
	if (expand.currLeftIsValue()) {
		write_xml(*this, val, name, expand); //Need to pass along name; we might have a nested structure here.
	} else { //if (name.curr().exp()==expand::ID) {
		seal_attrs(false); //Adds the ">"
		(*outFile) <<get_id(val);
		ignoreTabs = true;
	} //else { throw std::runtime_error("Unknown expand policy."); }

	//End
	prop_end(ignoreTabs);
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
/*template <class T, class U>
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
}*/

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
// Add any basic types here (you can even add tpyes such as Point2D which
// have an output operator if you prefer).
////////////////////////////////////////////////////////////////////



namespace sim_mob { //Function specializations require an explicit namespace wrapping.
namespace xml {


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//Test function. Functions of this kind will replace the write_xml_list functions listed earlier.
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T*>& vec, namer name, expander expand)
{
	write_pointer_array(write, vec.begin(), vec.end(), name, expand);
}
//Two-parameter functions (for containers) pass *up*
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::set<T*>& vec)
{
	write_pointer_array(write, vec.begin(), vec.end(), namer("<item>"), expander("<value>"));
}

//Test function. Functions of this kind will replace the write_xml_list functions listed earlier.
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T*>& vec, namer name, expander expand)
{
	write_pointer_array(write, vec.begin(), vec.end(), name, expand);
}
//Two-parameter functions (for containers) pass *up*
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T*>& vec)
{
	write_pointer_array(write, vec.begin(), vec.end(), namer("<item>"), expander("<value>"));
}


//TEMP: Write value array. (Can we remove the need for value arrays later?)
//TODO: These two are one level out of sync.
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec, namer name, expander expand)
{
	write_value_array(write, vec.begin(), vec.end(), name, expand);
}
template <class T>
void write_xml(sim_mob::xml::XmlWriter& write, const std::vector<T>& vec)
{
	write_value_array(write, vec.begin(), vec.end(), namer("<item>"), expander("<value>"));
}



//Test function for pairs
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<const T*, const U*>& pr, namer name, expander expand)
{
	//Propagate; use the left/right children for this.
	//TODO: How to handle left/right?
	write.prop(name.left(), *pr.first, name, expand);
	write.prop(name.right(), *pr.second, name, expand);
}
//Delegate up
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<const T*, const U*>& pr)
{
	write_xml(write, pr, namer("<first,second>"), expander("<value,value>"));
}


//Pairs: Delegate const variability up
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<const T*, U*>& pr, namer name, expander expand)
{
	write_xml(write, std::pair<const T*, const U*>(pr), name, expand);
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T*, const U*>& pr, namer name, expander expand)
{
	write_xml(write, std::pair<const T*, const U*>(pr), name, expand);
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<const T*, U*>& pr)
{
	write_xml(write, std::pair<const T*, const U*>(pr));
}
template <class T, class U>
void write_xml(sim_mob::xml::XmlWriter& write, const std::pair<T*, const U*>& pr)
{
	write_xml(write, std::pair<const T*, const U*>(pr));
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////



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
void XmlWriter::prop(const std::string& key, const bool& val)
{
	write_simple_prop(key, val);
}

}}
