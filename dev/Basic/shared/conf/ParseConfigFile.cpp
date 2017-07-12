//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParseConfigFile.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <xercesc/dom/DOM.hpp>

#include "conf/ConfigManager.hpp"
#include "util/GeomHelpers.hpp"
#include "util/XmlParseHelper.hpp"

using namespace std;
using namespace sim_mob;

namespace
{

WorkGroup::ASSIGNMENT_STRATEGY ParseWrkGrpAssignEnum(const XMLCh *srcX, WorkGroup::ASSIGNMENT_STRATEGY defValue)
{
	if (srcX)
	{
		string src = TranscodeString(srcX);

		if (src == "roundrobin")
		{
			return WorkGroup::ASSIGN_ROUNDROBIN;
		}
		else if (src == "smallest")
		{
			return WorkGroup::ASSIGN_SMALLEST;
		}

		stringstream msg;
		msg << "Invalid value for \'workgroup_assignment\': \"" << src
		    << "\". Expected: \"roundrobin\" or \"smallest\"";
		throw runtime_error(msg.str());
	}

	return defValue;
}

MutexStrategy ParseMutexStrategyEnum(const XMLCh *srcX, MutexStrategy defValue)
{
	if (srcX)
	{
		string src = TranscodeString(srcX);

		if (src == "buffered")
		{
			return MtxStrat_Buffered;
		}
		else if (src == "locked")
		{
			return MtxStrat_Locked;
		}

		stringstream msg;
		msg << "Invalid value for \'mutex_enforcement\': \"" << src
		    << "\". Expected: \"buffered\" or \"locked\"";
		throw runtime_error(msg.str());
	}

	return defValue;
}

const double MILLISECONDS_IN_SECOND = 1000.0;

} //End un-named namespace


ParseConfigFile::ParseConfigFile(const string &configFileName, RawConfigParams &result, bool longTerm)
		: cfg(result), ParseConfigXmlBase(configFileName), longTerm(longTerm)
{
	parseXmlAndProcess();
}

void ParseConfigFile::processXmlFile(XercesDOMParser &parser)
{
	//Verify that the root node is "config"
	DOMElement *rootNode = parser.getDocument()->getDocumentElement();

	if (TranscodeString(rootNode->getTagName()) != "config")
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". Root node must be \'config\'";
		throw runtime_error(msg.str());
	}

	try
	{
		processConstructsNode(GetSingleElementByName(rootNode, "constructs"));

		if (longTerm)
		{
			processSchemasParamsNode(GetSingleElementByName(rootNode, "schemas"));
			processLongTermParamsNode(GetSingleElementByName(rootNode, "longTermParams"));
			processModelScriptsNode(GetSingleElementByName(rootNode, "model_scripts"));
			return;
		}

		///Now just parse the document recursively.
		processSimulationNode(GetSingleElementByName(rootNode, "simulation", true));
		processGenericPropsNode(GetSingleElementByName(rootNode, "generic_props"));
		processMergeLogFilesNode(GetSingleElementByName(rootNode, "merge_log_files"));
	}
	catch (runtime_error &ex)
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". " << ex.what();
		throw runtime_error(msg.str());
	}
}

void ParseConfigFile::processConstructsNode(xercesc::DOMElement *node)
{
	if (!node)
	{
		return;
	}

	///Process each item in order.
	processConstructDatabaseNode(GetSingleElementByName(node, "databases"));
	processConstructCredentialNode(GetSingleElementByName(node, "credentials"));
}


void ParseConfigFile::processConstructDatabaseNode(xercesc::DOMElement *node)
{
	for (DOMElement *item = node->getFirstElementChild(); item; item = item->getNextElementSibling())
	{
		if (TranscodeString(item->getNodeName()) != "database")
		{
			Warn() << "\nWARNING! Invalid value for \'databases\': \"" << TranscodeString(item->getNodeName())
			       << "\" in file " << inFilePath << ". Expected: \'database\'\n";
			continue;
		}

		//Retrieve some attributes from the Node itself.
		Database db(ParseString(GetNamedAttributeValue(item, "id")));
		string dbType = ParseString(GetNamedAttributeValue(item, "dbtype"), "");

		if (dbType != "postgres")
		{
			stringstream msg;
			msg << "Invalid value for \'dbType\'"
			    << " in <database id=\"" << db.getId()
			    << "\" dbtype=\"" << dbType << "\">. Expected: \"postgres\"";
			throw runtime_error(msg.str());
		}

		///Now retrieve the required parameters from child nodes.
		db.host = ProcessValueString(GetSingleElementByName(item, "host"));
		db.port = ProcessValueString(GetSingleElementByName(item, "port"));
		db.dbName = ProcessValueString(GetSingleElementByName(item, "dbname"));

		cfg.constructs.databases[db.getId()] = db;
	}
}

void ParseConfigFile::processConstructCredentialNode(xercesc::DOMElement *node)
{
	for (DOMElement *item = node->getFirstElementChild(); item; item = item->getNextElementSibling())
	{
		string name = TranscodeString(item->getNodeName());
		if (name != "file-based-credential" && name != "plaintext-credential")
		{
			Warn() << "\nWARNING! Invalid value for \'db_proc_groups\': \"" << TranscodeString(item->getNodeName())
			       << "\" in file " << inFilePath
			       << ". Expected: \"file-based-credential\" or \"plaintext-credential\"\n";
			continue;
		}

		//Retrieve some attributes from the Node itself.
		Credential credential(ParseString(GetNamedAttributeValue(item, "id")));

		//Setting the actual credentials depends on the type of node.
		if (name == "file-based-credential")
		{
			//Scan children for "path" nodes.
			vector<string> paths;

			for (DOMElement *pathItem = item->getFirstElementChild(); pathItem; pathItem = pathItem->getNextElementSibling())
			{
				if (TranscodeString(pathItem->getNodeName()) != "file")
				{
					Warn() << "\nWARNING! Invalid value for \'file-based credential\': \""
					       << TranscodeString(pathItem->getNodeName()) << "\" in file " << inFilePath
					       << ". Expected: \"file\"\n";
					continue;
				}

				string path = ParseString(GetNamedAttributeValue(pathItem, "path"), "");

				if (!path.empty())
				{
					paths.push_back(path);
				}
			}

			//Try setting it.
			credential.LoadFileCredentials(paths);
		}
		else if (name == "plaintext-credential")
		{
			//Retrieve children manually.
			string username = ParseString(GetNamedAttributeValue(GetSingleElementByName(item, "username"), "value"), "");
			string password = ParseString(GetNamedAttributeValue(GetSingleElementByName(item, "password"), "value"), "");

			credential.SetPlaintextCredentials(username, password);
		}

		//Save it.
		cfg.constructs.credentials[credential.getId()] = credential;
	}
}

void ParseConfigFile::processSchemasParamsNode(xercesc::DOMElement *node)
{
	if (!node)
	{
		return;
	}

	//The schemaParams tag has an attribute
	cfg.schemas.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	if (!cfg.schemas.enabled)
	{
		return;
	}

	cfg.schemas.main_schema =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "main_schema"), "value"));

	cfg.schemas.calibration_schema =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "calibration_schema"), "value"));

	cfg.schemas.public_schema =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "public_schema"), "value"));

	cfg.schemas.demand_schema =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "demand_schema"), "value"));
}


void ParseConfigFile::processLongTermParamsNode(xercesc::DOMElement *node)
{
	if (!node)
	{
		return;
	}

	//The longTermParams tag has an attribute
	cfg.ltParams.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	if (!cfg.ltParams.enabled)
	{
		return;
	}

	//Now set the rest.
	cfg.ltParams.days =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					node, "days"), "value"), (unsigned int) 0);

	cfg.ltParams.maxIterations =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					node, "maxIterations"), "value"), (unsigned int) 0);

	cfg.ltParams.tickStep =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					node, "tickStep"), "value"), (unsigned int) 0);

	cfg.ltParams.workers =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					node, "workers"), "value"), (unsigned int) 0);

	cfg.ltParams.year =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					node, "year"), "value"), (unsigned int) 0);

	cfg.ltParams.resume =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					node, "resume"), "value"), false);

	cfg.ltParams.currentOutputSchema =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(
					node, "currentOutputSchema"), "value"), "");

	cfg.ltParams.mainSchemaVersion =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(
					node, "mainSchemaVersion"), "value"), "");

	cfg.ltParams.configSchemaVersion =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(
					node, "configSchemaVersion"), "value"), "");

	cfg.ltParams.calibrationSchemaVersion =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(
					node, "calibrationSchemaVersion"), "value"), "");

	cfg.ltParams.geometrySchemaVersion =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(
					node, "geometrySchemaVersion"), "value"), "");

	cfg.ltParams.opSchemaloadingInterval =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					node, "opSchemaloadingInterval"), "value"), (unsigned int) 0);

	cfg.ltParams.initialLoading =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					node, "initialLoading"), "value"), false);

	processDeveloperModelNode(GetSingleElementByName(node, "developerModel"));
	processHousingModelNode(GetSingleElementByName(node, "housingModel"));
	processHouseHoldLogsumsNode(GetSingleElementByName(node, "outputHouseholdLogsums"));
	processVehicleOwnershipModelNode(GetSingleElementByName(node, "vehicleOwnershipModel"));

	LongTermParams::TaxiAccessModel taxiAccessModel;

	taxiAccessModel.enabled =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(node, "taxiAccessModel"), "enabled"), false);

	cfg.ltParams.taxiAccessModel = taxiAccessModel;

	processSchoolAssignmentModelNode(GetSingleElementByName(node, "schoolAssignmentModel"));
	processScenarioNode(GetSingleElementByName(node, "scenario"));
	processOutputFilesNode(GetSingleElementByName(node, "outputFiles"));

	LongTermParams::Scenario scenario;
	DOMElement *scenarioNode = GetSingleElementByName(node, "scenario");

	scenario.enabled =
			ParseBoolean(GetNamedAttributeValue(scenarioNode, "enabled"), false);

	scenario.scenarioName =
				ParseString(GetNamedAttributeValue(GetSingleElementByName(
						scenarioNode, "name"), "value"), "");

	scenario.parcelsTable =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(
					scenarioNode, "parcelsTable"), "value"), "");

	scenario.scenarioSchema =
				ParseString(GetNamedAttributeValue(GetSingleElementByName(
						scenarioNode, "scenarioSchema"), "value"), "");

	cfg.ltParams.scenario = scenario;
}

void ParseConfigFile::processOutputFilesNode(xercesc::DOMElement *output)
{
	LongTermParams::OutputFiles outputFiles;

	outputFiles.enabled =
			ParseBoolean(GetNamedAttributeValue(output, "enabled"), false);

	outputFiles.bids =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "bids"), "value"), false);

	outputFiles.expectations =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "expectations"), "value"), false);

	outputFiles.parcels =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "parcels"), "value"), false);

	outputFiles.units =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "units"), "value"), false);

	outputFiles.projects =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "projects"), "value"), false);

	outputFiles.hh_pc =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "hh_pc"), "value"), false);

	outputFiles.units_in_market =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "units_in_market"), "value"), false);

	outputFiles.log_taxi_availability =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_taxi_availability"), "value"), false);

	outputFiles.log_vehicle_ownership =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_vehicle_ownership"), "value"), false);

	outputFiles.log_taz_level_logsum =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_taz_level_logsum"), "value"), false);

	outputFiles.log_householdgrouplogsum =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_householdgrouplogsum"), "value"), false);

	outputFiles.log_individual_hits_logsum =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_individual_hits_logsum"), "value"), false);

	outputFiles.log_householdbidlist =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_householdbidlist"), "value"), false);

	outputFiles.log_individual_logsum_vo =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_individual_logsum_vo"), "value"), false);

	outputFiles.log_screeningprobabilities =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_screeningprobabilities"), "value"), false);

	outputFiles.log_hhchoiceset =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_hhchoiceset"), "value"), false);

	outputFiles.log_error =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_error"), "value"), false);

	outputFiles.log_school_assignment =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_school_assignment"), "value"), false);

	outputFiles.log_pre_school_assignment =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_pre_school_assignment"), "value"), false);

	outputFiles.log_hh_awakening =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_hh_awakening"), "value"), false);

	outputFiles.log_hh_exit =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_hh_exit"), "value"), false);

	outputFiles.log_random_nums =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_random_nums"), "value"), false);

	outputFiles.log_dev_roi =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_dev_roi"), "value"), false);

	outputFiles.log_household_statistics =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_household_statistics"), "value"), false);

	outputFiles.log_out_xx_files =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					output, "log_out_xx_files"), "value"), true);

	cfg.ltParams.outputFiles = outputFiles;
}

void ParseConfigFile::processScenarioNode(xercesc::DOMElement *scenarioNode)
{
	LongTermParams::Scenario scenario;

	scenario.enabled =
			ParseBoolean(GetNamedAttributeValue(scenarioNode, "enabled"), false);

	scenario.scenarioName =
			ParseString(GetNamedAttributeValue(GetSingleElementByName(scenarioNode, "name"), "value"), "");

	cfg.ltParams.scenario = scenario;
}

void ParseConfigFile::processSchoolAssignmentModelNode(xercesc::DOMElement *schoolAssignModel)
{
	LongTermParams::SchoolAssignmentModel schoolAssignmentModel;

	schoolAssignmentModel.enabled =
			ParseBoolean(GetNamedAttributeValue(schoolAssignModel, "enabled"), false);

	schoolAssignmentModel.schoolChangeWaitingTimeInDays =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					schoolAssignModel, "schoolChangeWaitingTimeInDays"), "value"), (unsigned int) 0);

	cfg.ltParams.schoolAssignmentModel = schoolAssignmentModel;
}

void ParseConfigFile::processVehicleOwnershipModelNode(xercesc::DOMElement *vehOwnModel)
{
	LongTermParams::VehicleOwnershipModel vehicleOwnershipModel;

	vehicleOwnershipModel.enabled =
			ParseBoolean(GetNamedAttributeValue(vehOwnModel, "enabled"), false);

	vehicleOwnershipModel.vehicleBuyingWaitingTimeInDays =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					vehOwnModel, "vehicleBuyingWaitingTimeInDays"), "value"), (unsigned int) 0);

	cfg.ltParams.vehicleOwnershipModel = vehicleOwnershipModel;
}

void ParseConfigFile::processHouseHoldLogsumsNode(xercesc::DOMElement *outHHLogsums)
{
	LongTermParams::OutputHouseholdLogsums outputHouseholdLogsums;

	outputHouseholdLogsums.enabled =
			ParseBoolean(GetNamedAttributeValue(outHHLogsums, "enabled"), false);

	outputHouseholdLogsums.fixedHomeVariableWork =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					outHHLogsums, "fixedHomeVariableWork"), "value"), false);

	outputHouseholdLogsums.fixedWorkVariableHome =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					outHHLogsums, "fixedWorkVariableHome"), "value"), false);

	outputHouseholdLogsums.vehicleOwnership =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(
					outHHLogsums, "vehicleOwnershipLogsum"), "value"), false);

	cfg.ltParams.outputHouseholdLogsums = outputHouseholdLogsums;
}

void ParseConfigFile::processHousingModelNode(xercesc::DOMElement *houseModel)
{
	LongTermParams::HousingModel housingModel;

	housingModel.enabled =
			ParseBoolean(GetNamedAttributeValue(houseModel, "enabled"), false);

	housingModel.timeInterval =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "timeInterval"), "value"), (unsigned int) 0);

	housingModel.timeOnMarket =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "timeOnMarket"),"value"), (unsigned int) 0);

	housingModel.timeOffMarket =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "timeOffMarket"), "value"), (unsigned int) 0);

	housingModel.vacantUnitActivationProbability =
			ParseFloat(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "vacantUnitActivationProbability"), "value"), (float) 0.0);

	housingModel.housingMarketSearchPercentage =
			ParseFloat(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "housingMarketSearchPercentage"), "value"), (float) 0.0);

	housingModel.housingMoveInDaysInterval =
			ParseFloat(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "housingMoveInDaysInterval"), "value"), (float) 0.0);

	housingModel.offsetBetweenUnitBuyingAndSelling =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "offsetBetweenUnitBuyingAndSelling"), "value"), (int) 0);

	housingModel.bidderUnitsChoiceSet =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "bidderUnitsChoiceSet"), "value"), (int) 0);

	housingModel.bidderBTOUnitsChoiceSet =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "bidderBTOUnitsChoiceSet"), "value"), (int) 0);

	housingModel.householdBiddingWindow =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "householdBiddingWindow"), "value"), (int) 0);

	housingModel.householdBTOBiddingWindow =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "householdBTOBiddingWindow"), "value"), (int) 0);

	housingModel.householdAwakeningPercentageByBTO =
			ParseFloat(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "householdAwakeningPercentageByBTO"),"value"), (float) 0.0);

	housingModel.awakeningModel.initialHouseholdsOnMarket =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "awakeningModel"), "initialHouseholdsOnMarket"), "value"), (int) 0);

	housingModel.awakeningModel.dailyHouseholdAwakenings =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "awakeningModel"), "dailyHouseholdAwakenings"), "value"), (int) 0);

	housingModel.awakeningModel.awakenModelRandom =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "awakeningModel"), "awakenModelRandom"), "value"), false);

	housingModel.awakeningModel.awakenModelShan =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "awakeningModel"), "awakenModelShan"), "value"), false);

	housingModel.awakeningModel.awakenModelJingsi =
			ParseBoolean(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "awakeningModel"), "awakenModelJingsi"), "value"), false);

	housingModel.offsetBetweenUnitBuyingAndSellingAdvancedPurchase =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					houseModel, "offsetBetweenUnitBuyingAndSellingAdvancedPurchase"), "value"), (int) 0);

	housingModel.awakeningModel.awakeningOffMarketSuccessfulBid =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "awakeningModel"), "awakeningOffMarketSuccessfulBid"), "value"), (int) 0);

	housingModel.awakeningModel.awakeningOffMarketUnsuccessfulBid =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "awakeningModel"), "awakeningOffMarketUnsuccessfulBid"), "value"), (int) 0);

	housingModel.hedonicPriceModel.a =
			ParseFloat(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "hedonicPriceModel"), "a"), "value"), (float) 0);

	housingModel.hedonicPriceModel.b =
			ParseFloat(GetNamedAttributeValue(GetSingleElementByName(GetSingleElementByName(
					houseModel, "hedonicPriceModel"), "b"), "value"), (float) 0);

	cfg.ltParams.housingModel = housingModel;
}

void ParseConfigFile::processDeveloperModelNode(xercesc::DOMElement *devModel)
{
	LongTermParams::DeveloperModel developerModel;

	developerModel.enabled =
			ParseBoolean(GetNamedAttributeValue(devModel, "enabled"), false);

	developerModel.timeInterval =
			ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
					devModel, "timeInterval"), "value"), (unsigned int) 0);

	developerModel.initialPostcode =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					devModel, "InitialPostcode"), "value"), (int) 0);

	developerModel.initialUnitId =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					devModel, "initialUnitId"), "value"), (int) 0);

	developerModel.initialBuildingId =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					devModel, "initialBuildingId"), "value"), (int) 0);

	developerModel.initialProjectId =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(
					devModel, "initialProjectId"), "value"), (int) 0);

	developerModel.minLotSize =
			ParseFloat(GetNamedAttributeValue(GetSingleElementByName(
					devModel, "minLotSize"), "value"), (float) 0.0);

	cfg.ltParams.developerModel = developerModel;
}

void ParseConfigFile::processSimulationNode(xercesc::DOMElement *node)
{
	///Several properties are set up as "x ms", or "x seconds", etc.
	cfg.simulation.baseGranMS = processTimeGranUnits(GetSingleElementByName(node, "base_granularity", true));
	cfg.simulation.totalRuntimeMS = processTimeGranUnits(GetSingleElementByName(node, "total_runtime", true));
	cfg.simulation.totalWarmupMS = processTimeGranUnits(GetSingleElementByName(node, "total_warmup"));

	cfg.simulation.baseGranSecond = cfg.simulation.baseGranMS / MILLISECONDS_IN_SECOND;

	if(cfg.simMobRunMode == RawConfigParams::SimMobRunMode::MID_TERM && ! (unsigned) cfg.simulation.baseGranSecond)
	{
		stringstream msg;
		msg << "Invalid value for base_granularity : " << cfg.simulation.baseGranSecond
		    << " seconds" << ". Expected: Value greater than or equal to 1 second";
		throw runtime_error(msg.str());
	}

	cfg.simulation.simStartTime = processValueDailyTime(GetSingleElementByName(node, "start_time", true));
	cfg.simulation.inSimulationTTUsage =
			processInSimulationTTUsage(GetSingleElementByName(node, "in_simulation_travel_time_usage", true));

	processWorkgroupAssignmentNode(GetSingleElementByName(node, "workgroup_assignment"));
	processMutexEnforcementNode(GetSingleElementByName(node, "mutex_enforcement"));
	processClosedLoopPropertiesNode(GetSingleElementByName(node, "closed_loop"));

	cfg.simulation.startingAutoAgentID =
			ParseInteger(GetNamedAttributeValue(GetSingleElementByName(node, "auto_id_start"), "value"), (int) 0);
}

void ParseConfigFile::processMergeLogFilesNode(xercesc::DOMElement *node)
{
	cfg.mergeLogFiles = ParseBoolean(GetNamedAttributeValue(node, "value"), false);
}

void ParseConfigFile::processGenericPropsNode(xercesc::DOMElement *node)
{
	if (!node)
	{
		return;
	}

	vector<DOMElement *> properties = GetElementsByName(node, "property");

	for (vector<DOMElement *>::const_iterator it = properties.begin(); it != properties.end(); ++it)
	{
		string key = ParseString(GetNamedAttributeValue(*it, "key"), "");
		string val = ParseString(GetNamedAttributeValue(*it, "value"), "");
		if (!(key.empty() && val.empty()))
		{
			cfg.genericProps[key] = val;
		}
	}
}

unsigned int ParseConfigFile::processTimeGranUnits(xercesc::DOMElement *node)
{
	return ParseTimegranAsMs(GetNamedAttributeValue(node, "value"), GetNamedAttributeValue(node, "units"));
}

DailyTime ParseConfigFile::processValueDailyTime(xercesc::DOMElement *node)
{
	return ParseDailyTime(GetNamedAttributeValue(node, "value"));
}

unsigned int ParseConfigFile::processInSimulationTTUsage(xercesc::DOMElement *node)
{
	unsigned int percentage = ParseUnsignedInt(GetNamedAttributeValue(node, "value"));

	if (percentage > 100)
	{
		percentage = 100;
	}

	return percentage;
}

void ParseConfigFile::processWorkgroupAssignmentNode(xercesc::DOMElement *node)
{
	cfg.simulation.workGroupAssigmentStrategy = ParseWrkGrpAssignEnum(GetNamedAttributeValue(node, "value"),
	                                                                  WorkGroup::ASSIGN_SMALLEST);
}

void ParseConfigFile::processClosedLoopPropertiesNode(xercesc::DOMElement *node)
{
	if (node)
	{
		ClosedLoopParams &params = cfg.simulation.closedLoop;
		params.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

		if (params.enabled)
		{
			DOMElement *element = GetSingleElementByName(node, "closed_loop_guidance");
			params.guidanceFile = ParseString(GetNamedAttributeValue(element, "file"));
			params.isGuidanceDirectional = ParseBoolean(GetNamedAttributeValue(element, "is_guidance_directional"), false);

			element = GetSingleElementByName(node, "closed_loop_toll");
			params.tollFile = ParseString(GetNamedAttributeValue(element, "file"));

			element = GetSingleElementByName(node, "closed_loop_incentives");
			params.incentivesFile = ParseString(GetNamedAttributeValue(element, "file"));

			element = GetSingleElementByName(node, "sensor_output");
			params.sensorOutputFile = ParseString(GetNamedAttributeValue(element, "file"), "sensor_out.txt");
			params.sensorStepSize =	ParseUnsignedInt(GetNamedAttributeValue(element, "step_size"), 300);

			params.logger = new BasicLogger(params.sensorOutputFile);
		}
	}
}

void ParseConfigFile::processMutexEnforcementNode(xercesc::DOMElement *node)
{
	cfg.simulation.mutexStategy = ParseMutexStrategyEnum(GetNamedAttributeValue(node, "strategy"), MtxStrat_Buffered);
}

void ParseConfigFile::processModelScriptsNode(xercesc::DOMElement *node)
{
	string format = ParseString(GetNamedAttributeValue(node, "format"), "");

	if (format.empty() || format != "lua")
	{
		stringstream msg;
		msg << "Invalid value for <model_scripts format=\""
		    << format << "\">. Expected: \"lua\"";
		throw runtime_error(msg.str());
	}

	string scriptsDirectoryPath = ParseString(GetNamedAttributeValue(node, "path"), "");

	if (scriptsDirectoryPath.empty())
	{
		stringstream msg;
		msg << "Empty value for <model_scripts path=\"\"/>. "
		    << "Expected: path to scripts";
		throw runtime_error(msg.str());
	}

	if ((*scriptsDirectoryPath.rbegin()) != '/')
	{
		//add a / to the end of the path string if it is not already there
		scriptsDirectoryPath.push_back('/');
	}

	ModelScriptsMap scriptsMap(scriptsDirectoryPath, format);

	for (DOMElement *item = node->getFirstElementChild(); item; item = item->getNextElementSibling())
	{
		string name = TranscodeString(item->getNodeName());

		if (name != "script")
		{
			Warn() << "\nWARNING! Invalid value for \'model_scripts\': \"" << TranscodeString(item->getNodeName())
			       << "\" in file " << inFilePath << ". Expected: \'script\'\n";
			continue;
		}

		string key = ParseString(GetNamedAttributeValue(item, "name"), "");
		string val = ParseString(GetNamedAttributeValue(item, "file"), "");

		if (key.empty() || val.empty())
		{
			Warn() << "\nWARNING! Empty value in <script name=\"" << key << "\" file=\"" << val << "\"/>. "
			       << "Expected: script name and file name";
			continue;
		}

		scriptsMap.addScriptFileName(key, val);
	}
	cfg.luaScriptsMap = scriptsMap;
}

