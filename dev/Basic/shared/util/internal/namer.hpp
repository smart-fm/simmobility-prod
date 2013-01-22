#ifndef INCLUDE_UTIL_XML_WRITER_HPP
 #error Do not include util/internal/namer.hpp directly; instead, include util/XmlWriter.hpp
#else //INCLUDE_UTIL_XML_WRITER_HPP

#include <string>

namespace sim_mob {
namespace xml {


///The base class of namer and expander, which provides shared functionality for both.
///There is never any need to instantiate this class; use the "namer" and "expander" subclasses, below.
class prop_parser  {
protected:
	///Count the number of occurrences of "letter" in a given input string.
	static int LetterCount(const std::string& src, char letter);

	///Is this string complex? (Does it have <,> anywhere?)
	static bool IsComplex(const std::string& src);

	///Helper: Scan and break into left/right segments. Results are returned by reference into left and right.
	void ScanPair(const std::string& nameStr, std::string& left, std::string&right);

	//Only allow construction via subclasses.
	explicit prop_parser(const std::string& str);

	//Container for results
	struct res_item {
		res_item() : leaf(true) {}
		std::string item;  //E.g., "item", or "<first,second>"
		bool leaf;         //True if "item" or "", not "<first,second>"
	};

public:
	std::string leftStr() const;
	std::string rightStr() const;
	bool leftIsLeaf() const;
	bool rightIsLeaf() const;

	///An object is "empty" if it can be safely discarded; e.g., it contains no information (string) whatsoever.
	bool isEmpty() const;

private:
	void sanity_check(const std::string& str);

	///Parse the current string, removing the outer layer of brackets and retrieving the "left" and "right" elements (including remaining components).
	void parse(const std::string& str);

protected:
	//Our current results set.
	res_item left;
	res_item right;
};



/**
 * Class used for providing a customized name for a given STL container.
 *
 * This class serves to address a particular problem: we provide helper write_xml() wrappers
 * for vectors, sets, etc., but what happens if the user has two different sets, and wants
 * to name the elements in each set differently? What if we also want to provide a sensible default,
 * such as "item" for vectors?
 *
 * To accomplish this, you can pass a "namer" object into the "prop()" function of XmlWriter.
 * These objects are constructed using a string syntax similar in style to STL containers.
 * Some example syntax is listed below. Assume we have an XmlWriter named "write" available for all examples.
 *
 *   \code
 *   vector<string> modes = {"Car", "Taxi", "Bicycle"};
 *   namer name("<role>");
 *   write.prop("Roles", modes, name);
 *
 *   //Sample output:
 *   <Roles>
 *     <role>Car</role>
 *     <role>Taxi</role>
 *     <role>Bicycle</role>
 *   </Roles>
 *   \endcode
 *
 *   \code
 *   pair<string,string> item = {"January", "23"};
 *   namer name("<month,day>");
 *   write.prop("Today", item, name);
 *
 *   //Sample output:
 *   <Today>
 *     <month>January</month>
 *     <day>23</day>
 *   </Today>
 *   \endcode
 *
 *
 *   \code
 *   map<int,bool> primes = {{2,true},{4,false}};
 *   namer name(""<candidate,<number,is_prime>>"");
 *   write.prop("Primes", primes, name);
 *
 *   //Sample output:
 *   <Primes>
 *     <candidate>
 *       <number>2</number>
 *       <is_prime>true</is_prime>
 *     </candidate>
 *     <candidate>
 *       <number>4</number>
 *       <is_prime>false</is_prime>
 *     </candidate>
 *   </Primes>
 *   \endcode
 *
 *   You can even compose these items to some extent:
 *
 *   \code
 *   vector< pair<string,string> > holidays = {{"December","25"},{"August","9"}};
 *   namer name("<holiday,<month,day>>");
 *   write.prop("Holidays", holidays, name);
 *
 *   //Sample output:
 *   <Holidays>
 *     <holiday>
 *       <month>December</month>
 *       <day>25</day>
 *     </holiday>
 *     <holiday>
 *       <month>August</month>
 *       <day>9</day>
 *     </holiday>
 *   </Holidays>
 *   \endcode
 *
 * You can also use an asterisk or empty string to represent the "default" value; e.g., namer("<*>") will name
 * all elements in a vector "item", while namer("") will use "first" and "second" for elements in a pair.
 *
 * Finally, note that you can use a namer and an expander (below) by listing the namer first.
 * E.g., prop("something", item, namer("<item>"), expander(""))
 *
 * \note
 * The current namer syntax is too weak to represent all types. What we *should* have done is to
 * represent the namer as the actual XML subtree; e.g., "<items><item/></items>", or something similar.
 * However, we can't do this immediately, because of the way our namer function is called. In reality, we
 * should have something like prop(T& value, namer name"", expander expand=""), and use default propagation
 * for *everything*, not just for container types.
 *
 * \todo
 * It's very easy to descent recursively into a value-type expansion loop. There are two solutions;
 *    1) Make the defaults for vectors/sets be value types.
 *    2) Have the pointer-dereference template functions track each pointer used and throw an
 *       exception if the same pointer is being used as a value type more than once.
 *  Two is preferable; I'll add it once the API is stable.
 */

class namer : public prop_parser {
public:
	explicit namer(const std::string& nameStr="") : prop_parser(nameStr) {}

	namer leftChild() const;
	namer rightChild() const;
};



/**
 * Class used for providing a customized expanding scheme for a given STL container.
 *
 * See the documentation for "namer"  ---"expander" is just like it, except it focuses on item
 * expansion. You can either expand by "value" or by "id", where the latter provides a full expansion and
 * the latter provides just the id of the item (assumed to be defined elsewhere in the XML).
 *
 * \note
 * WARNING: The default expansion of all types is "value", which makes it extremely easy to get into an
 * infinite loop (if there are circular includes of items via pointers). We will catch these errors later;
 * for now, just kill your program if it takes too long to output.
 *
 * Some examples:
 *
 *   \code
 *   class Item { int id; string name; string value; }  //Assume write_xml() is defined somewhere for this class.
 *   vector<Item> items = {{100,"Trip","bus"},{200,"Activity","wait"}};
 *   expander expand("<value>");
 *   write.prop("TripChain", items, expand);
 *
 *   //Sample output:
 *   <TripChain>
 *     <item>
 *       <id>100</id>
 *       <name>Trip</name>
 *       <value>bus</value>
 *     </item>
 *     <item>
 *       <id>200</id>
 *       <name>Activity</name>
 *       <value>wait</value>
 *     </item>
 *   </TripChain>
 *   \endcode
 *
 *   \code
 *   class Item { int id; string name; string value; }  //Assume write_xml() is defined somewhere for this class.
 *   vector<Item> items = {{100,"Trip","bus"},{200,"Activity","wait"}};
 *   expander expand("<id>");
 *   write.prop("TripChain", items, expand);
 *
 *   //Sample output:
 *   <TripChain>
 *     <item>100</item>
 *     <item>200</item>
 *   </TripChain>
 *   \endcode
 */
class expander : public prop_parser {
public:
	explicit expander(const std::string& nameStr="") : prop_parser(nameStr) {}

	expander leftChild() const;
	expander rightChild() const;

	bool leftIsValue() const;
	bool rightIsValue() const;

private:
	static bool IsValue(const std::string& candidate);
};

}}

#endif //INCLUDE_UTIL_XML_WRITER_HPP
