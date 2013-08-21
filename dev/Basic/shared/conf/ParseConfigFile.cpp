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

sim_mob::ParseConfigFile::ParseConfigFile(const std::string& configFileName, RawConfigParams& result) : cfg(result), inFilePath(configFileName)
{
	//Print the network legacy format to our log file.
	ParseXmlAndProcess();
}

void sim_mob::ParseConfigFile::ParseXmlAndProcess()
{
	//Our error message string. If non-empty, it is thrown at the end of the function. Can be manually thrown before, if memory is cleaned up.
	std::stringstream errorMsg;;

	//Xerces initialization.
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& toCatch) {
		//Retrieve, copy, release.
		char* raw = XMLString::transcode(toCatch.getMessage());
		errorMsg << "Error during initialization! :\n" << raw << "\n";
		XMLString::release(&message);

		//Throw it now.
		throw std::runtime_error(errorMsg.str().c_str());
	}

	//Build a parser, set relevant properties on it.
	XercesDOMParser* parser = new XercesDOMParser(); //TODO: Static allocation?
	parser->setValidationScheme(XercesDOMParser::Val_Always);
	parser->setDoNamespaces(true); //This is optional.

	//Set an error handler. (TODO: Not quite sure what the cast is for.)
	ErrorHandler* errHandler = dynamic_cast<ErrorHandler*>(new HandlerBase());
	parser->setErrorHandler(errHandler);

	//Attempt to parse the XML file.
	try {
		parser->parse(inFilePath.c_str());
	} catch (const XMLException& toCatch) {
		//Retrieve, save, release.
		char* message = XMLString::transcode(toCatch.getMessage());
		errorMsg << "Exception message is: \n" << message << "\n";
		XMLString::release(&message);
     } catch (const DOMException& toCatch) {
    	//Retrieve, save, release.
    	char* message = XMLString::transcode(toCatch.msg);
    	errorMsg<< "Exception message is: \n" << message << "\n";
    	XMLString::release(&message);
     } catch (...) {
    	 errorMsg << "Unexpected Exception parsing config file.\n" ;
     }

     //Clean up our allocated memory.
     delete parser;
     delete errHandler;

     //If there's an error, throw it as an exception.
     if (!errorMsg.str().empty()) {
 		throw std::runtime_error(errorMsg.str().c_str());
     }

     //Now process it.
     ProcessXmlFile(parser);
}

std::string sim_mob::ParseConfigFile::ParseXmlFile(XercesDOMParser* parser)
{
	return "";

}


void sim_mob::ParseConfigFile::ProcessXmlFile(const XercesDOMParser* parser)
{

}









