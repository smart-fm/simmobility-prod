//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

//NOTE: This definition is necessary to use internal items within this folder. Do *not* do this in your
//      own code; instead, simply include util/XmlWriter.hpp, which pulls all of the related information.
#define INCLUDE_UTIL_XML_WRITER_HPP
#include "namer.hpp"
#include "xml_writer.hpp"

#include <sstream>
#include <stdexcept>

using std::string;


sim_mob::xml::XmlWriter::XmlWriter(std::ostream& outFile)
	: outFile(&outFile), tabCount(0), newlines(0), attPrefix(" "), sealedAttr(false)
{
	header();
}


string sim_mob::xml::XmlWriter::EscapeXML(const string& src)
{
	//Shortcut: no characters to replace?
	if (src.find_first_of("\"'<>&")==string::npos) { return src; }

	std::stringstream res;
	for (string::const_iterator it=src.begin(); it!=src.end(); it++) {
		     if (*it=='"')  { res <<"&quot;"; }
		else if (*it=='\'') { res <<"&apos;"; }
		else if (*it=='<')  { res <<"&lt;"; }
		else if (*it=='>')  { res <<"&gt;"; }
		else if (*it=='&')  { res <<"&amp;"; }
		else                { res <<*it; }
	}
	return res.str();
}


void sim_mob::xml::XmlWriter::attr(const string& key, const string& val)
{
	if (sealedAttr) { throw std::runtime_error("Can't write attribute; a property has already been written"); }
	(*outFile) <<attPrefix <<key <<"=\"" <<EscapeXML(val) <<"\"";
}

void sim_mob::xml::XmlWriter::endl()
{
	newlines++;
}

void sim_mob::xml::XmlWriter::attr_prefix(const string& pre)
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
		(*outFile) <<string(newlines, '\n');
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


void sim_mob::xml::XmlWriter::prop_begin(const string& key)
{
	seal_attrs();
	write_newlines();
	(*outFile) <<string(tabCount*TabSize, ' ') <<"<" <<key;

	//Next indentation/sealed level.
	tabCount++;
	sealedAttr = false;
	propStack.push_back(key);
}

void sim_mob::xml::XmlWriter::prop_end(bool ignoreTabs)
{
	//Restore indentation
	string key = propStack.back();
	propStack.pop_back();
	tabCount--;

	//Close tab.
	if (sealedAttr) { //Did we write at least one property?
		if (!ignoreTabs) {
			(*outFile) <<string(tabCount*TabSize, ' ');
		}
		(*outFile) <<"</" <<key <<">" <<std::endl;
	} else {
		(*outFile) <<"/>" <<std::endl;
	}

	//Closing will "seal" this property, even if we haven't called seal_attrs();
	sealedAttr = true;
}

