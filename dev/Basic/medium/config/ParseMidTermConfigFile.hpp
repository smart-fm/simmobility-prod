//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include "MT_Config.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ParseConfigXmlBase.hpp"
#include "conf/RawConfigParams.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;
using namespace xercesc;

namespace sim_mob
{

namespace medium
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
    /**
     * Parse a config file into MT_Config, performing all XML parsing and some trivial semantic processing
     *
     * @param configFileName - Config file to be processed
     *
     * @param result - Configuration values extracted from config file
     */
    ParseMidTermConfigFile(const std::string& configFileName, MT_Config& result, ConfigParams& cfg);

private:
    /**
     * Code for processing the xml
     *
     * @param configFileName is the filename of configuration
     */
	virtual void processXmlFile(xercesc::XercesDOMParser& parser);

	/**
	 * processes mid term run mode - supply or preday
	 * sets the run mode in configParams
     *
	 * @param node node corresponding to the mid-term run mode element in xml file
	 */
	void processMidTermRunMode(xercesc::DOMElement* node);

	/**
     * processes the supply element in config file
     *
	 * @param node node corresponding to the supply element in xml file
	 */
	void processSupplyNode(xercesc::DOMElement* node);

	/**
	 * processes the preday element in config file
     *
	 * @param node node corresponding to the preday element in xml file
	 */
	void processPredayNode(xercesc::DOMElement* node);

	/**
	 * processes the stored procedure mappings under supply node
     *
	 * @param node node corresponding to the procedure mappings for supply in xml file
	 */
	void processProcMapNode(xercesc::DOMElement* node);

	/**
	 * processes the activity load interval node
     *
	 * @param node node specifying the value and unit for activity load interval
	 */
	void processActivityLoadIntervalElement(xercesc::DOMElement* node);

	/**
	 * processes update interval node
     *
	 * @param node node corresponding to update interval element
	 */
	void processUpdateIntervalElement(xercesc::DOMElement* node);

	/**
	 * processes dwell time element included in xml file.
     *
	 * @param node node corresponding to the dwell time element inside xml file
	 */
	void processDwellTimeElement(xercesc::DOMElement* node);

	/**
	 * processes default capacity bust included in xml file.
     *
	 * @param node node corresponding to the bus capacity element inside xml file
	 */
	void processBusCapactiyElement(xercesc::DOMElement* node);

	/**
	 * processes pedestrian walk speed included in xml file.
     *
	 * @param node node corresponding to pedestrian walk speed element inside xml file
	 */
	void processWalkSpeedElement(xercesc::DOMElement* node);

	/**
     * processes model scripts element in config xml
     *
	 * @param node node corresponding to model_scripts element inside xml file
	 */
	ModelScriptsMap processModelScriptsNode(xercesc::DOMElement* node);

	/**
	 * processes mongo collections element in config xml
     *
	 * @param node node corresponding to mongo_collections element inside xml file
	 */
	void processMongoCollectionsNode(xercesc::DOMElement* node);

	/**
	 * processes statistics output file element in config xml
     *
	 * @param node node corresponding to statistics output file element inside xml file
	 */
	void processStatisticsOutputNode(xercesc::DOMElement* node);

	/**
	 * processes node for speed-density function params
	 */
	void processSpeedDensityParamsNode(xercesc::DOMElement* node);

	/**
	 * processes calibration element in config xml
     *
	 * @param node node corresponding to calibration element inside xml file
	 */
	void processCalibrationNode(xercesc::DOMElement* node);

    /**
     * Processes the system element in the config file
     *
     * @param node node corresponding to the system element inside xml file
     */
    void processSystemNode(xercesc::DOMElement* node);

    /**
     * Processes the generic_props element in the config file
     *
     * @param node node corresponding to the generic_props element inside xml file
     */
    void processGenericPropsNode(xercesc::DOMElement* node);

    /**
     * processes network_database element in config xml
     *
     * @param node node corresponding to network_database element inside xml file
     */
    void processDatabaseNode(xercesc::DOMElement* node, DatabaseDetails& dbDetails);

    /**
     * processes the workers element in config xml
     *
     * @param node node corresponding to workers element inside xml file
     */
    void processWorkersNode(xercesc::DOMElement* node);

    /**
     * processes the person element in config xml
     *
     * @param node node corresponding to person element inside xml file
     */
    void processWorkerPersonNode(xercesc::DOMElement* node);

    /**
     * processes the ScreenLine element in config xml
     *
     * @param node node corresponding to screenline element inside xml file
     */
    void processScreenLineNode(xercesc::DOMElement* node);

    /**
     * processes the cbd element in config xml
     *
     * @param node node corresponding to cbd element inside xml file
     */
    void processCBDNode(xercesc::DOMElement* node);

    /**
     * processes the generateBusRoutes element in config xml
     *
     * @param node node corresponding to generateBusRoutes element inside xml file
     */
    void processGenerateBusRoutesNode(xercesc::DOMElement* node);

    /**
     * processes the subtrip_travel_metrics_output element in config xml
     *
     * @param node node corresponding to generateBusRoutes element inside xml file
     */
    void processSubtripTravelMetricsOutputNode(xercesc::DOMElement* node);

    /**
     * processes the bus stop schedule times element in config xml
     *
     * @param node node correspoding to scheduledTimes element inside xml file
     */
    void processBusStopScheduledTimesNode(xercesc::DOMElement* node);

    /**
     * processes the travel time element the config xml
     *
     * @param node node corresponding to trave_time element inside xml file
     */
    void processTT_Update(xercesc::DOMElement* node);

    /**
     * Processes the publicTransit element in config xml
     *
     * @param node node corresponding to the publicTransit element inside xml file
     */
    void processPublicTransit(xercesc::DOMElement* node);

    /**
     * Processes the incidents element in config xml
     *
     * @param node node corresponding to the incidents element inside xml file
     */
    void processIncidentsNode(xercesc::DOMElement* node);

    /**
     * Processes the busController element in config xml
     *
     * @param node node corresponding to the busController element inside xml file
     */
    void processBusControllerNode(xercesc::DOMElement *node);

    /**
     * Processes the trainController element in config xml
     *
     * @param node node corresponding to the trainController element inside xml file
     */
    void processTrainControllerNode(xercesc::DOMElement *node);

    void processPathSetFileName(xercesc::DOMElement* node);

    /// The config we are currently loading
	MT_Config& mtCfg;

    /// Shared configuration
    ConfigParams& cfg;
};
} // namespace medium
} // namespace sim_mob
