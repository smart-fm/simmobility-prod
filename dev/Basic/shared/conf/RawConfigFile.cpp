/*
 * RawConfigFile.cpp
 *
 *  Created on: 2 Jun, 2014
 *      Author: zhang
 */

#include <conf/RawConfigFile.hpp>
using namespace sim_mob;
using namespace xercesc;
namespace sim_mob {
RawConfigFile::RawConfigFile() {
	// TODO Auto-generated constructor stub

}

RawConfigFile::~RawConfigFile() {
	// TODO Auto-generated destructor stub
}

bool RawConfigFile::parseConfigFile(const std::string& configFileName) {
	parser.setValidationScheme(XercesDOMParser::Val_Always);
	parser.setDoNamespaces(true);

	try {
		parser.parse(configFileName.c_str());
	} catch (...) {
		return false;
	}

	return true;
}


void RawConfigFile::parseXmlAndProcess()
{
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	if (!rootNode || TranscodeString(rootNode->getTagName()) != "config") {
		throw std::runtime_error(
				"xml parse error: root node must be \"config\"");
	}

	//Now just parse the document recursively.
	XMLSize_t count = rootNode->getChildElementCount();
	DOMElement* node = rootNode->getFirstElementChild();
	while(node){
		processElement(node, TranscodeString(rootNode->getTagName()));
		DOMElement* node = rootNode->getNextElementSibling();
	}
}

}
