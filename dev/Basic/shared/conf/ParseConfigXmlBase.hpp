//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/ErrorHandler.hpp>

namespace sim_mob
{

/**
 * Base class for xml config parsers
 *
 * \author Seth N. Hetu
 * \author Harish Loganathan
 */
class ParseConfigXmlBase
{
public:
    /**
     * Constructore
     *
     * @param configFileName configuration file name to be parsed
     */
	ParseConfigXmlBase(const std::string& configFileName);

    /**
     * Destructor
     */
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

	/**
	 * Perform per-process parser initialization
	 */
	void initXerces();

	/**
	 * parse xml file
     *
     * @param parser the parser to parse the xml
	 * @param errorHandler error handler in case of errors
     *
     * @return empty string in case of no errors; the error message other wise
	 */
	std::string parseXmlFile(xercesc::XercesDOMParser& parser, xercesc::ErrorHandler& errorHandler);

	/**
	 * pure virtual function to override in derived classes.
	 * code for processing the respective xml must be written in this function in the derived classes
     *
     * @param parser reference to parser after parsing the xml
	 */
	virtual void processXmlFile(xercesc::XercesDOMParser& parser) = 0;

    /// The path of the file we are loading our configuration from
	std::string inFilePath;
};
}

