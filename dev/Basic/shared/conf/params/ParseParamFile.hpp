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
namespace sim_mob {
using namespace xercesc;
class ParseParamFile {
public:
	ParseParamFile(const std::string& paramFileName);
	virtual ~ParseParamFile();

public:
	void parseElement(DOMElement* e);
};

} /* namespace sim_mob */

