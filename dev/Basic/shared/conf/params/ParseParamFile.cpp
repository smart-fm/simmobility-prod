/*
 * ParseParamFile.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Max
 */

#include "ParseParamFile.hpp"
#include <stdexcept>
#include <iostream>

namespace sim_mob {

using namespace xercesc;
using namespace std;
//Helper: turn a Xerces error message into a string.
std::string TranscodeString(const XMLCh* str) {
	char* raw = XMLString::transcode(str);
	std::string res(raw);
	XMLString::release(&raw);
	return res;
}
ParseParamFile::ParseParamFile(const std::string& paramFileName,ParameterManager* paramMgr)
	:paramMgr(paramMgr)
	{

	cout<<"ParseParamFile: "<<paramFileName<<endl;
	//Xerces initialization.
	try {
		XMLPlatformUtils::Initialize();
	} catch (const XMLException& error) {
		throw std::runtime_error(TranscodeString(error.getMessage()).c_str());
	}

	HandlerBase handBase;
	XercesDOMParser parser;

	//Build a parser, set relevant properties on it.
	parser.setValidationScheme(XercesDOMParser::Val_Always);
	parser.setDoNamespaces(true); //This is optional.

	//Set an error handler.
	parser.setErrorHandler(&handBase);

	//Attempt to parse the XML file.
	try {
		parser.parse(paramFileName.c_str());
	} catch (const XMLException& error) {
		throw std::runtime_error( TranscodeString(error.getMessage()).c_str());
	} catch (const DOMException& error) {
		throw std::runtime_error(TranscodeString(error.getMessage()).c_str());
	} catch (...) {
		throw std::runtime_error( "Unexpected Exception parsing config file.\n" );
	}

	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	parseElement(rootNode);
}
void ParseParamFile::parseElement(DOMElement* e)
{
//	cout<<"parseElement: "<<TranscodeString(e->getTagName())<<endl;
	if( XMLString::equals(e->getTagName(), XMLString::transcode("param") ) )
	{
	   // Read attributes of element "param".
	   const XMLCh* xmlchName
			 = e->getAttribute(XMLString::transcode("name"));
	   string name = XMLString::transcode(xmlchName);
	   if(name.empty())
	   {
		   throw std::runtime_error("name is empty");
	   }
	   cout<<"name: "<<name;
	   const XMLCh* xmlchValue
			 = e->getAttribute(XMLString::transcode("value"));
	   string value = XMLString::transcode(xmlchValue);
	   if(value.empty())
	   {
		   throw std::runtime_error("value is empty");
	   }
	   cout<<"  value: "<<value<<endl;
	   // save to parameter manager
	   XmlRpc::XmlRpcValue v(value);
	   paramMgr->setParam(name,v);
	}
	DOMNodeList*      children = e->getChildNodes();

	const  XMLSize_t nodeCount = children->getLength();
	// For all nodes, children of "root" in the XML tree.
	for( XMLSize_t xx = 0; xx < nodeCount; ++xx )
	{
		DOMNode* currentNode = children->item(xx);
//		cout<<"parseElement: currentNode "<<TranscodeString(currentNode->getNodeName())<<endl;
		if( currentNode->getNodeType() &&  // true is not NULL
		 currentNode->getNodeType() == DOMNode::ELEMENT_NODE ) // is element
		{
			// Found node which is an Element. Re-cast node as element
			DOMElement* currentElement
						= dynamic_cast< xercesc::DOMElement* >( currentNode );
			parseElement(currentElement);
		}
	}
}
ParseParamFile::~ParseParamFile() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
