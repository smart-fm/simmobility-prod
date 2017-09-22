//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/util/Xerces_autoconf_config.hpp>

#include "DailyTime.hpp"

using namespace sim_mob;
using namespace xercesc;
namespace sim_mob
{
/**
 * Helper: turn a Xerces error message into a string.
 * @param str the error msg to transcode
 * @return error message as std::string
 */
std::string TranscodeString(const XMLCh* str);

/**
 * Helper: make sure we actually have an element
 * @param node the dom node to convert
 * @return DOMElement* casted from node (DOMNode is the base class for DOMElement)
 */
DOMElement* NodeToElement(DOMNode* node);

/**
 * Helper: retrieve child elements without leaking memory
 * @param node the xml node to get elements from
 * @param key the name of element to seek
 * @param required flag to indicate whether the element we seek is a mandatory element or not
 * @return a vector of elements named as key
 */
std::vector<DOMElement*> GetElementsByName(DOMElement* node, const std::string& key, bool required=false);

/**
 * Helper: retrieve a single element; optionally required.
 * @param node the xml node to get elements from
 * @param key the name of element to seek
 * @param required flag to indicate whether the element we seek is a mandatory element or not
 * @return element named as key under node
 */
DOMElement* GetSingleElementByName(DOMElement* node, const std::string& key, bool required=false);

/**
 * Helper: retrieve an attribute
 * @param node the xml node to get attribute from
 * @param key the name of attribute to seek
 * @param required flag to indicate whether the element we seek is a mandatory element or not
 * @return attribute with name as key in node
 */
DOMAttr* GetNamedAttribute(DOMElement* node, const std::string& key, bool required=false);

/**
 * Helper: get value of an attribute
 * @param attr attribute to get value from
 * @return value of attribute
 */
const XMLCh* GetAttributeValue(const DOMAttr* attr);

/**
 * Helper: Combined
 * @param node the xml node to get attribute from
 * @param key the name of attribute to seek
 * @param required flag to indicate whether the element we seek is a mandatory element or not
 * @return value of attribute with name as key in node
 */
const XMLCh* GetNamedAttributeValue(DOMElement* node, const std::string& key, bool required=true);

/**
 * Helper: parse boolean
 * @param srcX XMLCh* to parse to boolean
 * @param defValue default value of parsing fails
 * @return bool value after parsing srcX
 */
bool ParseBoolean(const XMLCh* srcX, bool* defValue);

/**
 * Helper: parse int
 * @param srcX XMLCh* to parse to int
 * @param defValue default value of parsing fails
 * @return int value after parsing srcX
 */
int ParseInteger(const XMLCh* srcX, int* defValue);

/**
 * Helper: parse float
 * @param srcX XMLCh* to parse to float
 * @param defValue default value of parsing fails
 * @return float value after parsing srcX
 */
float ParseFloat(const XMLCh* srcX, float* defValue);

/**
 * Helper: parse unsigned int
 * @param srcX XMLCh* to parse to unsigned int
 * @param defValue default value of parsing fails
 * @return unsigned int value after parsing srcX
 */
unsigned int ParseUnsignedInt(const XMLCh* srcX, unsigned int* defValue);

/**
 * Helper: parse string
 * @param srcX XMLCh* to parse to string
 * @param defValue default value of parsing fails
 * @return string value after parsing srcX
 */
std::string ParseString(const XMLCh* srcX, std::string* defValue);

/**
 * Helper: amount+value for time-granularities.
 * @param amount input time value
 * @param units unit of input time value
 * @param defValue default value in case of invalid units
 * @return time equivalent to amount in ms
 */
unsigned int GetValueInMs(double amount, std::string units, unsigned int* defValue);

/**
 * Helper: amount+value for time-granularities.
 * @param amount input time value
 * @param units unit of input time value
 * @param defValue default value in case of invalid units
 * @return time equivalent to amount in seconds
 */
unsigned int GetValueInSecond(double amount, std::string units, unsigned int* defValue);

/**
 * Helper: Time units such as "10", "seconds"
 * @param amountX input time value
 * @param unitsX unit of input time value
 * @param defValue default value in case of invalid units
 * @return time equivalent to amount in ms
 */
unsigned int ParseTimegranAsMs(const XMLCh* amountX, const XMLCh* unitsX, unsigned int* defValue);

/**
 * Helper: Time units such as "1", "hour"
 * @param amountX input time value
 * @param unitsX unit of input time value
 * @param defValue default value in case of invalid units
 * @return time equivalent to amount in seconds
 */
unsigned int ParseTimegranAsSeconds(const XMLCh* amountX, const XMLCh* unitsX, unsigned int* defValue);

/**
 * Helper: Parse DailyTime
 * @param srcX XMLCh* to parse to DailyTime
 * @param defValue default value of parsing fails
 * @return DailyTime from parsing srcX
 */
DailyTime ParseDailyTime(const XMLCh* srcX, DailyTime* defValue);

/**
 * Helper: parse non-empty string
 * @param srcX XMLCh* to parse to string
 * @param defValue default value of parsing fails
 * @return string value after parsing srcX
 */
std::string ParseNonemptyString(const XMLCh* srcX, std::string* defValue);

//The following functions only mirror the functionality of the corresponding functions above
bool ParseBoolean(const XMLCh* src, bool defValue);
bool ParseBoolean(const XMLCh* src);
int ParseInteger(const XMLCh* src, int defValue);
int ParseInteger(const XMLCh* src);
unsigned int ParseUnsignedInt(const XMLCh* src, unsigned int defValue);
unsigned int ParseUnsignedInt(const XMLCh* src);
float ParseFloat(const XMLCh* src);
float ParseFloat(const XMLCh* src, float defValue);
DailyTime ParseDailyTime(const XMLCh* src, DailyTime defValue);
DailyTime ParseDailyTime(const XMLCh* src);
std::string ParseString(const XMLCh* src, std::string defValue);
std::string ParseString(const XMLCh* src);
std::string ParseNonemptyString(const XMLCh* src, std::string defValue);
std::string ParseNonemptyString(const XMLCh* src);
unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units, unsigned int defValue);
unsigned int ParseTimegranAsMs(const XMLCh* amount, const XMLCh* units);
unsigned int ParseTimegranAsSecond(const XMLCh* amount, const XMLCh* units, unsigned int defValue);
std::string ProcessValueString(xercesc::DOMElement* node);
}
