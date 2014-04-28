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
namespace sim_mob {

class ParseParamFile {
public:
	ParseParamFile(const std::string& paramFileName);
	virtual ~ParseParamFile();
};

} /* namespace sim_mob */

