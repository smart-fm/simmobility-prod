//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include "MT_Config.hpp"
#include "conf/ConfigParams.hpp"
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
	ParseMidTermConfigFile(const std::string& configFileName, MT_Config& result, ConfigParams& cfgPrms);

private:
	//virtual override
	virtual void processXmlFile(xercesc::XercesDOMParser& parser);

	/**
	 * processes mid term run mode - supply or preday
	 * sets the run mode in configParams
	 * @param node node corresponding to the mid-term run mode element in xml file
	 */
	void processMidTermRunMode(xercesc::DOMElement* node);

	/**
	 * processes the supply element in config file
	 * @param node node corresponding to the supply element in xml file
	 */
	void processSupplyNode(xercesc::DOMElement* node);

	/**
	 * processes the preday element in config file
	 * @param node node corresponding to the preday element in xml file
	 */
	void processPredayNode(xercesc::DOMElement* node);

	/**
	 * processes the stored procedure mappings under supply node
	 * @param node node corresponding to the procedure mappings for supply in xml file
	 */
	void processProcMapNode(xercesc::DOMElement* node);

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

	/**
	 * processes model scripts element in config xml
	 * @param node node corresponding to model_scripts element inside xml file
	 */
	void processModelScriptsNode(xercesc::DOMElement* node);

	/**
	 * processes mongo collections element in config xml
	 * @param node node corresponding to mongo_collections element inside xml file
	 */
	void processMongoCollectionsNode(xercesc::DOMElement* node);

	/**
	 * processes calibration element in config xml
	 * @param node node corresponding to calibration element inside xml file
	 */
	void processCalibrationNode(xercesc::DOMElement* node);

	/**The config we are currently loading*/
	MT_Config& mtCfg;

	/**config params already loaded from the global config file*/
	ConfigParams& configParams;
};
}
