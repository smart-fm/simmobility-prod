//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "conf/ParseConfigXmlBase.hpp"

#include <boost/filesystem.hpp>
#include <stdexcept>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include "util/XmlParseHelper.hpp"

#include "logging/Log.hpp"

using namespace sim_mob;
using namespace xercesc;

ParseConfigXmlBase::ParseConfigXmlBase(const std::string &configFileName) :
		inFilePath(configFileName)
{
}

ParseConfigXmlBase::~ParseConfigXmlBase()
{
}

void ParseConfigXmlBase::parseXmlAndProcess()
{
	//NOTE: I think the order of destruction matters (parser must be listed last). ~Seth
	initialiseXerces();
	HandlerBase errorHandler;
	XercesDOMParser parser;

	//Attempt to parse it.
	std::string errorMsg = parseXmlFile(parser, errorHandler);

	//If there's an error, throw it as an exception.
	if (!errorMsg.empty())
	{
		throw std::runtime_error(errorMsg.c_str());
	}

	//Now process it.
	processXmlFile(parser);
}

void ParseConfigXmlBase::initialiseXerces()
{
	//Xerces initialization.
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& error)
	{
		std::stringstream msg;
		msg << "XML toolkit initialisation error: " << TranscodeString(error.getMessage());
		throw std::runtime_error(msg.str());
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
	try
	{
		parser.parse(inFilePath.c_str());
	}
	catch (const XMLException &error)
	{
		std::stringstream msg;
		msg << "Error parsing file: " << inFilePath << ", " << TranscodeString(error.getMessage())
		    << " at line: " << error.getSrcLine();
		return msg.str();
	}
	catch (const DOMException &error)
	{
		std::stringstream msg;
		msg << "Error parsing file: " << inFilePath << ", " << TranscodeString(error.getMessage());
		return msg.str();
	}
	catch(const SAXParseException &error)
	{
		std::stringstream msg;
		msg << "Error parsing file: " << inFilePath << ", " << TranscodeString(error.getMessage())
		    << " at line: " << error.getLineNumber() << ", column: " << error.getColumnNumber();
		return msg.str();
	}
	catch (const std::exception &error)
	{
		std::stringstream msg;
		msg << "Error parsing file: " << inFilePath << ", " << error.what();
		return msg.str();
	}
	catch (...)
	{
		std::stringstream msg;
		msg << "Error parsing " << inFilePath << ", unknown error";
		return msg.str();
	}

	//No error.
	return "";
}



void ParseConfigXmlBase::processXmlFileForServiceControler(xercesc::XercesDOMParser& parser)
{

}

void ParseConfigXmlBase::parseXmlAndProcessForServiceController()
{
	//NOTE: I think the order of destruction matters (parser must be listed last). ~Seth
	initialiseXerces();
		HandlerBase handBase;
		XercesDOMParser parser;

		//Attempt to parse it.
		std::string errorMsg = parseXmlFile(parser, dynamic_cast<ErrorHandler&>(handBase));

		//If there's an error, throw it as an exception.
		if (!errorMsg.empty())
		{
			throw std::runtime_error(errorMsg.c_str());
		}

		//Now process it.
		//processXmlFile(parser);
		processXmlFileForServiceControler(parser);
}
