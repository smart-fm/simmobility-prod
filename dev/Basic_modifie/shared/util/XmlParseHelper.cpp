//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "XmlParseHelper.hpp"

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

//Helper: turn a Xerces error message into a string.
std::string TranscodeString(const XMLCh *str)
{
	char *raw = XMLString::transcode(str);
	std::string res(raw);
	XMLString::release(&raw);
	return res;
}

//Helper: make sure we actually have an element
DOMElement* NodeToElement(DOMNode* node)
{
	DOMElement* res = dynamic_cast<DOMElement*>(node);

	if (node && !res)
	{
		std::stringstream msg;
		msg << "Failed to cast <" << TranscodeString(node->getLocalName())
		    << "> from DOMNode to DOMElement";
		throw std::runtime_error(msg.str());
	}
	else if(!node)
	{
		std::stringstream msg;
		msg << "Null pointer encountered";
		throw std::runtime_error(msg.str());
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
		std::stringstream msg;
		msg << "Mandatory configuration fields <" << key << "> not found.";
		throw std::runtime_error(msg.str());
	}

	std::vector<DOMElement*> resV;
	for (XMLSize_t i = 0; i < res->getLength(); i++)
	{
		try
		{
			resV.push_back(NodeToElement(res->item(i)));
		}
		catch (std::runtime_error &ex)
		{
			std::stringstream msg;
			msg << "Invalid <" << TranscodeString(node->getLocalName()) << "> element at position "
			    << (i+1) << ". " << ex.what();
			throw std::runtime_error(msg.str());
		}
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
		std::stringstream msg;
		msg << "Multiple occurrence of configuration field <" << key << "> found. Expected: single occurrence";
		throw std::runtime_error(msg.str());
	}
	else if (res->getLength() == 1)
	{
		try
		{
			return NodeToElement(res->item(0));
		}
		catch (std::runtime_error &ex)
		{
			std::stringstream msg;
			msg << "Invalid <" << TranscodeString(node->getLocalName()) << "> element at position 1. "
			    << ex.what();
			throw std::runtime_error(msg.str());
		}
	}
	else if (required)
	{
		std::stringstream msg;
		msg << "Mandatory configuration field <" << key << "> not found.";
		throw std::runtime_error(msg.str());
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
			std::stringstream msg;
			msg << "Mandatory attribute \"" << key << "\" in <" << TranscodeString(node->getLocalName())
			    << "> not found.";
			throw std::runtime_error(msg.str());
		}
		else
		{
			return nullptr;
		}
	}

	DOMAttr* resAttr = dynamic_cast<DOMAttr*>(res);

	if (!resAttr)
	{
		std::stringstream msg;
		msg << "Failed to cast <" << TranscodeString(res->getLocalName())
		    << "> from DOMNode to DOMAttr";
		throw std::runtime_error(msg.str());
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

		std::stringstream msg;
		msg << "Invalid value \"" << src << "\". Expected: \"true\" or \"false\"";
		throw std::runtime_error(msg.str());
	}

	//Wasn't found.
	if (!defValue)
	{
		std::stringstream msg;
		msg << "No value provided for boolean attribute.";
		throw std::runtime_error(msg.str());
	}

	return *defValue;
}

int ParseInteger(const XMLCh* srcX, int* defValue)
{
	if (srcX)
	{
		int value = 0;
		std::string src = TranscodeString(srcX);

		try
		{
			value = boost::lexical_cast<int>(src);
		}
		catch (boost::bad_lexical_cast const&)
		{
			std::stringstream msg;
			msg << "Invalid value \"" << src << "\". Expected: Integer value";
			throw std::runtime_error(msg.str());
		}

		return value;
	}

	//Wasn't found.
	if (!defValue)
	{
		std::stringstream msg;
		msg << "No value provided for integer attribute.";
		throw std::runtime_error(msg.str());
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
		std::stringstream msg;
		msg << "No value provided for float attribute.";
		throw std::runtime_error(msg.str());
	}

	return *defValue;
}

unsigned int ParseUnsignedInt(const XMLCh* srcX, unsigned int* defValue)
{
	if (srcX)
	{
		unsigned int value = 0;
		std::string src = TranscodeString(srcX);

		try
		{
			value = boost::lexical_cast<unsigned int>(src);
		}
		catch (boost::bad_lexical_cast const&)
		{
			std::stringstream msg;
			msg << "Invalid value \"" << src << "\". Expected: Unsigned integer value";
			throw std::runtime_error(msg.str());
		}

		return value;
	}

	//Wasn't found.
	if (!defValue)
	{
		std::stringstream msg;
		msg << "No value provided for unsigned int attribute.";
		throw std::runtime_error(msg.str());
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
		std::stringstream msg;
		msg << "No value provided for string attribute.";
		throw std::runtime_error(msg.str());
	}

	return *defValue;
}

//Helper: amount+value for time granularity.
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
			std::stringstream msg;
			msg << "Invalid units provided for granularity. Expected: \"seconds\" or \"minutes\" or \"ms\"";
			throw std::runtime_error(msg.str());
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
		Warn() << "WARNING! Rounding granularity value in from " << amount << "ms to " << res << "ms\n";
	}

	return res;
}

unsigned int GetValueInSecond(double amount, std::string units, unsigned int* defValue)
{
	//Handle plural
	if (units == "second") { units = "seconds"; }
	if (units == "minute") { units = "minutes"; }
	if (units == "hour") { units = "hours"; }

	//Detect errors
	if (units.empty() || (units != "minutes" && units != "seconds" && units != "ms" && units!= "hours"))
	{
		if (defValue)
		{
			return *defValue;
		}
		else
		{
			std::stringstream msg;
			msg << "Invalid units provided for granularity. Expected: \"seconds\" or \"minutes\" or \"ms\"";
			throw std::runtime_error(msg.str());
		}
	}

	//Reduce to seconds
	if (units == "ms") { amount /= 1000; }
	if (units == "minutes") { amount *= 60; }
	if (units == "hours") { amount *= 3600; }

	//Check for overflow:
	unsigned int res = static_cast<unsigned int>(amount);

	if (static_cast<double>(res) != amount)
	{
		Warn() << "WARNING! Rounding granularity value in from " << amount << "ms to " << res << "ms\n";
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

unsigned int ParseTimegranAsSeconds(const XMLCh* amountX, const XMLCh* unitsX, unsigned int* defValue)
{
	double amount = boost::lexical_cast<double>(TranscodeString(amountX));
	std::string units = TranscodeString(unitsX);
	return GetValueInSecond(amount, units, defValue);
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
		std::stringstream msg;
		msg << "No value provided for time attribute.";
		throw std::runtime_error(msg.str());
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
		std::stringstream msg;
		msg << "No value provided for string attribute.";
		throw std::runtime_error(msg.str());
	}

	return *defValue;
}

//How to do defaults
bool ParseBoolean(const XMLCh* src, bool defValue)
{
	return ParseBoolean(src, &defValue);
}

bool ParseBoolean(const XMLCh* src)
{
	//No default
	return ParseBoolean(src, nullptr);
}

int ParseInteger(const XMLCh* src, int defValue)
{
	return ParseInteger(src, &defValue);
}

int ParseInteger(const XMLCh* src)
{
	//No default
	return ParseInteger(src, nullptr);
}

unsigned int ParseUnsignedInt(const XMLCh* src, unsigned int defValue)
{
	return ParseUnsignedInt(src, &defValue);
}

unsigned int ParseUnsignedInt(const XMLCh* src)
{
	//No default
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
{
	//No default
	return ParseDailyTime(src, nullptr);
}

std::string ParseString(const XMLCh* src, std::string defValue)
{
	return ParseString(src, &defValue);
}

std::string ParseString(const XMLCh* src)
{
	//No default
	return ParseString(src, nullptr);
}

std::string ParseNonemptyString(const XMLCh* src, std::string defValue)
{
	return ParseNonemptyString(src, &defValue);
}

std::string ParseNonemptyString(const XMLCh* src)
{
	//No default
	return ParseNonemptyString(src, nullptr);
}

unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units, unsigned int defValue)
{
	return ParseTimegranAsMs(amount, units, &defValue);
}

unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units)
{
	//No default
	return ParseTimegranAsMs(amount, units, nullptr);
}

unsigned int ParseTimegranAsSecond(const XMLCh* amount, const XMLCh* units, unsigned int defValue)
{
	return ParseTimegranAsSeconds(amount, units, &defValue);
}

//TODO: Same issue; needs to be easier access to these things.
std::string ProcessValueString(xercesc::DOMElement* node)
{
	return ParseString(GetNamedAttributeValue(node, "value"));
}

}





