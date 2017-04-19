//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParseMidTermConfigFile.hpp"

#include <boost/algorithm/string.hpp>
#include "path/ParsePathXmlConfig.hpp"
#include "util/XmlParseHelper.hpp"

namespace
{
const int DEFAULT_NUM_THREADS_DEMAND = 2; // default number of threads for demand
const unsigned NUM_METERS_IN_KM = 1000;
const unsigned NUM_SECONDS_IN_AN_HOUR = 3600;

unsigned int ProcessTimegranUnits(xercesc::DOMElement* node)
{
	return ParseTimegranAsSecond(GetNamedAttributeValue(node, "value"), GetNamedAttributeValue(node, "units"),
	                             NUM_SECONDS_IN_AN_HOUR);
}

unsigned int ParseGranularitySingle(const XMLCh* srcX)
{
	if (srcX)
	{
		//Search for "[0-9]+ ?[^0-9]+), roughly.
		std::string src = TranscodeString(srcX);
		size_t digStart = src.find_first_of("1234567890");
		size_t digEnd = src.find_first_not_of("1234567890", digStart + 1);
		size_t unitStart = src.find_first_not_of(" ", digEnd);

		if (digStart != 0 || digStart == std::string::npos || digEnd == std::string::npos || unitStart == std::string::npos)
		{
			std::stringstream msg;
			msg << "Unable to parse granularity value: " << src;
			throw std::runtime_error(msg.str());
		}

		//Now split/parse it.
		double value = boost::lexical_cast<double>(src.substr(digStart, (digEnd - digStart)));
		std::string units = src.substr(unitStart, std::string::npos);

		return GetValueInMs(value, units, nullptr);
	}
	else
	{
		std::stringstream msg;
		msg << "Unable to parse granularity value: " << srcX;
		throw std::runtime_error(msg.str());
	}
}

}
namespace sim_mob
{
ParseMidTermConfigFile::ParseMidTermConfigFile(const std::string& configFileName, MT_Config& result, ConfigParams &cfg) :
        ParseConfigXmlBase(configFileName), mtCfg(result), cfg(cfg)
{
	parseXmlAndProcess();
}

void ParseMidTermConfigFile::processXmlFile(xercesc::XercesDOMParser& parser)
{
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();

	//Verify that the root node is "config"
	if (TranscodeString(rootNode->getTagName()) != "config")
	{
		std::stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". Root node must be \'config\'";
		throw std::runtime_error(msg.str());
	}

	try
	{
		processMidTermRunMode(GetSingleElementByName(rootNode, "mid_term_run_mode", true));
		processProcMapNode(GetSingleElementByName(rootNode, "db_proc_groups", true));
		processSystemNode(GetSingleElementByName(rootNode, "system", true));
		processWorkersNode(GetSingleElementByName(rootNode, "workers", true));
		processIncidentsNode(GetSingleElementByName(rootNode, "incidentsData"));
		processBusStopScheduledTimesNode(GetSingleElementByName(rootNode, "scheduledTimes"));
		processBusControllerNode(GetSingleElementByName(rootNode, "busController", true));
		processGenerateBusRoutesNode(GetSingleElementByName(rootNode, "generateBusRoutes"));
		processTrainControllerNode(GetSingleElementByName(rootNode, "trainController", true));
		processTT_Update(GetSingleElementByName(rootNode, "travel_time_update", true));
		processPublicTransit(GetSingleElementByName(rootNode, "public_transit"));
		processRegionRestrictionNode(GetSingleElementByName(rootNode, "region_restriction"));
		processPathSetFileName(GetSingleElementByName(rootNode, "pathset_config_file", true));

		if (mtCfg.RunningMidSupply())
		{
			processSupplyNode(GetSingleElementByName(rootNode, "supply", true));
		}
		else if (mtCfg.RunningMidDemand())
		{
			processPredayNode(GetSingleElementByName(rootNode, "preday", true));
		}
	}
	catch(std::runtime_error &ex)
	{
		std::stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". " << ex.what();
		throw std::runtime_error(msg.str());
	}

	//Take care of path-set manager configuration in here
	ParsePathXmlConfig(cfg.pathsetFile, cfg.getPathSetConf());

	if (mtCfg.isRegionRestrictionEnabled() && cfg.getPathSetConf().psRetrievalWithoutBannedRegion.empty())
	{
		std::stringstream msg;
		msg << "Error parsing file: " << cfg.pathsetFile << ". Invalid value for "
		    << "<functions pathset_without_banned_area=\"\"/>. Expected: stored procedure name";
		throw std::runtime_error(msg.str());
	}

	mtCfg.sealConfig();
}

void ParseMidTermConfigFile::processMidTermRunMode(xercesc::DOMElement* node)
{
	mtCfg.setMidTermRunMode(TranscodeString(GetNamedAttributeValue(node, "value", true)));
}

void ParseMidTermConfigFile::processSupplyNode(xercesc::DOMElement* node)
{
	processUpdateIntervalElement(GetSingleElementByName(node, "update_interval", true));
	processDwellTimeElement(GetSingleElementByName(node, "dwell_time_parameters", true));
	processWalkSpeedElement(GetSingleElementByName(node, "pedestrian_walk_speed", true));
	processThreadsNumInPersonLoaderElement(GetSingleElementByName(node, "thread_number_in_person_loader", true));
	processStatisticsOutputNode(GetSingleElementByName(node, "output_statistics", true));
	processBusCapactiyElement(GetSingleElementByName(node, "bus_default_capacity", true));
	processSpeedDensityParamsNode(GetSingleElementByName(node, "speed_density_params", true));
	cfg.luaScriptsMap = processModelScriptsNode(GetSingleElementByName(node, "model_scripts", true));
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

	childNode = GetSingleElementByName(node, "console_output", true);
	mtCfg.setConsoleOutput(ParseBoolean(GetNamedAttributeValue(childNode, "enabled", true)));

	childNode = GetSingleElementByName(node, "logsum_table", true);
	mtCfg.setLogsumTableName(ParseString(GetNamedAttributeValue(childNode, "name", true)));

	ModelScriptsMap luaModelsMap = processModelScriptsNode(GetSingleElementByName(node, "model_scripts", true));
	mtCfg.setModelScriptsMap(luaModelsMap);

	processCalibrationNode(GetSingleElementByName(node, "calibration", true));
}

void ParseMidTermConfigFile::processProcMapNode(xercesc::DOMElement* node)
{
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling())
	{
		if (TranscodeString(item->getNodeName())!="proc_map")
		{
			Warn() << "\nWARNING! Invalid value for \'db_proc_groups\': \"" << TranscodeString(item->getNodeName())
			       << "\" in file " << inFilePath << ". Expected: \'proc_map\'\n";
			continue;
		}

		//Retrieve some attributes from the Node itself.
		StoredProcedureMap pm(ParseString(GetNamedAttributeValue(item, "id")));
		pm.dbFormat = ParseString(GetNamedAttributeValue(item, "format"), "");

		if (pm.dbFormat != "aimsun" && pm.dbFormat != "long-term")
		{
			std::stringstream msg;
			msg << "Invalid value for <proc_map format=\""
			    << pm.dbFormat << "\">. Expected: \"aimsun\"";
			throw std::runtime_error(msg.str());
		}

		//Loop through and save child attributes.
		for (DOMElement* mapItem=item->getFirstElementChild(); mapItem; mapItem=mapItem->getNextElementSibling())
		{
			if (TranscodeString(mapItem->getNodeName())!="mapping")
			{
				Warn() << "\nWARNING! Invalid value for \'proc_map\': \"" << TranscodeString(item->getNodeName())
				       << "\" in file " << inFilePath << ". Expected: \'mapping\'\n";
				continue;
			}

			std::string key = ParseString(GetNamedAttributeValue(mapItem, "name"), "");
			std::string val = ParseString(GetNamedAttributeValue(mapItem, "procedure"), "");

			if (key.empty() || val.empty())
			{
				Warn() << "\nWARNING! Empty value in <mapping name=\"" << key << "\" procedure=\"" << val
				       << "\"/>. Expected: mapping name and stored procedure name";
				continue;
			}

			pm.procedureMappings[key] = val;
		}

		cfg.procedureMaps[pm.getId()] = pm;
	}
}

void ParseMidTermConfigFile::processUpdateIntervalElement(xercesc::DOMElement* node)
{
	unsigned interval = ProcessTimegranUnits(node)/((unsigned)cfg.baseGranSecond());

	if(interval == 0)
	{
		std::stringstream msg;
		msg << "Invalid value for <update_interval value=\""
		    << interval << "\"/>. Expected: \"Value greater than 0\"";
		throw std::runtime_error(msg.str());
	}

	mtCfg.setSupplyUpdateInterval(interval);
}

void ParseMidTermConfigFile::processDwellTimeElement(xercesc::DOMElement* node)
{
	DOMElement* child = GetSingleElementByName(node, "parameters", true);
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
		}
		catch (...)
		{
			std::stringstream msg;
			msg << "Invalid value for <parameters value=\""
			    << value << "\"/>. Expected: \"Comma (and/or space) separated numeric values\"";
			throw std::runtime_error(msg.str());
		}
	}
}

void ParseMidTermConfigFile::processStatisticsOutputNode(xercesc::DOMElement* node)
{
	DOMElement* child = GetSingleElementByName(node, "journey_time", true);
	std::string value = ParseString(GetNamedAttributeValue(child, "file"), "");
	cfg.setJourneyTimeStatsFilename(value);

	child = GetSingleElementByName(node, "waiting_time", true);
	value = ParseString(GetNamedAttributeValue(child, "file"), "");
	cfg.setWaitingTimeStatsFilename(value);

	child = GetSingleElementByName(node, "waiting_count", true);
	value = ParseString(GetNamedAttributeValue(child, "file"), "");
	cfg.setWaitingCountStatsFilename(value);

	child = GetSingleElementByName(node, "travel_time", true);
	value = ParseString(GetNamedAttributeValue(child, "file"), "");
	cfg.setTravelTimeStatsFilename(value);

	child = GetSingleElementByName(node, "pt_stop_stats", true);
	value = ParseString(GetNamedAttributeValue(child, "file"), "");
	cfg.setPT_StopStatsFilename(value);

	child = GetSingleElementByName(node, "subtrip_metrics");
	processSubtripTravelMetricsOutputNode(child);

	child = GetSingleElementByName(node, "screen_line_count");
	processScreenLineNode(child);

	child = GetSingleElementByName(node, "pt_reroute");
	value = ParseString(GetNamedAttributeValue(child, "file"), "");
	cfg.setPT_PersonRerouteFilename(value);
}

void ParseMidTermConfigFile::processSpeedDensityParamsNode(xercesc::DOMElement* node)
{
	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling())
	{
		if (TranscodeString(item->getNodeName())!="param")
		{
			Warn() << "\nWARNING! Invalid value for \'speed_density_params\': \""
			       << TranscodeString(item->getNodeName())
			       << "\" in file " << inFilePath << ". Expected: \'param\'\n";
			continue;
		}

		//Retrieve some attributes from the Node itself.
		int linkCategory = ParseInteger(GetNamedAttributeValue(item, "category"));
		SpeedDensityParams speedDensityParams;
		speedDensityParams.setAlpha(ParseFloat(GetNamedAttributeValue(item, "alpha")));
		speedDensityParams.setBeta(ParseFloat(GetNamedAttributeValue(item, "beta")));
		speedDensityParams.setMinDensity(ParseFloat(GetNamedAttributeValue(item, "kmin")));
		speedDensityParams.setJamDensity(ParseFloat(GetNamedAttributeValue(item, "kjam")));
		mtCfg.setSpeedDensityParam(linkCategory, speedDensityParams);
	}
}


void ParseMidTermConfigFile::processWalkSpeedElement(xercesc::DOMElement* node)
{
	double walkSpeed = ParseFloat(GetNamedAttributeValue(node, "value", true), nullptr);
	walkSpeed = walkSpeed * (NUM_METERS_IN_KM/NUM_SECONDS_IN_AN_HOUR); //convert kmph to m/s
	mtCfg.setPedestrianWalkSpeed(walkSpeed);
}

void ParseMidTermConfigFile::processThreadsNumInPersonLoaderElement(xercesc::DOMElement* node)
{
	if(!node)
	{
		return;
	}

	unsigned int num = ParseUnsignedInt(GetNamedAttributeValue(node, "value", true), 1);
	mtCfg.setThreadsNumInPersonLoader(num);
}

void ParseMidTermConfigFile::processBusCapactiyElement(xercesc::DOMElement* node)
{
	mtCfg.setBusCapacity(ParseUnsignedInt(GetNamedAttributeValue(node, "value", true), nullptr));
}

ModelScriptsMap ParseMidTermConfigFile::processModelScriptsNode(xercesc::DOMElement* node)
{
	std::string format = ParseString(GetNamedAttributeValue(node, "format"), "");

	if (format.empty() || format != "lua")
	{
		std::stringstream msg;
		msg << "Invalid value for <model_scripts format=\""
		    << format << "\">. Expected: \"lua\"";
		throw std::runtime_error(msg.str());
	}

	std::string scriptsDirectoryPath = ParseString(GetNamedAttributeValue(node, "path"), "");

	if (scriptsDirectoryPath.empty())
	{
		std::stringstream msg;
		msg << "Empty value for <model_scripts path=\"\"/>. "
		    << "Expected: path to scripts";
		throw std::runtime_error(msg.str());
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
			Warn() << "\nWARNING! Invalid value for \'model_scripts\': \"" << TranscodeString(item->getNodeName())
			       << "\" in file " << inFilePath << ". Expected: \'script\'\n";
			continue;
		}

		std::string key = ParseString(GetNamedAttributeValue(item, "name"), "");
		std::string val = ParseString(GetNamedAttributeValue(item, "file"), "");

		if (key.empty() || val.empty())
		{
			Warn() << "\nWARNING! Empty value in <script name=\"" << key << "\" file=\"" << val << "\"/>. "
			       << "Expected: script name and file name";
			continue;
		}

		scriptsMap.addScriptFileName(key, val);
	}

	mtCfg.setModelScriptsMap(scriptsMap );
	return scriptsMap;
}

void ParseMidTermConfigFile::processCalibrationNode(xercesc::DOMElement* node)
{
	if(mtCfg.runningPredayCalibration())
	{
		PredayCalibrationParams spsaCalibrationParams;
		PredayCalibrationParams wspsaCalibrationParams;

        ///get name of csv listing variables to calibrate
		DOMElement* variablesNode = GetSingleElementByName(node, "variables", true);
		spsaCalibrationParams.setCalibrationVariablesFile(ParseString(GetNamedAttributeValue(variablesNode, "file"), ""));
		wspsaCalibrationParams.setCalibrationVariablesFile(ParseString(GetNamedAttributeValue(variablesNode, "file"), ""));

        ///get name of csv listing observed statistics
		DOMElement* observedStatsNode = GetSingleElementByName(node, "observed_statistics", true);
		std::string observedStatsFile = ParseString(GetNamedAttributeValue(observedStatsNode, "file"), "");
		spsaCalibrationParams.setObservedStatisticsFile(observedStatsFile);
		wspsaCalibrationParams.setObservedStatisticsFile(observedStatsFile);

		DOMElement* calibrationMethod = GetSingleElementByName(node, "calibration_technique", true);
		mtCfg.setCalibrationMethodology(ParseString(GetNamedAttributeValue(calibrationMethod, "value"), "WSPSA"));

		DOMElement* logsumFrequencyNode = GetSingleElementByName(node, "logsum_computation_frequency", true);
		mtCfg.setLogsumComputationFrequency(ParseUnsignedInt(GetNamedAttributeValue(logsumFrequencyNode, "value", true)));

        ///parse SPSA node
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

        ///process W-SPSA node
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

void ParseMidTermConfigFile::processSystemNode(DOMElement *node)
{
	processDatabaseNode(GetSingleElementByName(node, "network_database", true), cfg.networkDatabase);
	processDatabaseNode(GetSingleElementByName(node, "population_database", true), cfg.populationDatabase);
	processGenericPropsNode(GetSingleElementByName(node, "generic_props", true));
}

void ParseMidTermConfigFile::processDatabaseNode(DOMElement *node, DatabaseDetails& dbDetails)
{
	dbDetails.database = ParseString(GetNamedAttributeValue(node, "database"), "");
	dbDetails.credentials = ParseString(GetNamedAttributeValue(node, "credentials"), "");
	dbDetails.procedures = ParseString(GetNamedAttributeValue(node, "proc_map"), "");
}

void ParseMidTermConfigFile::processGenericPropsNode(DOMElement *node)
{
	if (!node)
	{
		return;
	}

	std::vector<DOMElement*> properties = GetElementsByName(node, "property");
	for (std::vector<DOMElement*>::const_iterator it=properties.begin(); it!=properties.end(); ++it)
	{
		std::string key = ParseString(GetNamedAttributeValue(*it, "key"), "");
		std::string val = ParseString(GetNamedAttributeValue(*it, "value"), "");
		if (!(key.empty() && val.empty()))
		{
			mtCfg.genericProps[key] = val;
		}
	}
}

void ParseMidTermConfigFile::processWorkersNode(DOMElement *node)
{
	processWorkerPersonNode(GetSingleElementByName(node, "person", true));
}

void ParseMidTermConfigFile::processWorkerPersonNode(DOMElement *node)
{
	mtCfg.workers.person.count = ParseUnsignedInt(GetNamedAttributeValue(node, "count"));
	mtCfg.workers.person.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
}

void ParseMidTermConfigFile::processScreenLineNode(DOMElement *node)
{
	if(node)
	{
		mtCfg.screenLineParams.outputEnabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

		if(mtCfg.screenLineParams.outputEnabled)
		{
			mtCfg.screenLineParams.interval = ParseUnsignedInt(GetNamedAttributeValue(node, "interval"), 300);
			mtCfg.screenLineParams.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "screenLineCount.txt");

			if(mtCfg.screenLineParams.interval == 0)
			{
				std::stringstream msg;
				msg << "Invalid value for <screen_line_count interval=\"" << mtCfg.screenLineParams.interval
				    << "\">. Expected: \"non zero value\"";
				throw std::runtime_error(msg.str());
			}

			if(mtCfg.screenLineParams.fileName.empty())
			{
				std::stringstream msg;
				msg << "Empty value for <screen_line_count file=\"\">. Expected: \"file name\"";
				throw std::runtime_error(msg.str());
			}
		}
	}
}

void ParseMidTermConfigFile::processGenerateBusRoutesNode(xercesc::DOMElement* node)
{
	if (!node)
	{
		cfg.generateBusRoutes = false;
		return;
	}
	cfg.generateBusRoutes = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
}

void ParseMidTermConfigFile::processSubtripTravelMetricsOutputNode(xercesc::DOMElement* node)
{
	//subtrip output for preday
	if (node)
	{
		bool enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"));
		if (enabled)
		{
			cfg.subTripTravelTimeEnabled = true;
			cfg.subTripLevelTravelTimeOutput =
					ParseString(GetNamedAttributeValue(node, "file"), "subtrip_travel_times.csv");
		}
	}
}

void ParseMidTermConfigFile::processPublicTransit(xercesc::DOMElement* node)
{
	if (!node)
	{
		cfg.setPublicTransitEnabled(false);
	}
	else
	{
		cfg.setPublicTransitEnabled(ParseBoolean(GetNamedAttributeValue(node, "enabled"), false));
		if (cfg.isPublicTransitEnabled())
		{
			const std::string& key = cfg.networkDatabase.procedures;
			std::map<std::string, StoredProcedureMap>::const_iterator procMapIt = cfg.procedureMaps.find(key);

			if (procMapIt->second.procedureMappings.count("pt_vertices") == 0 ||
					procMapIt->second.procedureMappings.count("pt_edges") == 0)
			{
				std::stringstream msg;
				msg << "Public transit is enabled, but stored procedures \"pt_vertices\" and / or "
				    << " \"pt_edges\" not defined";
				throw std::runtime_error(msg.str());
			}
		}
	}
}

void ParseMidTermConfigFile::processTT_Update(xercesc::DOMElement* node)
{
	sim_mob::ConfigManager::GetInstanceRW().PathSetConfig().interval =
			ParseInteger(GetNamedAttributeValue(node, "interval"), 300);
}


void ParseMidTermConfigFile::processRegionRestrictionNode(xercesc::DOMElement* node)
{
	if (!node)
	{
		mtCfg.regionRestrictionEnabled = false;
		return;
	}

	mtCfg.regionRestrictionEnabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
}


void ParseMidTermConfigFile::processBusStopScheduledTimesNode(xercesc::DOMElement* node)
{
	if (!node)
	{
		return;
	}

	//Loop through all children
	int count=0;

	for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling())
	{
		if (TranscodeString(item->getNodeName())!="stop")
		{
			Warn() << "\nWARNING! Invalid value for \'scheduledTimes\': \"" << TranscodeString(item->getNodeName())
			       << "\" in file " << inFilePath << ". Expected: \'stop\'\n";
			continue;
		}

		///Retrieve properties, add a new item to the vector.
		BusStopScheduledTime res;
		res.offsetAT = ParseUnsignedInt(GetNamedAttributeValue(item, "offsetAT"), static_cast<unsigned int>(0));
		res.offsetDT = ParseUnsignedInt(GetNamedAttributeValue(item, "offsetDT"), static_cast<unsigned int>(0));
		cfg.busScheduledTimes[count++] = res;
	}
}

void ParseMidTermConfigFile::processIncidentsNode(xercesc::DOMElement* node)
{
	if(!node)
	{
		return;
	}

	bool enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	if(!enabled)
	{
		return;
	}

	for(DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling())
	{
		if(TranscodeString(item->getNodeName())=="incident")
		{
			IncidentParams incident;
			incident.incidentId = ParseUnsignedInt(GetNamedAttributeValue(item, "id"));
			incident.visibilityDistance = ParseFloat(GetNamedAttributeValue(item, "visibility"));
			incident.segmentId = ParseUnsignedInt(GetNamedAttributeValue(item, "segment") );
			incident.position = ParseFloat(GetNamedAttributeValue(item, "position"));
			incident.capFactor = ParseFloat(GetNamedAttributeValue(item, "cap_factor") );
			incident.startTime = ParseDailyTime(GetNamedAttributeValue(item, "start_time") ).getValue();
			incident.duration = ParseDailyTime(GetNamedAttributeValue(item, "duration") ).getValue();
			incident.length = ParseFloat(GetNamedAttributeValue(item, "length") );
			incident.compliance = ParseFloat(GetNamedAttributeValue(item, "compliance") );
			incident.accessibility = ParseFloat(GetNamedAttributeValue(item, "accessibility") );

			for(DOMElement* child=item->getFirstElementChild(); child; child=child->getNextElementSibling())
			{
				IncidentParams::LaneParams lane;
				lane.laneId = ParseUnsignedInt(GetNamedAttributeValue(child, "laneId"));
				lane.speedLimit = ParseFloat(GetNamedAttributeValue(child, "speedLimitFactor") );
				incident.laneParams.push_back(lane);
			}

			mtCfg.incidents.push_back(incident);
		}
		else if(TranscodeString(item->getNodeName())=="disruption")
		{
			DisruptionParams disruption;
			disruption.id = ParseUnsignedInt(GetNamedAttributeValue(item, "id"));
			disruption.startTime = ParseDailyTime(GetNamedAttributeValue(item, "start_time") );
			disruption.duration = ParseDailyTime(GetNamedAttributeValue(item, "duration") );

			for(DOMElement* child=item->getFirstElementChild(); child; child=child->getNextElementSibling())
			{
				std::string val = ParseString(GetNamedAttributeValue(child, "name"), "");
				disruption.platformNames.push_back(val);
				val = ParseString(GetNamedAttributeValue(child, "lineId"), "");
				disruption.platformLineIds.push_back(val);
			}

			mtCfg.disruptions.push_back(disruption);
		}
	}
}

void ParseMidTermConfigFile::processBusControllerNode(DOMElement *node)
{
	cfg.busController.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
	cfg.busController.busLineControlType = ParseString(GetNamedAttributeValue(node, "busline_control_type"), "");
}

void ParseMidTermConfigFile::processTrainControllerNode(xercesc::DOMElement *node)
{
	cfg.trainController.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
	cfg.trainController.trainControlType = ParseString(GetNamedAttributeValue(node, "train_control_type"), "");

	DOMElement* child  = GetSingleElementByName(node, "output_enabled", true);
	cfg.trainController.outputEnabled = ParseBoolean(GetNamedAttributeValue(child, "value"), false);
	child  = GetSingleElementByName(node, "distance_arriving_at_platform", true);
	cfg.trainController.distanceArrivingAtPlatform = ParseFloat(GetNamedAttributeValue(child, "value"));
}

void ParseMidTermConfigFile::processPathSetFileName(xercesc::DOMElement* node)
{
	cfg.pathsetFile = ParseString(GetNamedAttributeValue(node, "value"));
}

}


