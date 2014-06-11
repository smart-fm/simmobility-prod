/*
 * Copyright Singapore-MIT Alliance for Research and Technology
 *
 * File:   RawConfigFile.hpp
 * Author: zhang huai peng
 *
 * Created on 2 Jun, 2014
 */

#pragma once

#include <string>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

namespace sim_mob {

/**
 * Base class for xml config parsers
 *
 * \author Seth N. Hetu
 * \author Harish Loganathan
 */
class ParseConfigXmlBase
{
public:
	ParseConfigXmlBase(const std::string& configFileName);
	virtual ~ParseConfigXmlBase();
	/**
	 * Parse a xml config file
	 * @param configFileName is the filename of configuration
	 * @return true if success otherwise false
	 */
	bool parseConfigFile(const std::string& configFileName);

protected:
	/**
	 * Does all the work. Parse xml configuration and process each node.
	 * Normally meant to be called from within the constructor of derived classes.
	 */
	void parseXmlAndProcess();

	void initXerces();

	std::string parseXmlFile(xercesc::XercesDOMParser& parser, xercesc::ErrorHandler& errorHandler);

	virtual void processXmlFile(xercesc::XercesDOMParser& parser) = 0;

	/**The path of the file we are loading our configuration from.*/
	std::string inFilePath;
};
}


