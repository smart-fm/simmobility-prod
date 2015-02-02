//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/noncopyable.hpp>
#include <string>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include "conf/ParseConfigXmlBase.hpp"
#include "conf/RawConfigParams.hpp" //For EntityTemplate.
#include "util/DailyTime.hpp"

using namespace sim_mob;
using namespace xercesc;

namespace sim_mob {

/**
 * Class used to parse a config file into a RawConfigParams object.
 * Typically used like a verb:
 *     RawConfigParams& cfg = ConfigParams::getInstanceRW();
 *     ParseConfigFile print(cfg);
 *
 * \note
 * This class is actually USED by the old config format (simpleconf). Don't delete it if you are cleaning
 * up the remains of the new config format (which doesn't work at the moment). ~Seth
 */
class ParseConfigFile : public ParseConfigXmlBase, private boost::noncopyable {
public:
	///Parse a config file into RawConfigParams, performing all XML parsing and some trivial semantic processing.
	ParseConfigFile(const std::string& configFileName, RawConfigParams& result);

private:
	virtual void processXmlFile(xercesc::XercesDOMParser& parser);

	//These functions are called by parseXmlAndProcess()
	//Process through recursive descent.
	void ProcessSystemNode(xercesc::DOMElement* node);
	//void ProcessGeometryNode(xercesc::DOMElement* node);
	void ProcessConstructsNode(xercesc::DOMElement* node);
	void ProcessFMOD_Node(xercesc::DOMElement* node);
	void ProcessAMOD_Node(xercesc::DOMElement* node);
	void ProcessIncidentsNode(xercesc::DOMElement* node);
	void ProcessBusStopScheduledTimesNode(xercesc::DOMElement* node);
	void ProcessPersonCharacteristicsNode(xercesc::DOMElement* node);
	void ProcessDriversNode(xercesc::DOMElement* node);
	void ProcessTaxiDriversNode(xercesc::DOMElement* node);
	void ProcessPedestriansNode(xercesc::DOMElement* node);
	void ProcessBusDriversNode(xercesc::DOMElement* node);
	void ProcessPassengersNode(xercesc::DOMElement* node);
	void ProcessSignalsNode(xercesc::DOMElement* node);
	void ProcessBusControllersNode(xercesc::DOMElement* node);
	void ProcessCBD_Node(xercesc::DOMElement* node);
	void processPathSetFileName(xercesc::DOMElement* node);
	void ProcessLongTermParamsNode(xercesc::DOMElement* node);

	//Descend through Constructs
	void ProcessConstructDatabasesNode(xercesc::DOMElement* node);
	void ProcessConstructDbProcGroupsNode(xercesc::DOMElement* node);
	void ProcessConstructCredentialsNode(xercesc::DOMElement* node);

	//Descend through System
	void ProcessSystemSimulationNode(xercesc::DOMElement* node);
	void ProcessSystemWorkersNode(xercesc::DOMElement* node);
	void ProcessSystemSingleThreadedNode(xercesc::DOMElement* node);
	void ProcessSystemMergeLogFilesNode(xercesc::DOMElement* node);
	void ProcessSystemNetworkSourceNode(xercesc::DOMElement* node);
	void ProcessSystemNetworkXmlInputFileNode(xercesc::DOMElement* node);
	void ProcessSystemNetworkXmlOutputFileNode(xercesc::DOMElement* node);
	void ProcessSystemDatabaseNode(xercesc::DOMElement* node);
	void ProcessSystemXmlSchemaFilesNode(xercesc::DOMElement* node);
	void ProcessSystemGenericPropsNode(xercesc::DOMElement* node);

	//Descend through System/Simulation
	unsigned int ProcessTimegranUnits(xercesc::DOMElement* node); //This is reused in several places.
	bool ProcessValueBoolean(xercesc::DOMElement* node);
	int ProcessValueInteger(xercesc::DOMElement* node); //Represents nodes with "value" attributes.
	sim_mob::DailyTime ProcessValueDailyTime(xercesc::DOMElement* node);
	void ProcessSystemAuraManagerImplNode(xercesc::DOMElement* node);
	void ProcessSystemWorkgroupAssignmentNode(xercesc::DOMElement* node);
	void ProcessSystemLoadAgentsOrderNode(xercesc::DOMElement* node);
	void ProcessSystemMutexEnforcementNode(xercesc::DOMElement* node);
	void ProcessSystemCommsimNode(xercesc::DOMElement* node);

	//Descend through System/Workers
	void ProcessWorkerPersonNode(xercesc::DOMElement* node);
	void ProcessWorkerSignalNode(xercesc::DOMElement* node);
	void ProcessWorkerCommunicationNode(xercesc::DOMElement* node);

	//Dabase mappings/connection
	//void ProcessGeomDbConnection(xercesc::DOMElement* node);
	//void ProcessGeomDbMappings(xercesc::DOMElement* node);

	//All entities are added to a "pending" list in the same manner.
	void ProcessFutureAgentList(xercesc::DOMElement* node, const std::string& itemName, std::vector<sim_mob::EntityTemplate>& res, bool originReq=true, bool destReq=true, bool timeReq=true, bool laneReq=false);


private:
	//The config file we are currently loading
	RawConfigParams& cfg;
};

}
