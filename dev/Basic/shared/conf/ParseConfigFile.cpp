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
std::string TranscodeString(const XMLCh* str) {
	char* raw = XMLString::transcode(str);
	std::string res(raw);
	XMLString::release(&raw);
	return res;
}

//Helper: retrieve child elements without leaking memory
/*DOMNodeList* GetElementsByName(DOMElement* node, const std::string& key) {
	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNodeList* res = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);
	return res;
}*/

//Helper: make sure we actually have an element
DOMElement* NodeToElement(DOMNode* node) {
	DOMElement* res = dynamic_cast<DOMElement*>(node);
	if (!res) {
		throw std::runtime_error("DOMNode is expected to be a DOMElement.");
	}
	return res;
}

//Helper: retrieve a single element; optionally required.
DOMElement* GetSingleElementByName(DOMElement* node, const std::string& key, bool required=false) {
	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNodeList* res = node->getElementsByTagName(keyX);
	XMLString::release(&keyX);

	//Check.
	if (res->getLength()>1) {
		throw std::runtime_error("Error: single element expected, but returned more than 1.");
	} else if (res->getLength()==1) {
		return NodeToElement(res->item(0));
	} else if (required) {
		throw std::runtime_error("Error: single element expected, but returned zero.");
	}

	return nullptr;
}

//Helper: retrieve an attribute
DOMAttr* GetNamedAttribute(DOMElement* node, const std::string& key, bool required=false) {
	if (!node) {
		return nullptr;
	}

	XMLCh* keyX = XMLString::transcode(key.c_str());
	DOMNode* res= node->getAttributes()->getNamedItem(keyX);
	XMLString::release(&keyX);

	//Check.
	if (!res) {
		if (required) {
			throw std::runtime_error("Error: attribute expected, but none found.");
		} else {
			return nullptr;
		}
	}

	DOMAttr* resAttr = dynamic_cast<DOMAttr*>(res);
	if (!resAttr) {
		throw std::runtime_error("Error: attribute expected, but couldn't be cast.");
	}

	return resAttr;
}

//Helper
const XMLCh* GetAttributeValue(const DOMAttr* attr) {
	if (!attr) {
		return nullptr;
	}
	return attr->getNodeValue();
}

//Helper: boolean stuff
bool ParseBoolean(const XMLCh* srcX, bool* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		if (src=="true") {
			return true;
		} else if (src=="false") {
			return false;
		}
		throw std::runtime_error("Expected boolean value.");
	}

	//Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory boolean variable; no default available.");
	}
	return *defValue;
}

//How to do defaults
bool ParseBoolean(const XMLCh* src, bool defValue) {
	return ParseBoolean(src, &defValue);
}
bool ParseBoolean(const XMLCh* src) { //No default
	return ParseBoolean(src, nullptr);
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
	InitXerces();
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

void sim_mob::ParseConfigFile::InitXerces()
{
	//Xerces initialization.
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& error) {
		throw std::runtime_error(TranscodeString(error.getMessage()).c_str());
	}
}

std::string sim_mob::ParseConfigFile::ParseXmlFile(XercesDOMParser& parser, ErrorHandler& errorHandler)
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


void sim_mob::ParseConfigFile::ProcessXmlFile(XercesDOMParser& parser)
{
	//Verify that the root node is "config"
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	if (TranscodeString(rootNode->getTagName()) != "config") {
		throw std::runtime_error("xml parse error: root node must be \"config\"");
	}

	//Now just parse the document recursively.
	ProcessSystemNode(GetSingleElementByName(rootNode,"system", true));
	ProcessGeometryNode(GetSingleElementByName(rootNode, "geometry", true));
	ProcessDriversNode(GetSingleElementByName(rootNode, "drivers"));
	ProcessPedestriansNode(GetSingleElementByName(rootNode, "pedestrians"));
	ProcessBusDriversNode(GetSingleElementByName(rootNode, "busdrivers"));
	ProcessSignalsNode(GetSingleElementByName(rootNode, "signals"));

	//TEMP
	throw 1;
}


void sim_mob::ParseConfigFile::ProcessSystemNode(DOMElement* node)
{
	ProcessSystemSimulationNode(GetSingleElementByName(node, "simulation", true));
	ProcessSystemWorkersNode(GetSingleElementByName(node, "workers", true));
	ProcessSystemSingleThreadedNode(GetSingleElementByName(node, "single_threaded"));
	ProcessSystemMergeLogFilesNode(GetSingleElementByName(node, "merge_log_files"));
	ProcessSystemNetworkSourceNode(GetSingleElementByName(node, "network_source"));

	// value
	//network_xml_file
	//xsd_schema_files
	//generic_props
}


void sim_mob::ParseConfigFile::ProcessGeometryNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessDriversNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessPedestriansNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessBusDriversNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessSignalsNode(xercesc::DOMElement* node)
{
}

void sim_mob::ParseConfigFile::ProcessSystemSimulationNode(xercesc::DOMElement* node)
{

}

void sim_mob::ParseConfigFile::ProcessSystemWorkersNode(xercesc::DOMElement* node)
{

}

void sim_mob::ParseConfigFile::ProcessSystemSingleThreadedNode(xercesc::DOMElement* node)
{
	//TODO: GetAttribute not working; there's a null throwing it off.
	cfg.system.singleThreaded = ParseBoolean(GetAttributeValue(GetNamedAttribute(node, "value")), false);
	std::cout <<"Single: " <<cfg.system.singleThreaded <<std::endl;
}


void sim_mob::ParseConfigFile::ProcessSystemMergeLogFilesNode(xercesc::DOMElement* node)
{
	cfg.system.mergeLogFiles = ParseBoolean(GetAttributeValue(GetNamedAttribute(node, "value")), false);
}


void sim_mob::ParseConfigFile::ProcessSystemNetworkSourceNode(xercesc::DOMElement* node)
{

}





