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
	processDwellTimeElement(GetSingleElementByName(node, "dwell_time_parameters", true));
	processWalkSpeedElement(GetSingleElementByName(node, "pedestrian_walk_speed", true));
}

void ParseMidTermConfigFile::processPredayNode(xercesc::DOMElement* node)
{
	DOMElement* childNode = nullptr;
	childNode = GetSingleElementByName(node, "run_mode", true);
	mtCfg.setPredayRunMode(ParseString(GetNamedAttributeValue(childNode, "value", true), "simulation"));

	childNode = GetSingleElementByName(node, "threads", true);
	mtCfg.setNumPredayThreads(ParseUnsignedInt(GetNamedAttributeValue(childNode, "value", true), DEFAULT_NUM_THREADS_DEMAND));
	childNode = GetSingleElementByName(node, "output_tripchains", true);
	mtCfg.setOutputTripchains(ParseBoolean(GetNamedAttributeValue(childNode, "enabled", true)));
	childNode = GetSingleElementByName(node, "console_output", true);
	mtCfg.setConsoleOutput(ParseBoolean(GetNamedAttributeValue(childNode, "enabled", true)));

	processModelScriptsNode(GetSingleElementByName(node, "model_scripts", true));
	processMongoCollectionsNode(GetSingleElementByName(node, "mongo_collections", true));
	processCalibrationNode(GetSingleElementByName(node, "calibration", true));
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
	std::vector<int>& dwellTimeParams = mtCfg.getDwellTimeParams();
	for (std::vector<std::string>::const_iterator it = valArray.begin(); it != valArray.end(); it++)
	{
		try
		{
			int val = boost::lexical_cast<int>(*it);
			dwellTimeParams.push_back(val);
		} catch (...)
		{
			throw std::runtime_error("load dwelling-time parameters errors in MT_Config");
		}
	}
}

void ParseMidTermConfigFile::processWalkSpeedElement(xercesc::DOMElement* node)
{
	mtCfg.setPedestrianWalkSpeed(ParseFloat(GetNamedAttributeValue(node, "value", true), nullptr));
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


