//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include "ST_Config.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/ParseConfigXmlBase.hpp"

namespace sim_mob
{
/**
 * Parser for the short-term config file
 *
 * \author Balakumar Marimuthu
 */
class ParseShortTermConfigFile : public ParseConfigXmlBase, private boost::noncopyable
{
public:
    /**
     * Parse a config file into ST_Config, performing all XML parsing and some trivial semantic processing
     *
     * @param configFileName - config file name which needs to be parsed
     * @param sharedCfg - the the values extracted from the shared configuration file
     * @param result - Configuration values extracted from config file
     */
    ParseShortTermConfigFile(const std::string& configFileName, ConfigParams& sharedCfg, ST_Config& result);

private:
    /**
     * Code for processing the xml
     *
     * @param configFileName is the filename of configuration
     */
    virtual void processXmlFile(xercesc::XercesDOMParser& parser);

    /**
     * Processes the db_proc_groups (stored procedures for network) element in the config file
     *
     * @param node node corresponding to the db_proc_groups element in the xml file
     */
    void processProcMapNode(xercesc::DOMElement* node);

    /**
     * Processes the amod_controller element in the config file
     *
     * @param node node corresponding to the amod_controller element in the xml file
     */
    void processAmodControllerNode(xercesc::DOMElement* node);

    /**
     * Processes the fmod_controller element in the config file
     *
     * @param node node corresponding to the fmod_controller element in the xml file
     */
    void processFmodControllerNode(xercesc::DOMElement* node);

    /**
     * Processes the segment_density element in the config file
     *
     * @param node node corresponding to the segment_density element in the xml file
     */
    void processSegmentDensityNode(xercesc::DOMElement* node);

    /**
     * Processes the system element in the config file
     *
     * @param node node corresponding to the system element in the xml file
     */
    void processSystemNode(xercesc::DOMElement* node);
	
	/**
     * Processes the model scripts element in config file
     *
	 * @param node node corresponding to model_scripts element inside the xml file
	 */
	ModelScriptsMap processModelScriptsNode(xercesc::DOMElement* node);

    /**
     * Processes the network element in the config file
     *
     * @param node node corresponding to the network element in the xml file
     */
    void processNetworkNode(xercesc::DOMElement* node);

    /**
     * Processes the loop-detector_counts element in the config file
     *
     * @param node node corresponding to the loop-detector_counts element in the xml file
     */
    void processLoopDetectorCountNode(xercesc::DOMElement* node);

    /**
     * Processes the xsd_schema_files element in the config file
     *
     * @param node node corresponding to the xsd_schema_files element in the xml file
     */
    void processXmlSchemaFilesNode(xercesc::DOMElement* node);

    /**
     * Processes the network_xml_file_output element in the config file
     *
     * @param node node corresponding to the network_xml_file_output element in the xml file
     */
    void processNetworkXmlOutputNode(xercesc::DOMElement* node);

    /**
     * Processes the network_xml_file_input element in the config file
     *
     * @param node node corresponding to the network_xml_file_input element in the xml file
     */
    void processNetworkXmlInputNode(xercesc::DOMElement* node);

    /**
     * Processes the network_source element in the config file
     *
     * @param node node corresponding to the network_source element in the xml file
     */
    void processNetworkSourceNode(xercesc::DOMElement* node);

    /**
     * Processes the network_database element in the config file
     *
     * @param node node corresponding to the network_database element in the xml file
     */
    void processDatabaseNode(xercesc::DOMElement* node);

    /**
     * Processes the workers element in the config file
     *
     * @param node node correspoding to the workers element in the xml file
     */
    void processWorkersNode(xercesc::DOMElement* node);

    /**
     * Processes the person element in the config file
     *
     * @param node node corresponding to the persons element in the xml file
     */
    void processWorkerPersonNode(xercesc::DOMElement* node);

    /**
     * Processes the signal element in the config file
     *
     * @param node node corresponding to the signal element in the xml file
     */
    void processWorkerSignalNode(xercesc::DOMElement* node);

    /**
     * Processes the intersection_mananger element in the config file
     *
     * @param node node corresponding to the intersection_mananger element in the xml file
     */
    void processWorkerIntMgrNode(xercesc::DOMElement* node);

    /**
     * Processes the communication element in the config file
     *
     * @param node node corresponding to the communication element in the xml file
     */
    void processWorkerCommunicationNode(xercesc::DOMElement* node);

    /**
     * Processes the personCharacteristics element in the config file
     *
     * @param node node corresponding to the personCharacteristics element in the xml file
     */
    void processPersonCharacteristicsNode(xercesc::DOMElement* node);

    /**
     * Processes the aura_manager_impl element in the config file
     *
     * @param node node corresponding to the aura_manager_impl element in the xml file
     */
    void processAuraManagerImpNode(xercesc::DOMElement* node);

    /**
	 * Helper function to process integer from nodes represented with "value" attribute
	 *
	 * @param node xml element to be processed
	 *
	 * @return extracted integer value
	 */
	int processValueInteger(xercesc::DOMElement* node);

	/**
	 * Helper function to process boolean from nodes represented with "value" attribute
	 *
	 * @param node xml element to be processed
	 *
	 * @return extracted boolean value
	 */
	bool processValueBoolean(xercesc::DOMElement* node);

    /**
     * Processes the commsim element in the config file
     *
     * @param node node corresponding to the commsim element in the xml file
     */
    void processCommSimNode(xercesc::DOMElement* node);

    /**
     * Processes the loadAgentOrder element in the config file
     *
     * @param node node corresponding to the loadAgentsOrder element in the xml file
     */
    void processLoadAgentsOrder(xercesc::DOMElement* node);

    /**
     * Processes the generic_props element in the config file
     *
     * @param node node corresponding to the generic_props element in the xml file
     */
    void processGenericPropsNode(xercesc::DOMElement* node);

    /**
     * Processes the vehicleTypes element in the config file
     *
     * @param node node corresponding to the vehicleTypes element in the xml file
     */
    void processVehicleTypesNode(xercesc::DOMElement* node);

    /**
     * Processes the tripFiles element in the config file
     *
     * @param node node corresponding to the tripFiles element in the config file
     */
    void processTripFilesNode(xercesc::DOMElement* node);

    /**
     * Processes the busController element in the config file
     *
     * @param node node corresponding to the busController element in the config file
     */
    void processBusControllerNode(xercesc::DOMElement* node);
	
	/**
     * Processes the bus capacity element in the config file
     *
     * @param node node corresponding to the bus capacity element in the config file
     */
	void processBusCapacityNode(xercesc::DOMElement* node);

	/**
     * Processes the publicTransit element in config xml
     *
     * @param node node corresponding to the publicTransit element inside xml file
     */
    void processPublicTransit(xercesc::DOMElement* node);
	
    /**
     * Processes the pathSet element in the config file
     *
     * @param node node corresponding to the pathSetFile element in the xml file
     */
    void processPathSetFileName(xercesc::DOMElement* node);
	
	/**
     * processes the travel time element the config xml
     *
     * @param node node corresponding to trave_time element inside xml file
     */
    void processTT_Update(xercesc::DOMElement* node);

    /**
     * processes the subtrip_travel_metrics_output element in config xml
     *
     * @param node node corresponding to subtrip_travel_metrics_output element inside xml file
     */
    void processSubtripTravelMetricsOutputNode(xercesc::DOMElement* node);

    /**
     * processes the assignment_matrix element in config xml
     *
     * @param node node corresponding to assingment_matrix element inside xml file
     */
    void processAssignmentMatrixNode(xercesc::DOMElement* node);

    /**
     * processes the OD Travel Time node in the config xml
     *
     * @param node node correspoding to od travel time element inside xml file
     */
    void processODTravelTimeNode(xercesc::DOMElement* node);

    /**
	 * processes the Segment Travel Time node in the config xml
	 *
	 * @param node node correspoding to segment travel time element inside xml file
	 */
    void processSegmentTravelTimeNode(xercesc::DOMElement* node);

    /// Short Term config reference
    ST_Config& stCfg;
	
	/// Shared configuration
    ConfigParams& cfg;
};

class ParseShortTermTripFile : public ParseConfigXmlBase, private boost::noncopyable
{
public:
    /**
     * ParseShortTermTripFile
     * @param tripFileName
     * @param stConfig
     */
    ParseShortTermTripFile(const std::string& tripFileName, const std::string& tripName, ST_Config& stConfig);

private:
    /**
     * Code for processing the xml
     *
     * @param configFileName is the filename of configuration
     */
    virtual void processXmlFile(xercesc::XercesDOMParser& parser);

    /**
     * Processes the trips element in the trips config file
     *
     * @param node node corresponding to the trips element in the xml file
     */
    void processTrips(xercesc::DOMElement* node);

    /// Short term config reference
    ST_Config& cfg;

    /// Trip name
    std::string tripName;
};

}
