/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   RawConfigFile.hpp
 * Author: zhang huai peng
 *
 * Created on 2 Jun, 2014
 */

#pragma once

#include "ParseConfigFile.hpp"
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

namespace sim_mob {

class RawConfigFile {
public:
	RawConfigFile();
	virtual ~RawConfigFile();
	/**
	 * Parse a xml config file
	 * @param configFileName is the filename of configuration
	 * @return true if success otherwise false
	 */
	bool parseConfigFile(const std::string& configFileName);

protected:
	/**
	 * parse a xml configuration file and process each node.
	 */
	void parseXmlAndProcess();

	/**
	 * process each node included in xml file. this method should be override by its children
	 * @param node is a element inside xml file
	 */
	virtual void processElement(xercesc::DOMElement* node) = 0;

private:
	xercesc::XercesDOMParser parser;
};

}


