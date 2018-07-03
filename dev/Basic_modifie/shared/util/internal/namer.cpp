//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

//NOTE: This definition is necessary to use internal items within this folder. Do *not* do this in your
//      own code; instead, simply include util/XmlWriter.hpp, which pulls all of the related information.
#define INCLUDE_UTIL_XML_WRITER_HPP
#include "namer.hpp"

#include <sstream>
#include <stdexcept>

using std::string;


int sim_mob::xml::prop_parser::LetterCount(const string& src, char letter) {
	int res=0;
	for (string::const_iterator it=src.begin(); it!=src.end(); it++) {
		if (*it == letter) { res++; }
	}
	return res;
}

bool sim_mob::xml::prop_parser::IsComplex(const string& src) {
	return (src.find('<')<src.size()) || (src.find(',')<src.size()) || (src.find('>')<src.size());
}

void sim_mob::xml::prop_parser::ScanPair(const string& nameStr, string& left, string&right) {
	std::stringstream newLeft;
	int depth=0; // nested in brackets?
	for (string::const_iterator it=nameStr.begin(); it!=nameStr.end(); it++) {
		//Only three characters matter here.
		if (*it==',') {
			if (depth==0) {
				//We found our comma; left will be set later, so set right to the remainder.
				right = string(it+1, nameStr.end());
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

sim_mob::xml::prop_parser::prop_parser(const string& str)
{
	//Perform some sanity checks, then parse.
	sanity_check(str);
	parse(str);
}


string sim_mob::xml::prop_parser::leftStr() const
{
	return left.leaf ? left.item : "";
}

string sim_mob::xml::prop_parser::rightStr() const
{
	return right.leaf ? right.item : "";
}

bool sim_mob::xml::prop_parser::leftIsLeaf() const
{
	return left.leaf;
}

bool sim_mob::xml::prop_parser::rightIsLeaf() const
{
	return right.leaf;
}

bool sim_mob::xml::prop_parser::isEmpty() const
{
	return left.item.empty() && right.item.empty();
}


void sim_mob::xml::prop_parser::sanity_check(const std::string& str)
{
	//We can handle spaces later; it just messes up our definition of "empty". For now it's not worth it.
	if (str.find(' ')<str.size()) { throw std::runtime_error("Bad namer/expander string: no spaces!"); }

	//Very simple sanity check: do our left and right brackets match up (and outnumber commas)?
	int countL = LetterCount(str, '<');
	int countR = LetterCount(str, '>');
	int countC = LetterCount(str, ',');
	if (countL==countR && countC<=countL) { return; }
	throw std::runtime_error("Bad namer/expander string: brackets and commas don't match up.");
}


void sim_mob::xml::prop_parser::parse(const string& str)
{
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
	ScanPair(str.substr(1,str.size()-2), left.item, right.item);

	//Replace "*" with "" (optional padding, looks better)
	if (left.item=="*") { left.item=""; }
	if (right.item=="*") { right.item=""; }

	//Set "leaf" property.
	left.leaf = !IsComplex(left.item);
	right.leaf = !IsComplex(right.item);
}

sim_mob::xml::namer sim_mob::xml::namer::leftChild() const
{
	return left.leaf ? namer() : namer(left.item);
}

sim_mob::xml::namer sim_mob::xml::namer::rightChild() const
{
	return right.leaf ? namer() : namer(right.item);
}

sim_mob::xml::expander sim_mob::xml::expander::leftChild() const
{
	return left.leaf ? expander() : expander(left.item);
}

sim_mob::xml::expander sim_mob::xml::expander::rightChild() const
{
	return right.leaf ? expander() : expander(right.item);
}

bool sim_mob::xml::expander::leftIsValue() const
{
	return IsValue(leftStr());
}

bool sim_mob::xml::expander::rightIsValue() const
{
	return IsValue(rightStr());
}

bool sim_mob::xml::expander::IsValue(const std::string& candidate)
{
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
