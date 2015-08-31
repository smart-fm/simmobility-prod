//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParseMidTermConfigFile.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include "behavioral/CalibrationStatistics.hpp"
#include "util/LangHelpers.hpp"
#include "util/XmlParseHelper.hpp"

namespace
{
const int DEFAULT_NUM_THREADS_DEMAND = 2; // default number of threads for demand
const unsigned NUM_SECONDS_IN_AN_HOUR = 3600;
const std::string EMPTY_STRING = std::string();

unsigned int ProcessTimegranUnits(xercesc::DOMElement* node)
{
	return ParseTimegranAsSecond(GetNamedAttributeValue(node, "value"), GetNamedAttributeValue(node, "units"), NUM_SECONDS_IN_AN_HOUR);
}

}
namespace sim_mob
{
ParseMidTermConfigFile::ParseMidTermConfigFile(const std::string& configFileName, MT_Config& result, ConfigParams& cfgPrms) :
		ParseConfigXmlBase(configFileName), mtCfg(result), configParams(cfgPrms)
{
	parseXmlAndProcess();
}

void ParseMidTermConfigFile::processXmlFile(xercesc::XercesDOMParser& parser)
{
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();
	//Verify that the root node is "config"
	if (TranscodeString(rootNode->getTagName()) != "config")
	{
		throw std::runtime_error("xml parse error: root node must be \"config\"");
	}

	processMidTermRunMode(GetSingleElementByName(rootNode, "mid_term_run_mode", true));

	if (configParams.RunningMidSupply())
	{
		processSupplyNode(GetSingleElementByName(rootNode, "supply", true));
	}
	else if (configParams.RunningMidDemand())
	{
		processPredayNode(GetSingleElementByName(rootNode, "preday", true));
	}
	mtCfg.sealConfig(); //no more updation in mtConfig
}

void ParseMidTermConfigFile::processMidTermRunMode(xercesc::DOMElement* node)
{
	configParams.setMidTermRunMode(TranscodeString(GetNamedAttributeValue(node, "value", true)));
}

void ParseMidTermConfigFile::processSupplyNode(xercesc::DOMElement* node)
{
	//processProcMapNode(GetSingleElementByName(node, "proc_map", true));
	//processActivityLoadIntervalElement(GetSingleElementByName(node, "activity_load_interval", true));
	processUpdateIntervalElement(GetSingleElementByName(node, "update_interval", true));
	processDwellTimeElement(GetSingleElementByName(node, "dwell_time_parameters", true));
	processWalkSpeedElement(GetSingleElementByName(node, "pedestrian_walk_speed", true));
	processStatisticsOutputNode(GetSingleElementByName(node, "statistics_output_paramemters", true));
	processBusCapactiyElement(GetSingleElementByName(node, "bus_default_capacity", true));
}


void ParseMidTermConfigFile::processPredayNode(xercesc::DOMElement* node)
{
	DOMElement* childNode = nullptr;
	childNode = GetSingleElementByName(node, "run_mode", true);
	mtCfg.setPredayRunMode(ParseString(GetNamedAttributeValue(childNode, "value", true), "simulation"));

	childNode = GetSingleElementByName(node, "threads", true);
	mtCfg.setNumPredayThreads(ParseUnsignedInt(GetNamedAttributeValue(childNode, "value", true), DEFAULT_NUM_THREADS_DEMAND));
	if(mtCfg.runningPredaySimulation())
	{
		childNode = GetSingleElementByName(node, "output_activity_schedule", true);
		mtCfg.setFileOutputEnabled(ParseBoolean(GetNamedAttributeValue(childNode, "enabled", true)));
	}
	else if(mtCfg.runningPredayLogsumComputationForLT()) { mtCfg.setFileOutputEnabled(true); }
	childNode = GetSingleElementByName(node, "output_predictions", true);
	mtCfg.setOutputPredictions(ParseBoolean(GetNamedAttributeValue(childNode, "enabled", true)));
	childNode = GetSingleElementByName(node, "console_output", true);
	mtCfg.setConsoleOutput(ParseBoolean(GetNamedAttributeValue(childNode, "enabled", true)));

	childNode = GetSingleElementByName(node, "population", true);
	mtCfg.setPopulationSource(ParseString(GetNamedAttributeValue(childNode, "source", false), EMPTY_STRING));
	if(mtCfg.getPopulationSource() == db::POSTGRES)
	{
		std::string database = ParseString(GetNamedAttributeValue(childNode, "database", false), EMPTY_STRING);
		std::string credential = ParseString(GetNamedAttributeValue(childNode, "credential", false), EMPTY_STRING);
		mtCfg.setPopulationDb(database, credential);

		childNode = GetSingleElementByName(node, "logsum", true);
		database = ParseString(GetNamedAttributeValue(childNode, "database", false), EMPTY_STRING);
		credential = ParseString(GetNamedAttributeValue(childNode, "credential", false), EMPTY_STRING);
		mtCfg.setLogsumDb(database, credential);
	}

	processModelScriptsNode(GetSingleElementByName(node, "model_scripts", true));
	processMongoCollectionsNode(GetSingleElementByName(node, "mongo_collections", true));
	processCalibrationNode(GetSingleElementByName(node, "calibration", true));
}

void ParseMidTermConfigFile::processProcMapNode(xercesc::DOMElement* node)
{
	StoredProcedureMap spMap(ParseString(GetNamedAttributeValue(node, "id")));
	spMap.dbFormat = ParseString(GetNamedAttributeValue(node, "format"), "");

	//Loop through and save child attributes.
	for (DOMElement* mapItem=node->getFirstElementChild(); mapItem; mapItem=mapItem->getNextElementSibling()) {
		if (TranscodeString(mapItem->getNodeName())!="mapping") {
			Warn() <<"Invalid proc_map child node.\n";
			continue;
		}

		std::string key = ParseString(GetNamedAttributeValue(mapItem, "name"), "");
		std::string val = ParseString(GetNamedAttributeValue(mapItem, "procedure"), "");
		if (key.empty() || val.empty()) {
			Warn() <<"Invalid mapping; missing \"name\" or \"procedure\".\n";
			continue;
		}

		spMap.procedureMappings[key] = val;
	}
	mtCfg.setStoredProcedureMap(spMap);
	configParams.constructs.procedureMaps[spMap.getId()] = spMap;
}

void ParseMidTermConfigFile::processActivityLoadIntervalElement(xercesc::DOMElement* node)
{
	unsigned interval = ProcessTimegranUnits(node);
	mtCfg.setActivityScheduleLoadInterval(interval);
	configParams.system.genericProps["activity_load_interval"] = boost::lexical_cast<std::string>(interval);
}

void ParseMidTermConfigFile::processUpdateIntervalElement(xercesc::DOMElement* node)
{
	unsigned interval = ProcessTimegranUnits(node)/((unsigned)configParams.baseGranSecond());
	mtCfg.setSupplyUpdateInterval(interval);
	configParams.system.genericProps["update_interval"] = boost::lexical_cast<std::string>(interval);
}

void ParseMidTermConfigFile::processDwellTimeElement(xercesc::DOMElement* node)
{
	DOMElement* child = GetSingleElementByName(node, "parameters");
	if (child == nullptr)
	{
		throw std::runtime_error("load dwelling-time parameters errors in MT_Config");
	}
	std::string value = ParseString(GetNamedAttributeValue(child, "value"), "");
	std::vector<std::string> valArray;
	boost::split(valArray, value, boost::is_any_of(", "), boost::token_compress_on);
	std::vector<float>& dwellTimeParams = mtCfg.getDwellTimeParams();
	for (std::vector<std::string>::const_iterator it = valArray.begin(); it != valArray.end(); it++)
	{
		try
		{
			float val = boost::lexical_cast<float>(*it);
			dwellTimeParams.push_back(val);
		} catch (...)
		{
			throw std::runtime_error("load dwelling-time parameters errors in MT_Config");
		}
	}
}


void ParseMidTermConfigFile::processStatisticsOutputNode(xercesc::DOMElement* node)
{
	DOMElement* child = GetSingleElementByName(node, "journey_time_csv_file_output");
	if (child == nullptr)
	{
		throw std::runtime_error("load statistics output parameters errors in MT_Config");
	}
	std::string value = ParseString(GetNamedAttributeValue(child, "value"), "");
	mtCfg.setFilenameOfJourneyTimeStats(value);

	child = GetSingleElementByName(node, "waiting_time_csv_file_output");
	if (child == nullptr)
	{
		throw std::runtime_error("load statistics output parameters errors in MT_Config");
	}
	value = ParseString(GetNamedAttributeValue(child, "value"), "");
	mtCfg.setFilenameOfWaitingTimeStats(value);

	child = GetSingleElementByName(node, "waiting_amount_csv_file_output");
	if (child == nullptr)
	{
		throw std::runtime_error("load statistics output parameters errors in MT_Config");
	}
	value = ParseString(GetNamedAttributeValue(child, "value"), "");
	mtCfg.setFilenameOfWaitingAmountStats(value);

	child = GetSingleElementByName(node, "travel_time_csv_file_output");
	if (child == nullptr)
	{
		throw std::runtime_error("load statistics output parameters errors in MT_Config");
	}
	value = ParseString(GetNamedAttributeValue(child, "value"), "");
	mtCfg.setFilenameOfTravelTimeStats(value);
}


void ParseMidTermConfigFile::processWalkSpeedElement(xercesc::DOMElement* node)
{
	mtCfg.setPedestrianWalkSpeed(ParseFloat(GetNamedAttributeValue(node, "value", true), nullptr));
}

void ParseMidTermConfigFile::processBusCapactiyElement(xercesc::DOMElement* node)
{
	mtCfg.setBusCapacity(ParseUnsignedInt(GetNamedAttributeValue(node, "value", true), nullptr));
}

void ParseMidTermConfigFile::processModelScriptsNode(xercesc::DOMElement* node)
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
		//add a / to the end of the path string if it is not already there
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
	mtCfg.setModelScriptsMap(scriptsMap);
}

void ParseMidTermConfigFile::processMongoCollectionsNode(xercesc::DOMElement* node)
{
	MongoCollectionsMap mongoColls(ParseString(GetNamedAttributeValue(node, "db_name"), ""));
	for (DOMElement* item = node->getFirstElementChild(); item; item = item->getNextElementSibling())
	{
		std::string name = TranscodeString(item->getNodeName());
		if (name != "mongo_collection")
		{
			Warn() << "Invalid db_proc_groups child node.\n";
			continue;
		}
		std::string key = ParseString(GetNamedAttributeValue(item, "name"), "");
		std::string val = ParseString(GetNamedAttributeValue(item, "collection"), "");
		if (key.empty() || val.empty())
		{
			Warn() << "Invalid mongo_collection; missing \"name\" or \"collection\".\n";
			continue;
		}
		mongoColls.addCollectionName(key, val);
	}
	mtCfg.setMongoCollectionsMap(mongoColls);
}

void sim_mob::ParseMidTermConfigFile::processCalibrationNode(xercesc::DOMElement* node)
{
	if(mtCfg.runningPredayCalibration())
	{
		PredayCalibrationParams spsaCalibrationParams;
		PredayCalibrationParams wspsaCalibrationParams;

		//get name of csv listing variables to calibrate
		DOMElement* variablesNode = GetSingleElementByName(node, "variables", true);
		spsaCalibrationParams.setCalibrationVariablesFile(ParseString(GetNamedAttributeValue(variablesNode, "file"), ""));
		wspsaCalibrationParams.setCalibrationVariablesFile(ParseString(GetNamedAttributeValue(variablesNode, "file"), ""));

		//get name of csv listing observed statistics
		DOMElement* observedStatsNode = GetSingleElementByName(node, "observed_statistics", true);
		std::string observedStatsFile = ParseString(GetNamedAttributeValue(observedStatsNode, "file"), "");
		spsaCalibrationParams.setObservedStatisticsFile(observedStatsFile);
		wspsaCalibrationParams.setObservedStatisticsFile(observedStatsFile);

		DOMElement* calibrationMethod = GetSingleElementByName(node, "calibration_technique", true);
		mtCfg.setCalibrationMethodology(ParseString(GetNamedAttributeValue(calibrationMethod, "value"), "WSPSA"));

		DOMElement* logsumFrequencyNode = GetSingleElementByName(node, "logsum_computation_frequency", true);
		mtCfg.setLogsumComputationFrequency(ParseUnsignedInt(GetNamedAttributeValue(logsumFrequencyNode, "value", true)));

		/**parse SPSA node*/
		DOMElement* spsaNode = GetSingleElementByName(node, "SPSA", true);
		DOMElement* childNode = nullptr;

		childNode = GetSingleElementByName(spsaNode, "iterations", true);
		spsaCalibrationParams.setIterationLimit(ParseUnsignedInt(GetNamedAttributeValue(childNode, "value", true)));

		childNode = GetSingleElementByName(spsaNode, "tolerence", true);
		spsaCalibrationParams.setTolerance(ParseFloat(GetNamedAttributeValue(childNode, "value", true)));

		childNode = GetSingleElementByName(spsaNode, "gradient_step_size", true);
		spsaCalibrationParams.setInitialGradientStepSize(ParseFloat(GetNamedAttributeValue(childNode, "initial_value", true)));
		spsaCalibrationParams.setAlgorithmCoefficient2(ParseFloat(GetNamedAttributeValue(childNode, "algorithm_coefficient2", true)));

		childNode = GetSingleElementByName(spsaNode, "step_size", true);
		spsaCalibrationParams.setStabilityConstant(ParseFloat(GetNamedAttributeValue(childNode, "stability_constant", true)));
		spsaCalibrationParams.setInitialStepSize(ParseFloat(GetNamedAttributeValue(childNode, "initial_value", true)));
		spsaCalibrationParams.setAlgorithmCoefficient1(ParseFloat(GetNamedAttributeValue(childNode, "algorithm_coefficient1", true)));

		/**process W-SPSA node*/
		DOMElement* wspsaNode = GetSingleElementByName(node, "WSPSA", true);

		childNode = GetSingleElementByName(wspsaNode, "iterations", true);
		wspsaCalibrationParams.setIterationLimit(ParseUnsignedInt(GetNamedAttributeValue(childNode, "value", true)));

		childNode = GetSingleElementByName(wspsaNode, "tolerence", true);
		wspsaCalibrationParams.setTolerance(ParseFloat(GetNamedAttributeValue(childNode, "value", true)));

		childNode = GetSingleElementByName(wspsaNode, "gradient_step_size", true);
		wspsaCalibrationParams.setInitialGradientStepSize(ParseFloat(GetNamedAttributeValue(childNode, "initial_value", true)));
		wspsaCalibrationParams.setAlgorithmCoefficient2(ParseFloat(GetNamedAttributeValue(childNode, "algorithm_coefficient2", true)));

		childNode = GetSingleElementByName(wspsaNode, "step_size", true);
		wspsaCalibrationParams.setStabilityConstant(ParseFloat(GetNamedAttributeValue(childNode, "stability_constant", true)));
		wspsaCalibrationParams.setInitialStepSize(ParseFloat(GetNamedAttributeValue(childNode, "initial_value", true)));
		wspsaCalibrationParams.setAlgorithmCoefficient1(ParseFloat(GetNamedAttributeValue(childNode, "algorithm_coefficient1", true)));

		childNode = GetSingleElementByName(wspsaNode, "weight_matrix", true);
		wspsaCalibrationParams.setWeightMatrixFile(ParseString(GetNamedAttributeValue(childNode, "file"), ""));

		mtCfg.setSPSA_CalibrationParams(spsaCalibrationParams);
		mtCfg.setWSPSA_CalibrationParams(wspsaCalibrationParams);

		DOMElement* outputNode = GetSingleElementByName(node, "output", true);
		mtCfg.setCalibrationOutputFile(ParseString(GetNamedAttributeValue(outputNode, "file"), "out.txt"));
	}
	//else just return.
}

}


