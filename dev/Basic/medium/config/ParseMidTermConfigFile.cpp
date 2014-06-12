//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "ParseMidTermConfigFile.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
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
	processThreadsNode(GetSingleElementByName(node, "threads", true));
	processModelScriptsNode(GetSingleElementByName(node, "model_scripts", true));
	processMongoCollectionsNode(GetSingleElementByName(node, "mongo_collections", true));
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

void ParseMidTermConfigFile::processThreadsNode(xercesc::DOMElement* node)
{
	mtCfg.setNumPredayThreads(ParseUnsignedInt(GetNamedAttributeValue(node, "value", true), DEFAULT_NUM_THREADS_DEMAND));
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

		scriptsMap.scriptFileName[key] = val;
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
		mongoColls.collectionName[key] = val;
	}
	mtCfg.setMongoCollectionsMap(mongoColls);
}

}
