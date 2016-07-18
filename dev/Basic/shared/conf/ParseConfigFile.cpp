//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParseConfigFile.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <xercesc/dom/DOM.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "conf/RawConfigParams.hpp"
#include "geospatial/network/Point.hpp"
#include "util/GeomHelpers.hpp"
#include "util/XmlParseHelper.hpp"
#include "path/ParsePathXmlConfig.hpp"

namespace {

WorkGroup::ASSIGNMENT_STRATEGY ParseWrkGrpAssignEnum(const XMLCh* srcX, WorkGroup::ASSIGNMENT_STRATEGY* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		if (src=="roundrobin") {
			return WorkGroup::ASSIGN_ROUNDROBIN;
		} else if (src=="smallest") {
			return WorkGroup::ASSIGN_SMALLEST;
		}
		throw std::runtime_error("Expected WorkGroup::ASSIGNMENT_STRATEGY value.");
	}

    ///Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory WorkGroup::ASSIGNMENT_STRATEGY variable; no default available.");
	}
	return *defValue;
}

MutexStrategy ParseMutexStrategyEnum(const XMLCh* srcX, MutexStrategy* defValue) {
	if (srcX) {
		std::string src = TranscodeString(srcX);
		if (src=="buffered") {
			return MtxStrat_Buffered;
		} else if (src=="locked") {
			return MtxStrat_Locked;
		}
		throw std::runtime_error("Expected sim_mob::MutexStrategy value.");
	}

    ///Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory sim_mob::MutexStrategy variable; no default available.");
	}
	return *defValue;
}

unsigned int ParseGranularitySingle(const XMLCh* srcX, unsigned int* defValue) {
	if (srcX) {
        ///Search for "[0-9]+ ?[^0-9]+), roughly.
		std::string src = TranscodeString(srcX);
		size_t digStart = src.find_first_of("1234567890");
		size_t digEnd = src.find_first_not_of("1234567890", digStart+1);
		size_t unitStart = src.find_first_not_of(" ", digEnd);
		if (digStart!=0 || digStart==std::string::npos || digEnd==std::string::npos || unitStart==std::string::npos) {
			throw std::runtime_error("Badly formatted single-granularity string.");
		}

        ///Now split/parse it.
		double value = boost::lexical_cast<double>(src.substr(digStart, (digEnd-digStart)));
		std::string units = src.substr(unitStart, std::string::npos);

		return GetValueInMs(value, units, defValue);
	}

    ///Wasn't found.
	if (!defValue) {
		throw std::runtime_error("Mandatory integer (granularity) variable; no default available.");
	}
	return *defValue;
}

unsigned int ParseGranularitySingle(const XMLCh* src, unsigned int defValue) {
	return ParseGranularitySingle(src, &defValue);
}
unsigned int ParseGranularitySingle(const XMLCh* src) { ///No default
	return ParseGranularitySingle(src, nullptr);
}

WorkGroup::ASSIGNMENT_STRATEGY ParseWrkGrpAssignEnum(const XMLCh* srcX, WorkGroup::ASSIGNMENT_STRATEGY defValue) {
	return ParseWrkGrpAssignEnum(srcX, &defValue);
}
WorkGroup::ASSIGNMENT_STRATEGY ParseWrkGrpAssignEnum(const XMLCh* srcX) {
	return ParseWrkGrpAssignEnum(srcX, nullptr);
}
MutexStrategy ParseMutexStrategyEnum(const XMLCh* srcX, MutexStrategy defValue) {
	return ParseMutexStrategyEnum(srcX, &defValue);
}
MutexStrategy ParseMutexStrategyEnum(const XMLCh* srcX) {
	return ParseMutexStrategyEnum(srcX, nullptr);
}

const double MILLISECONDS_IN_SECOND = 1000.0;
} ///End un-named namespace


sim_mob::ParseConfigFile::ParseConfigFile(const std::string& configFileName, RawConfigParams& result, bool longTerm) : cfg(result), ParseConfigXmlBase(configFileName), longTerm(longTerm)
{
	parseXmlAndProcess();
}

void sim_mob::ParseConfigFile::processXmlFile(XercesDOMParser& parser)
{
    ///Verify that the root node is "config"
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	if (TranscodeString(rootNode->getTagName()) != "config")
	{
		throw std::runtime_error("xml parse error: root node must be \"config\"");
	}

    ///Make sure we don't have a geometry node.
	DOMElement* geom = GetSingleElementByName(rootNode, "geometry");
	if (geom)
	{
		throw std::runtime_error("Config file contains a <geometry> node, which is no longer allowed. See the <constructs> node for documentation.");
	}

	if( longTerm )
	{
        processConstructsNode(GetSingleElementByName(rootNode,"constructs"));
        processLongTermParamsNode( GetSingleElementByName(rootNode, "longTermParams"));
        processModelScriptsNode(GetSingleElementByName(rootNode, "model_scripts"));
		return;
	}

    ///Now just parse the document recursively.
    processSimulationNode(GetSingleElementByName(rootNode, "simulation", true));
    processMergeLogFilesNode(GetSingleElementByName(rootNode, "merge_log_files"));
    processGenericPropsNode(GetSingleElementByName(rootNode, "generic_props"));
    processConstructsNode(GetSingleElementByName(rootNode,"constructs"));
}

void sim_mob::ParseConfigFile::processConstructsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

    ///Process each item in order.
    processConstructDatabaseNode(GetSingleElementByName(node, "databases"));
    processConstructCredentialNode(GetSingleElementByName(node, "credentials"));
}


void sim_mob::ParseConfigFile::processConstructDatabaseNode(xercesc::DOMElement* node)
{
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		if (TranscodeString(item->getNodeName())!="database") {
			Warn() <<"Invalid databases child node.\n";
			continue;
		}

        ///Retrieve some attributes from the Node itself.
		Database db(ParseString(GetNamedAttributeValue(item, "id")));
		std::string dbType = ParseString(GetNamedAttributeValue(item, "dbtype"), "");
		if (dbType != "postgres" && dbType != "mongodb") {
			throw std::runtime_error("Database type not supported.");
		}

        ///Now retrieve the required parameters from child nodes.
		db.host = ProcessValueString(GetSingleElementByName(item, "host"));
		db.port = ProcessValueString(GetSingleElementByName(item, "port"));
		db.dbName = ProcessValueString(GetSingleElementByName(item, "dbname"));

		cfg.constructs.databases[db.getId()] = db;
	}
}

void sim_mob::ParseConfigFile::processConstructCredentialNode(xercesc::DOMElement* node)
{
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
		std::string name = TranscodeString(item->getNodeName());
		if (name!="file-based-credential" && name!="plaintext-credential") {
			Warn() <<"Invalid db_proc_groups child node.\n";
			continue;
		}

        ///Retrieve some attributes from the Node itself.
		Credential cred(ParseString(GetNamedAttributeValue(item, "id")));

        ///Setting the actual credentials depends on the type of node.
		if (name=="file-based-credential") {
            ///Scan children for "path" nodes.
			std::vector<std::string> paths;
			for (DOMElement* pathItem=item->getFirstElementChild(); pathItem; pathItem=pathItem->getNextElementSibling()) {
				if (TranscodeString(pathItem->getNodeName())!="file") {
					Warn() <<"file-based credentials contain invalid child node; expected path.\n";
					continue;
				}
				std::string path = ParseString(GetNamedAttributeValue(pathItem, "path"), "");
				if (!path.empty()) {
					paths.push_back(path);
				}
			}

            ///Try setting it.
			cred.LoadFileCredentials(paths);
		} else if (name=="plaintext-credential") {
            ///Retrieve children manually.
			std::string username = ParseString(GetNamedAttributeValue(GetSingleElementByName(item, "username"), "value"), "");
			std::string password = ParseString(GetNamedAttributeValue(GetSingleElementByName(item, "password"), "value"), "");
			cred.SetPlaintextCredentials(username, password);
		} else {
			throw std::runtime_error("Unexpected (but allowed) credentials.");
		}

        ///Save it.
		cfg.constructs.credentials[cred.getId()] = cred;
	}
}

void sim_mob::ParseConfigFile::processLongTermParamsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

    ///The longtermParams tag has an attribute
	cfg.ltParams.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	if (!cfg.ltParams.enabled)
	{
		return;
	}

    ///Now set the rest.
	cfg.ltParams.days 				 = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "days"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.maxIterations 		 = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "maxIterations"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.tickStep 			 = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "tickStep"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.workers 			 = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "workers"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.year 				 = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "year"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.simulationScenario  = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "simulationScenario"), "value"), static_cast<std::string>(""));
	cfg.ltParams.resume              = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName( node, "resume"), "value"), false );
	cfg.ltParams.currentOutputSchema = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "currentOutputSchema"), "value"), static_cast<std::string>(""));
	cfg.ltParams.mainSchemaVersion = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "mainSchemaVersion"), "value"), static_cast<std::string>(""));
	cfg.ltParams.configSchemaVersion = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "configSchemaVersion"), "value"), static_cast<std::string>(""));
	cfg.ltParams.calibrationSchemaVersion = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "calibrationSchemaVersion"), "value"), static_cast<std::string>(""));
	cfg.ltParams.geometrySchemaVersion = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "geometrySchemaVersion"), "value"), static_cast<std::string>(""));
	cfg.ltParams.opSchemaloadingInterval = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "opSchemaloadingInterval"), "value"), static_cast<unsigned int>(0));

	LongTermParams::DeveloperModel developerModel;
	developerModel.enabled = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName( node, "developerModel"), "enabled"), false );
	developerModel.timeInterval = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "developerModel"), "timeInterval"), "value"), static_cast<unsigned int>(0));
	developerModel.initialPostcode = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "developerModel"), "InitialPostcode"), "value"), static_cast<int>(0));
	developerModel.initialUnitId = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "developerModel"), "initialUnitId"), "value"), static_cast<int>(0));
	developerModel.initialBuildingId = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "developerModel"), "initialBuildingId"), "value"), static_cast<int>(0));
	developerModel.initialProjectId = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "developerModel"), "initialProjectId"), "value"), static_cast<int>(0));
	developerModel.minLotSize = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(node, "minLotSize"), "value"));
	cfg.ltParams.developerModel = developerModel;


	LongTermParams::HousingModel housingModel;
	housingModel.enabled = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName( node, "housingModel"), "enabled"), false);
	housingModel.timeInterval = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "timeInterval"), "value"), static_cast<unsigned int>(0));
	housingModel.timeOnMarket = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "timeOnMarket"), "value"), static_cast<unsigned int>(0));
	housingModel.timeOffMarket = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "timeOffMarket"), "value"), static_cast<unsigned int>(0));
	housingModel.vacantUnitActivationProbability = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(node, "vacantUnitActivationProbability"), "value"));
	housingModel.initialHouseholdsOnMarket = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "InitialHouseholdsOnMarket"), "value"), static_cast<int>(0));
	housingModel.housingMarketSearchPercentage = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "housingMarketSearchPercentage"), "value"));
	housingModel.housingMoveInDaysInterval = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "housingMoveInDaysInterval"), "value"));
	housingModel.offsetBetweenUnitBuyingAndSelling = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "offsetBetweenUnitBuyingAndSelling"), "value"), static_cast<int>(0));
	housingModel.bidderUnitsChoiceSet = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "bidderUnitsChoiceSet"), "value"), static_cast<int>(0));
	housingModel.bidderBTOUnitsChoiceSet = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "bidderBTOUnitsChoiceSet"), "value"), static_cast<int>(0));
	housingModel.householdBiddingWindow = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "householdBiddingWindow"), "value"), static_cast<int>(0));
	housingModel.dailyHouseholdAwakenings = ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "dailyHouseholdAwakenings"), "value"), static_cast<int>(0));
	housingModel.householdAwakeningPercentageByBTO = ParseFloat(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "housingModel"), "householdAwakeningPercentageByBTO"), "value"));
	cfg.ltParams.housingModel = housingModel;

	LongTermParams::OutputHouseholdLogsums outputHouseholdLogsums;
	outputHouseholdLogsums.enabled = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName( node, "outputHouseholdLogsums"), "enabled"), false);
	outputHouseholdLogsums.fixedHomeVariableWork = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "outputHouseholdLogsums"), "fixedHomeVariableWork"), "value"), false);
	outputHouseholdLogsums.fixedWorkVariableHome = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "outputHouseholdLogsums"), "fixedWorkVariableHome"), "value"), false);
	cfg.ltParams.outputHouseholdLogsums = outputHouseholdLogsums;

	LongTermParams::VehicleOwnershipModel vehicleOwnershipModel;
	vehicleOwnershipModel.enabled = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName( node, "vehicleOwnershipModel"), "enabled"), false);
	vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "vehicleOwnershipModel"), "vehicleBuyingWaitingTimeInDays"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.vehicleOwnershipModel = vehicleOwnershipModel;

	LongTermParams::TaxiAccessModel taxiAccessModel;
	taxiAccessModel.enabled = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName( node, "taxiAccessModel"), "enabled"), false);
	cfg.ltParams.vehicleOwnershipModel = vehicleOwnershipModel;

	LongTermParams::SchoolAssignmentModel schoolAssignmentModel;
	schoolAssignmentModel.enabled = ParseBoolean(GetNamedAttributeValue(GetSingleElementByName( node, "schoolAssignmentModel"), "enabled"), false);
	schoolAssignmentModel.schoolChangeWaitingTimeInDays = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName( node, "schoolAssignmentModel"), "schoolChangeWaitingTimeInDays"), "value"), static_cast<unsigned int>(0));
	cfg.ltParams.taxiAccessModel = taxiAccessModel;
}

void sim_mob::ParseConfigFile::processSimulationNode(xercesc::DOMElement* node)
{
    ///Several properties are set up as "x ms", or "x seconds", etc.
    cfg.simulation.baseGranMS = processTimegranUnits(GetSingleElementByName(node, "base_granularity", true));
    cfg.simulation.baseGranSecond = cfg.simulation.baseGranMS / MILLISECONDS_IN_SECOND;
    cfg.simulation.totalRuntimeMS = processTimegranUnits(GetSingleElementByName(node, "total_runtime", true));
    cfg.simulation.totalWarmupMS = processTimegranUnits(GetSingleElementByName(node, "total_warmup"));

    cfg.simulation.simStartTime = processValueDailyTime(GetSingleElementByName(node, "start_time", true));
	cfg.simulation.inSimulationTTUsage = processInSimulationTTUsage(GetSingleElementByName(node, "in_simulation_travel_time_usage", true));

    ///Now we're getting back to real properties.
    processWorkgroupAssignmentNode(GetSingleElementByName(node, "workgroup_assignment"));
    cfg.simulation.startingAutoAgentID = ProcessValueInteger2(GetSingleElementByName(node, "auto_id_start"), 0);
    processMutexEnforcementNode(GetSingleElementByName(node, "mutex_enforcement"));
}

void sim_mob::ParseConfigFile::processMergeLogFilesNode(xercesc::DOMElement* node)
{
	cfg.mergeLogFiles = ParseBoolean(GetNamedAttributeValue(node, "value"), false);
}


void sim_mob::ParseConfigFile::processGenericPropsNode(xercesc::DOMElement* node)
{
	if (!node) {
		return;
	}

	std::vector<DOMElement*> properties = GetElementsByName(node, "property");
	for (std::vector<DOMElement*>::const_iterator it=properties.begin(); it!=properties.end(); ++it) {
		std::string key = ParseString(GetNamedAttributeValue(*it, "key"), "");
		std::string val = ParseString(GetNamedAttributeValue(*it, "value"), "");
		if (!(key.empty() && val.empty())) {
			cfg.genericProps[key] = val;
		}
	}
}

unsigned int sim_mob::ParseConfigFile::processTimegranUnits(xercesc::DOMElement* node)
{
	return ParseTimegranAsMs(GetNamedAttributeValue(node, "value"), GetNamedAttributeValue(node, "units"));
}

bool sim_mob::ParseConfigFile::processValueBoolean(xercesc::DOMElement* node)
{
	return ParseBoolean(GetNamedAttributeValue(node, "value"));
}

int sim_mob::ParseConfigFile::processValueInteger(xercesc::DOMElement* node)
{
	return ParseInteger(GetNamedAttributeValue(node, "value"));
}

DailyTime sim_mob::ParseConfigFile::processValueDailyTime(xercesc::DOMElement* node)
{
	return ParseDailyTime(GetNamedAttributeValue(node, "value"));
}

unsigned int sim_mob::ParseConfigFile::processInSimulationTTUsage(xercesc::DOMElement* node)
{
	unsigned int percentage = ParseUnsignedInt(GetNamedAttributeValue(node, "value"));
	
	if(percentage > 100)
	{
		percentage = 100;
	}
	
	return percentage;
}

void sim_mob::ParseConfigFile::processWorkgroupAssignmentNode(xercesc::DOMElement* node)
{
    cfg.simulation.workGroupAssigmentStrategy = ParseWrkGrpAssignEnum(GetNamedAttributeValue(node, "value"), WorkGroup::ASSIGN_SMALLEST);
}

void sim_mob::ParseConfigFile::processMutexEnforcementNode(xercesc::DOMElement* node)
{
    cfg.simulation.mutexStategy = ParseMutexStrategyEnum(GetNamedAttributeValue(node, "strategy"), MtxStrat_Buffered);
}

void sim_mob::ParseConfigFile::processModelScriptsNode(xercesc::DOMElement* node)
{
	std::string format = ParseString(GetNamedAttributeValue(node, "format"), "");
	if (format.empty() || format != "lua")
	{
		throw std::runtime_error("Unsupported script format");
	}

	std::string scriptsDirectoryPath = ParseString(GetNamedAttributeValue(node, "path"), "");
	if (scriptsDirectoryPath.empty())
	{
		throw std::runtime_error("path to scripts is not provided");
	}
	if ((*scriptsDirectoryPath.rbegin()) != '/')
	{
        ///add a / to the end of the path string if it is not already there
		scriptsDirectoryPath.push_back('/');
	}
	ModelScriptsMap scriptsMap(scriptsDirectoryPath, format);
	for (DOMElement* item = node->getFirstElementChild(); item; item = item->getNextElementSibling())
	{
		std::string name = TranscodeString(item->getNodeName());
		if (name != "script")
		{
			Warn() << "Invalid db_proc_groups child node.\n";
			continue;
		}

		std::string key = ParseString(GetNamedAttributeValue(item, "name"), "");
		std::string val = ParseString(GetNamedAttributeValue(item, "file"), "");
		if (key.empty() || val.empty())
		{
			Warn() << "Invalid script; missing \"name\" or \"file\".\n";
			continue;
		}

		scriptsMap.addScriptFileName(key, val);
	}
	cfg.luaScriptsMap = scriptsMap;
}

