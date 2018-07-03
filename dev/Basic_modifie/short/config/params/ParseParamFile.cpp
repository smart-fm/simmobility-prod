/*
 * ParseParamFile.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: Max
 */

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "ParseParamFile.hpp"
#include "ParamData.hpp"
#include "util/XmlParseHelper.hpp"

using namespace sim_mob;
using namespace xercesc;
using namespace std;

ParseParamFile::ParseParamFile(const std::string& paramFileName, ParameterManager *paramMgr)
: paramMgr(paramMgr), fileName(paramFileName)
{
	//Xerces initialization.
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& error)
	{
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
	try
	{
		parser.parse(paramFileName.c_str());
	}
	catch (const XMLException& error)
	{
		stringstream msg;
		msg << "Failed to parse parameter file: " << paramFileName << ". Error: " << TranscodeString(error.getMessage()).c_str();
		throw std::runtime_error(msg.str());
	}
	catch (const DOMException& error)
	{
		stringstream msg;
		msg << "Failed to parse parameter file: " << paramFileName << ". Error: " << TranscodeString(error.getMessage()).c_str();
		throw std::runtime_error(msg.str());
	}
	catch (...)
	{
		stringstream msg;
		msg << "Unexpected Exception while parsing parameter file: " << paramFileName;
		throw std::runtime_error(msg.str());
	}

	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	if (XMLString::equals(rootNode->getTagName(), XMLString::transcode("parameters")))
	{
		// get model name
		const XMLCh* xmlchName = rootNode->getAttribute(XMLString::transcode("model_name"));
		modelName = XMLString::transcode(xmlchName);
		
		if (modelName.empty())
		{
			stringstream msg;
			msg << "In parameter file: " << paramFileName << " the attribute 'model_name' under the 'parameters' tag is empty!";
			throw std::runtime_error(msg.str());
		}

		parseElement(rootNode);
	}
	else
	{
		stringstream msg;
		msg << "In parameter file: " << paramFileName << " the 'parameters' tag is missing!";
		throw std::runtime_error(msg.str());
	}
}

void ParseParamFile::parseElement(DOMElement* e)
{
	if (XMLString::equals(e->getTagName(), XMLString::transcode("param")))
	{
		// Read attributes of element "param".
		const XMLCh* xmlchName = e->getAttribute(XMLString::transcode("name"));
		string name = XMLString::transcode(xmlchName);
		
		if (name.empty())
		{
			stringstream msg;
			msg << "In parameter file: " << fileName << " the attribute 'name' under a 'param' tag is empty!";
			throw std::runtime_error(msg.str());
		}

		const XMLCh* xmlchValue = e->getAttribute(XMLString::transcode("value"));
		string value = XMLString::transcode(xmlchValue);
		
		if (value.empty())
		{
			stringstream msg;
			msg << "In parameter file: " << fileName << " the attribute 'value' under a 'param' tag is empty!";
			throw std::runtime_error(msg.str());
		}

		// save to parameter manager
		ParamData v(value);
		v.setParaFileName(fileName);
		paramMgr->setParam(modelName, name, v);
	}
	
	DOMNodeList *children = e->getChildNodes();
	const XMLSize_t nodeCount = children->getLength();
	
	// For all nodes, children of "root" in the XML tree.
	for (XMLSize_t xx = 0; xx < nodeCount; ++xx)
	{
		DOMNode* currentNode = children->item(xx);

		if (currentNode->getNodeType() && // true is not NULL
				currentNode->getNodeType() == DOMNode::ELEMENT_NODE) // is element
		{
			// Found node which is an Element. Re-cast node as element
			DOMElement *currentElement = dynamic_cast<xercesc::DOMElement*> (currentNode);
			parseElement(currentElement);
		}
	}
}

ParseParamFile::~ParseParamFile()
{
	// TODO Auto-generated destructor stub
}
