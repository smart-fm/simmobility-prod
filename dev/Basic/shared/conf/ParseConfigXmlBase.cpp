/*
 * RawConfigFile.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include "conf/ParseConfigXmlBase.hpp"

#include <boost/filesystem.hpp>
#include <stdexcept>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include "util/XmlParseHelper.hpp"

using namespace sim_mob;
using namespace xercesc;

namespace sim_mob {
ParseConfigXmlBase::ParseConfigXmlBase(const std::string& configFileName) : inFilePath(configFileName) {}

ParseConfigXmlBase::~ParseConfigXmlBase() {}

void sim_mob::ParseConfigXmlBase::parseXmlAndProcess()
{
	//NOTE: I think the order of destruction matters (parser must be listed last). ~Seth
	initXerces();
	HandlerBase handBase;
	XercesDOMParser parser;

	//Attempt to parse it.
	std::string errorMsg = parseXmlFile(parser, dynamic_cast<ErrorHandler&>(handBase));

	//If there's an error, throw it as an exception.
	if (!errorMsg.empty()) {
		throw std::runtime_error(errorMsg.c_str());
	}

	//Now process it.
	processXmlFile(parser);
}

void ParseConfigXmlBase::initXerces()
{
	//Xerces initialization.
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& error) {
		throw std::runtime_error(TranscodeString(error.getMessage()).c_str());
	}
}

std::string ParseConfigXmlBase::parseXmlFile(XercesDOMParser& parser, ErrorHandler& errorHandler)
{
	//Build a parser, set relevant properties on it.
	parser.setValidationScheme(XercesDOMParser::Val_Always);
	parser.setDoNamespaces(true); //This is optional.

	//Set an error handler.
	parser.setErrorHandler(&errorHandler);

	//Attempt to parse the XML file.
	try {
		parser.parse(inFilePath.c_str());
	} catch (const XMLException& error) {
		return TranscodeString(error.getMessage());
	} catch (const DOMException& error) {
		return TranscodeString(error.getMessage());
	} catch (...) {
		return "Unexpected Exception parsing config file.\n" ;
	}

	//No error.
	return "";
}

}
