/* Copyright Singapore-MIT Alliance for Research and Technology */
//tripChains Branch

#include "ParseConfigFile.hpp"

#include <sstream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "conf/RawConfigParams.hpp"

using namespace sim_mob;
using namespace xercesc;


namespace {
//Helper: turn a Xerces error message into a string.
template <class XmlError>
std::string TranscodeErrorMessage(const XmlError& error) {
	char* raw = XMLString::transcode(error.getMessage());
	std::string errorMsg(raw);
	XMLString::release(&raw);
	return errorMsg;
}

} //End un-named namespace


sim_mob::ParseConfigFile::ParseConfigFile(const std::string& configFileName, RawConfigParams& result) : cfg(result), inFilePath(configFileName)
{
	//Print the network legacy format to our log file.
	ParseXmlAndProcess();
}

void sim_mob::ParseConfigFile::ParseXmlAndProcess()
{
	//NOTE: I think the order of destruction matters (parser must be listed last). ~Seth
	HandlerBase handBase;
	XercesDOMParser parser;

	//Attempt to parse it.
	std::string errorMsg = ParseXmlFile(parser, dynamic_cast<ErrorHandler&>(handBase));

	//If there's an error, throw it as an exception.
	if (!errorMsg.empty()) {
		throw std::runtime_error(errorMsg.c_str());
	}

	//Now process it.
	ProcessXmlFile(parser);
}

std::string sim_mob::ParseConfigFile::ParseXmlFile(XercesDOMParser& parser, ErrorHandler& errorHandler)
{
	//Xerces initialization.
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& error) {
		return TranscodeErrorMessage(error);
	}

	//Build a parser, set relevant properties on it.
	parser.setValidationScheme(XercesDOMParser::Val_Always);
	parser.setDoNamespaces(true); //This is optional.

	//Set an error handler.
	parser.setErrorHandler(&errorHandler);

	//Attempt to parse the XML file.
	try {
		parser.parse(inFilePath.c_str());
	} catch (const XMLException& error) {
		return TranscodeErrorMessage(error);
	} catch (const DOMException& error) {
		return TranscodeErrorMessage(error);
	} catch (...) {
		return "Unexpected Exception parsing config file.\n" ;
	}

	//No error.
	return "";
}


void sim_mob::ParseConfigFile::ProcessXmlFile(const XercesDOMParser& parser)
{

}









