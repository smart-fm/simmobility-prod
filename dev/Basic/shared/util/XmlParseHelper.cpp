//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "XmlParseHelper.hpp"

#include <string>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/util/XMLString.hpp>
#include "LangHelpers.hpp"
#include "logging/Log.hpp"

using namespace sim_mob;
using namespace xercesc;
namespace sim_mob
{

//Helper: make sure we actually have an element
DOMElement* NodeToElement(DOMNode* node)
{
	DOMElement* res = dynamic_cast<DOMElement*>(node);
	if (!res)
	{
		throw std::runtime_error("DOMNode is expected to be a DOMElement.");
	}
	return res;
}

//Helper: retrieve child elements without leaking memory
std::vector<DOMElement*> GetElementsByName(DOMElement* node, const std::string& key, bool required)
{
	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNodeList* res = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);

	if (res->getLength() == 0 && required)
	{
		throw std::runtime_error("Elements expected, but none returned.");
	}

	std::vector<DOMElement*> resV;
	for (XMLSize_t i = 0; i < res->getLength(); i++)
	{
		resV.push_back(NodeToElement(res->item(i)));
	}

	return resV;
}

//Helper: retrieve a single element; optionally required.
DOMElement* GetSingleElementByName(DOMElement* node, const std::string& key, bool required)
{
	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNodeList* res = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);

	//Check.
	if (res->getLength() > 1)
	{
		throw std::runtime_error("Error: single element expected, but returned more than 1.");
	}
	else if (res->getLength() == 1)
	{
		return NodeToElement(res->item(0));
	}
	else if (required)
	{
		throw std::runtime_error("Error: single element expected, but returned zero.");
	}

	return nullptr;
}

//Helper: retrieve an attribute
DOMAttr* GetNamedAttribute(DOMElement* node, const std::string& key, bool required)
{
	if (!node)
	{
		return nullptr;
	}

	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNode* res = node->getAttributes()->getNamedItem(keyX);
	XMLString::release(&keyX);

	//Check.
	if (!res)
	{
		if (required)
		{
			throw std::runtime_error("Error: attribute expected, but none found.");
		}
		else
		{
			return nullptr;
		}
	}

	DOMAttr* resAttr = dynamic_cast<DOMAttr*>(res);
	if (!resAttr)
	{
		throw std::runtime_error("Error: attribute expected, but couldn't be cast.");
	}

	return resAttr;
}

//Helper
const XMLCh* GetAttributeValue(const DOMAttr* attr)
{
	if (!attr)
	{
		return nullptr;
	}
	return attr->getNodeValue();
}

//Helper: Combined
const XMLCh* GetNamedAttributeValue(DOMElement* node, const std::string& key, bool required)
{
	return GetAttributeValue(GetNamedAttribute(node, key, required));
}

//Helper: boolean stuff
bool ParseBoolean(const XMLCh* srcX, bool* defValue)
{
	if (srcX)
	{
		std::string src = TranscodeString(srcX);
		std::transform(src.begin(), src.end(), src.begin(), ::tolower);
		if (src == "true" || src == "yes")
		{
			return true;
		}
		else if (src == "false" || src == "no")
		{
			return false;
		}
		throw std::runtime_error("Expected boolean value.");
	}

	//Wasn't found.
	if (!defValue)
	{
		throw std::runtime_error("Mandatory boolean variable; no default available.");
	}
	return *defValue;
}

int ParseInteger(const XMLCh* srcX, int* defValue)
{
	if (srcX)
	{
		int value = 0;
		try
		{
			std::string src = TranscodeString(srcX);
			value = boost::lexical_cast<int>(src);
		} catch (boost::bad_lexical_cast const&)
		{
			throw std::runtime_error("Bad formatted source string for Integer parsing.");
		}
		return value;
	}

	//Wasn't found.
	if (!defValue)
	{
		throw std::runtime_error("Mandatory integer variable; no default available.");
	}
	return *defValue;
}

float ParseFloat(const XMLCh* srcX, float* defValue)
{
	if (srcX)
	{
		std::string src = TranscodeString(srcX);
		return boost::lexical_cast<float>(src);
	}

	//Wasn't found.
	if (!defValue)
	{
		throw std::runtime_error("Mandatory float variable; no default available.");
	}
	return *defValue;
}

unsigned int ParseUnsignedInt(const XMLCh* srcX, unsigned int* defValue)
{
	if (srcX)
	{
		unsigned int value = 0;
		try
		{
			std::string src = TranscodeString(srcX);
			value = boost::lexical_cast<unsigned int>(src);
		} catch (boost::bad_lexical_cast const&)
		{
			throw std::runtime_error("Bad formatted source string for unsigned integer parsing.");
		}
		return value;
	}

	//Wasn't found.
	if (!defValue)
	{
		throw std::runtime_error("Mandatory unsigned integer variable; no default available.");
	}
	return *defValue;
}

std::string ParseString(const XMLCh* srcX, std::string* defValue)
{
	if (srcX)
	{
		return TranscodeString(srcX);
	}

	//Wasn't found.
	if (!defValue)
	{
		throw std::runtime_error("Mandatory string variable; no default available.");
	}

	return *defValue;
}

//Helper: amount+value for time-granularities.
unsigned int GetValueInMs(double amount, std::string units, unsigned int* defValue)
{
	//Handle plural
	if (units == "second")
	{
		units = "seconds";
	}
	if (units == "minute")
	{
		units = "minutes";
	}

	//Detect errors
	if (units.empty() || (units != "minutes" && units != "seconds" && units != "ms"))
	{
		if (defValue)
		{
			return *defValue;
		}
		else
		{
			throw std::runtime_error("Invalid units in parsing time granularity.");
		}
	}

	//Reduce to ms
	if (units == "minutes")
	{
		amount *= 60 * 1000;
	}
	if (units == "seconds")
	{
		amount *= 1000;
	}

	//Check for overflow:
	unsigned int res = static_cast<unsigned int>(amount);
	if (static_cast<double>(res) != amount)
	{
		Warn() << "NOTE: Rounding value in ms from " << amount << " to " << res << "\n";
	}

	return res;
}

//Helper: Time units such as "10", "seconds"
unsigned int ParseTimegranAsMs(const XMLCh* amountX, const XMLCh* unitsX, unsigned int* defValue)
{
	double amount = boost::lexical_cast<double>(TranscodeString(amountX));
	std::string units = TranscodeString(unitsX);

	return GetValueInMs(amount, units, defValue);
}

DailyTime ParseDailyTime(const XMLCh* srcX, DailyTime* defValue)
{
	if (srcX)
	{
		return DailyTime(TranscodeString(srcX));
	}

	//Wasn't found.
	if (!defValue)
	{
		throw std::runtime_error("Mandatory integer variable; no default available.");
	}
	return *defValue;
}

std::string ParseNonemptyString(const XMLCh* srcX, std::string* defValue)
{
	std::string res = ParseString(srcX, defValue);
	if (!res.empty())
	{
		return res;
	}

	//Wasn't found.
	if (!defValue)
	{
		throw std::runtime_error("Mandatory string variable; no default available. (Empty strings NOT allowed.)");
	}
	return *defValue;
}

//How to do defaults
bool ParseBoolean(const XMLCh* src, bool defValue)
{
	return ParseBoolean(src, &defValue);
}
bool ParseBoolean(const XMLCh* src)
{ //No default
	return ParseBoolean(src, nullptr);
}
int ParseInteger(const XMLCh* src, int defValue)
{
	return ParseInteger(src, &defValue);
}
int ParseInteger(const XMLCh* src)
{ //No default
	return ParseInteger(src, nullptr);
}
unsigned int ParseUnsignedInt(const XMLCh* src, unsigned int defValue)
{
	return ParseUnsignedInt(src, &defValue);
}
unsigned int ParseUnsignedInt(const XMLCh* src)
{ //No default
	return ParseUnsignedInt(src, nullptr);
}
float ParseFloat(const XMLCh* src)
{
	return ParseFloat(src, nullptr);
}
float ParseFloat(const XMLCh* src, float defValue)
{
	return ParseFloat(src, &defValue);
}
DailyTime ParseDailyTime(const XMLCh* src, DailyTime defValue)
{
	return ParseDailyTime(src, &defValue);
}
DailyTime ParseDailyTime(const XMLCh* src)
{ //No default
	return ParseDailyTime(src, nullptr);
}

std::string ParseString(const XMLCh* src, std::string defValue)
{
	return ParseString(src, &defValue);
}
std::string ParseString(const XMLCh* src)
{ //No default
	return ParseString(src, nullptr);
}
std::string ParseNonemptyString(const XMLCh* src, std::string defValue)
{
	return ParseNonemptyString(src, &defValue);
}
std::string ParseNonemptyString(const XMLCh* src)
{ //No default
	return ParseNonemptyString(src, nullptr);
}
unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units, unsigned int defValue)
{
	return ParseTimegranAsMs(amount, units, &defValue);
}
unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units)
{ //No default
	return ParseTimegranAsMs(amount, units, nullptr);
}

//TODO: Now we are starting to overlap...
int ProcessValueInteger2(xercesc::DOMElement* node, int defVal)
{
	return ParseInteger(GetNamedAttributeValue(node, "value"), defVal);
}

//TODO: Same issue; needs to be easier access to these things.
std::string ProcessValueString(xercesc::DOMElement* node)
{
	return ParseString(GetNamedAttributeValue(node, "value"));
}
}





