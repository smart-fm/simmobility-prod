//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * ParseMidTermConfigFile.hpp
 *
 *  Created on: Jun 11, 2014
 *      Author: smart-fm
 */
#pragma once

#include <boost/noncopyable.hpp>
#include "MT_Config.hpp"
#include "conf/ParseConfigXmlBase.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
using namespace xercesc;

namespace sim_mob
{
/**
 * Parser for the mid-term config file
 *
 * \author Harish Loganathan
 * \author Zhang Huai Peng
 */
class ParseMidTermConfigFile: public ParseConfigXmlBase, private boost::noncopyable
{
public:
	/**Parse a config file into MT_Config, performing all XML parsing and some trivial semantic processing*/
	ParseMidTermConfigFile(const std::string& configFileName, MT_Config& result);

private:
	virtual void processXmlFile(xercesc::XercesDOMParser& parser);

	/**
	 * processes dwell time element included in xml file.
	 * @param node node corresponding to the dwell time element inside xml file
	 */
	void processDwellTimeElement(xercesc::DOMElement* node);

	/**
	 * processes pedestrian walk speed included in xml file.
	 * @param node node corresponding to pedestrian walk speed element inside xml file
	 */
	void processWalkSpeedElement(xercesc::DOMElement* node);

	/**The config we are currently loading*/
	MT_Config& mtCfg;
};
}
