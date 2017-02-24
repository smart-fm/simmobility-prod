/*
 * ParseParamFile.hpp
 *
 *  Created on: Apr 28, 2014
 *      Author: redheli
 */

#pragma once
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <string>

#include "ParameterManager.hpp"

namespace sim_mob
{
using namespace xercesc;

class ParameterManager;

class ParseParamFile
{
public:
	ParameterManager *paramMgr;
	std::string fileName;
	std::string modelName;
	
	ParseParamFile(const std::string& paramFileName, ParameterManager *paramMgr);
	virtual ~ParseParamFile();
	void parseElement(DOMElement *e);
};

}

