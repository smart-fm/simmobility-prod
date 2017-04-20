#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "ParseShortTermConfigFile.hpp"
#include "path/ParsePathXmlConfig.hpp"
#include "logging/Log.hpp"
#include "util/XmlParseHelper.hpp"

using namespace std;
using namespace xercesc;

namespace
{

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
			stringstream msg;
			msg << "Unable to parse granularity value: " << src;
			throw runtime_error(msg.str());
		}

		//Now split/parse it.
		double value = boost::lexical_cast<double>(src.substr(digStart, (digEnd - digStart)));
		std::string units = src.substr(unitStart, std::string::npos);

		return GetValueInMs(value, units, nullptr);
	}
	else
	{
		stringstream msg;
		msg << "Unable to parse granularity value: \"" << srcX << "\"";
		throw runtime_error(msg.str());
	}
}

}

namespace sim_mob
{

ParseShortTermConfigFile::ParseShortTermConfigFile(const std::string &configFileName, ConfigParams& sharedCfg, ST_Config &result) :
ParseConfigXmlBase(configFileName), cfg(sharedCfg), stCfg(result)
{
	parseXmlAndProcess();
}

void ParseShortTermConfigFile::processXmlFile(XercesDOMParser& parser)
{
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();

	//Verify that the root node is "config"
	if (TranscodeString(rootNode->getTagName()) != "config")
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". Root node must be \'config\'";
		throw runtime_error(msg.str());
	}

	try
	{
		processProcMapNode(GetSingleElementByName(rootNode, "db_proc_groups", true));
		processSystemNode(GetSingleElementByName(rootNode, "system", true));
		processModelScriptsNode(GetSingleElementByName(rootNode, "model_scripts", true));
		processWorkersNode(GetSingleElementByName(rootNode, "workers", true));
		processAmodControllerNode(GetSingleElementByName(rootNode, "amodcontroller"));
		processFmodControllerNode(GetSingleElementByName(rootNode, "fmodcontroller"));
		processVehicleTypesNode(GetSingleElementByName(rootNode, "vehicleTypes", true));
		processTripFilesNode(GetSingleElementByName(rootNode, "tripFiles"));
		processPersonCharacteristicsNode(GetSingleElementByName(rootNode, "person_characteristics"));
		processBusControllerNode(GetSingleElementByName(rootNode, "busController"));
		processBusCapacityNode(GetSingleElementByName(rootNode, "bus_default_capacity"));
		processPublicTransit(GetSingleElementByName(rootNode, "public_transit"));
		processOutputStatistics(GetSingleElementByName(rootNode, "output_statistics", true));
		processPathSetFileName(GetSingleElementByName(rootNode, "path-set-config-file", true));
		processTT_Update(GetSingleElementByName(rootNode, "travel_time_update", true));
	}
	catch(runtime_error &ex)
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". " << ex.what();
		throw runtime_error(msg.str());
	}
	
	//Take care of path-set manager configuration in here
    ParsePathXmlConfig(cfg.pathsetFile, cfg.getPathSetConf());
}

void ParseShortTermConfigFile::processProcMapNode(DOMElement* node)
{
	for (DOMElement* item = node->getFirstElementChild(); item; item = item->getNextElementSibling())
	{
		if (TranscodeString(item->getNodeName()) != "proc_map")
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
			stringstream msg;
			msg << "Invalid value for <proc_map format=\""
			    << pm.dbFormat << "\">. Expected: \"aimsun\"";
			throw runtime_error(msg.str());
		}

		//Loop through and save child attributes.
		for (DOMElement* mapItem = item->getFirstElementChild(); mapItem; mapItem = mapItem->getNextElementSibling())
		{
			if (TranscodeString(mapItem->getNodeName()) != "mapping")
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

void ParseShortTermConfigFile::processAmodControllerNode(DOMElement* node)
{
	if (node)
	{
		//Read the attribute value indicating whether AMOD is enabled or disabled
		stCfg.amod.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
		stCfg.amod.fileName = ParseString(GetNamedAttributeValue(node, "config_file"), "amod_config.xml");
	}
}

class FmodParser : public ParseConfigXmlBase
{
public:

	std::map<std::string, TripChainItem*>& allItems;

	FmodParser(std::map<std::string, TripChainItem*>& items, const std::string& configFileName) :
			ParseConfigXmlBase(configFileName), allItems(items)
	{
		parseXmlAndProcess();
	}

	virtual void processXmlFile(xercesc::XercesDOMParser& parser)
	{
		DOMElement* rootNode = parser.getDocument()->getDocumentElement();
		
		if (TranscodeString(rootNode->getTagName()) != "requests")
		{
			stringstream msg;
			msg << "Error parsing file: " << inFilePath << ". Root node must be \'requests\'";
			throw runtime_error(msg.str());
		}

		for (DOMElement* item = rootNode->getFirstElementChild(); item; item = item->getNextElementSibling())
		{
			if (TranscodeString(item->getNodeName()) == "request")
			{
				TripChainItem* trip = new Trip();
				trip->startTime = DailyTime(ParseString(GetNamedAttributeValue(item, "startTime"), ""));
				trip->endTime = DailyTime(ParseString(GetNamedAttributeValue(item, "endTime"), ""));
				trip->requestTime = ParseUnsignedInt(GetNamedAttributeValue(item, "timeWin"));
				trip->sequenceNumber = ParseUnsignedInt(GetNamedAttributeValue(item, "frequency"));
				trip->startLocationId = ParseString(GetNamedAttributeValue(item, "originNode"), "");
				trip->endLocationId = ParseString(GetNamedAttributeValue(item, "destNode"), "");
				std::string startId = ParseString(GetNamedAttributeValue(item, "startId"), "");
				ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
				DailyTime simStart = cfg.simulation.simStartTime;
				DailyTime simEnd = DailyTime(simStart.getValue() + cfg.simulation.totalRuntimeMS);
				DailyTime tripStart(trip->startTime);
				
				if (tripStart.getValue() > simStart.getValue() && tripStart.getValue() < simEnd.getValue())
				{
					allItems[startId] = trip;
				}
				else
				{
					delete trip;
				}
			}
			else
			{
				Warn() << "\nWARNING! Invalid value for \'requests\': \"" << TranscodeString(item->getNodeName())
				       << "\" in file " << inFilePath << ". Expected: \'request\'\n";
			}
		}
	}
};

void sim_mob::ParseShortTermConfigFile::processFmodControllerNode(xercesc::DOMElement* node)
{
	if (!node)
	{
		return;
	}

	//The fmod tag has an attribute
	stCfg.fmod.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	if (stCfg.fmod.enabled)
	{
		//Now set the rest.
		stCfg.fmod.ipAddress =
				ParseString(GetNamedAttributeValue(GetSingleElementByName(
						node, "ip_address"), "value"), "");

		stCfg.fmod.port =
				ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
						node, "port"), "value"), (unsigned int) 0);

		stCfg.fmod.updateTimeMS =
				ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
						node, "update_time_ms"), "value"), (unsigned int) 0);

		stCfg.fmod.mapfile =
				ParseString(GetNamedAttributeValue(GetSingleElementByName(
						node, "map_file"), "value"), "");

		stCfg.fmod.blockingTimeSec =
				ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(
						node, "blocking_time_sec"), "value"), (unsigned int) 0);

		std::string filename =
				ParseString(GetNamedAttributeValue(GetSingleElementByName(
						node, "requests_file"), "value"), "");

		FmodParser fmodParser(stCfg.fmod.allItems, filename);
	}
}

void ParseShortTermConfigFile::processSegmentDensityNode(DOMElement* node)
{
	if (node)
	{
		SegmentDensityMap &densityMap = stCfg.outputStats.segDensityMap;

		densityMap.outputEnabled = ParseBoolean(GetNamedAttributeValue(node, "outputEnabled"), false);
		
		if (densityMap.outputEnabled)
		{
			densityMap.updateInterval = ParseUnsignedInt(GetNamedAttributeValue(node, "updateInterval"), 1000);
			densityMap.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "private/DensityMap.csv");

			if (densityMap.updateInterval == 0)
			{
				stringstream msg;
				msg << "Invalid value for <segment_density updateInterval=\"" << densityMap.updateInterval
				    << "\">. Expected: \"non zero value\"";
				throw runtime_error(msg.str());
			}

			if (densityMap.fileName.empty())
			{
				stringstream msg;
				msg << "Empty value for <segment_density file-name=\""
				    << "\">. Expected: \"file name\"";
				throw runtime_error(msg.str());
			}
		}
	}
}

void ParseShortTermConfigFile::processSystemNode(DOMElement *node)
{
	processNetworkNode(GetSingleElementByName(node, "network", true));
	processAuraManagerImpNode(GetSingleElementByName(node, "aura_manager_impl", true));
	processLoadAgentsOrder(GetSingleElementByName(node, "load_agents", true));
	processCommSimNode(GetSingleElementByName(node, "commsim"));
	processGenericPropsNode(GetSingleElementByName(node, "generic_props", true));
}

void ParseShortTermConfigFile::processModelScriptsNode(xercesc::DOMElement* node)
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

void ParseShortTermConfigFile::processNetworkNode(DOMElement *node)
{
	processNetworkSourceNode(GetSingleElementByName(node, "network_source", true));
	processDatabaseNode(GetSingleElementByName(node, "network_database", true));
	processNetworkXmlInputNode(GetSingleElementByName(node, "network_xml_file_input", true));
	processNetworkXmlOutputNode(GetSingleElementByName(node, "network_xml_file_output", true));
}

void ParseShortTermConfigFile::processAuraManagerImpNode(DOMElement *node)
{
	const XMLCh *xmlValue = GetNamedAttributeValue(node, "value");

	if (xmlValue)
	{
		std::string value = TranscodeString(xmlValue);

		if(value == "packing-tree")
		{
			stCfg.auraManagerImplementation = AuraManager::IMPL_PACKING;
		}
		else if (value == "rstar")
		{
			stCfg.auraManagerImplementation = AuraManager::IMPL_RSTAR;
		}
		else if (value == "rdu")
		{
			stCfg.auraManagerImplementation = AuraManager::IMPL_RDU;
		}
		else if(value == "simtree")
		{
			stCfg.auraManagerImplementation = AuraManager::IMPL_SIMTREE;
		}
		else
		{
			stringstream msg;
			msg << "Invalid value for <aura_manager_impl value=\""
			    << value << "\">. Expected: \"packing-tree\" or \"rstar\" or \"rdu\" or \"simtree\"";
			throw runtime_error(msg.str());
		}
	}
}

void ParseShortTermConfigFile::processLoadAgentsOrder(DOMElement *node)
{
	///Separate into a string array.
	std::string value = ParseString(GetNamedAttributeValue(node, "order"), "");
	std::vector<std::string> valArray;
	boost::split(valArray, value, boost::is_any_of(", "), boost::token_compress_on);

	///Now, turn into an enum array.
	for (std::vector<std::string>::const_iterator it = valArray.begin(); it != valArray.end(); ++it)
	{
		LoadAgentsOrderOption opt(LoadAg_Database);

		if ((*it) == "database")
		{
			opt = LoadAg_Database;
		}
		else if ((*it) == "xml")
		{
			opt = LoadAg_XML;
		}
		else
		{
			std::stringstream out;
			out << "Invalid value for <load_agents order=\""
			    << *it << "\">. Expected: Comma-separated values: \"database\" and / or \"xml\"";
			throw std::runtime_error(out.str());
		}
		stCfg.loadAgentsOrder.push_back(opt);
	}
}

int ParseShortTermConfigFile::processValueInteger(xercesc::DOMElement* node)
{
	return ParseInteger(GetNamedAttributeValue(node, "value"));
}

bool ParseShortTermConfigFile::processValueBoolean(xercesc::DOMElement* node)
{
	return ParseBoolean(GetNamedAttributeValue(node, "value"), false);
}

void ParseShortTermConfigFile::processCommSimNode(DOMElement *node)
{
	if (!node)
	{
		return;
	}

	//Enabled?
	stCfg.commsim.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

	if (stCfg.commsim.enabled)
	{
		//Number of threads assigned to the boost I/O service that reads from Android clients.
		stCfg.commsim.numIoThreads = processValueInteger(GetSingleElementByName(node, "io_threads", true));

		//Minimum clients
		stCfg.commsim.minClients = processValueInteger(GetSingleElementByName(node, "min_clients", true));

		//Hold tick
		stCfg.commsim.holdTick = processValueInteger(GetSingleElementByName(node, "hold_tick", true));

		//Use ns-3 for routing?
		stCfg.commsim.useNs3 = processValueBoolean(GetSingleElementByName(node, "use_ns3", true));
	}
}

void ParseShortTermConfigFile::processLoopDetectorCountNode(DOMElement *node)
{
	if (node)
	{
		LoopDetectorCounts &counts = stCfg.outputStats.loopDetectorCounts;
		counts.outputEnabled = ParseBoolean(GetNamedAttributeValue(node, "outputEnabled"), false);

		if (counts.outputEnabled)
		{
			counts.frequency = ParseUnsignedInt(GetNamedAttributeValue(node, "frequency"), 600000);
			counts.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "private/VehCounts.csv");

			if (counts.frequency == 0)
			{
				stringstream msg;
				msg << "Invalid value for <loop-detector_counts frequency=\"" << counts.frequency
				    << "\">. Expected: \"non zero value\"";
				throw runtime_error(msg.str());
			}

			if (counts.fileName.empty())
			{
				stringstream msg;
				msg << "Empty value for <loop-detector_counts file-name=\"\">. Expected: \"file name\"";
				throw runtime_error(msg.str());
			}
		}
	}
}

void ParseShortTermConfigFile::processNetworkXmlOutputNode(DOMElement *node)
{
	stCfg.getNetworkXmlOutputFile() = ParseNonemptyString(GetNamedAttributeValue(node, "value"), "");
}

void ParseShortTermConfigFile::processNetworkXmlInputNode(DOMElement *node)
{
	stCfg.getNetworkXmlInputFile() = ParseNonemptyString(GetNamedAttributeValue(node, "value"), "private/SimMobilityInput.xml");
}

void ParseShortTermConfigFile::processNetworkSourceNode(DOMElement *node)
{
	const XMLCh *xmlValue = GetNamedAttributeValue(node, "value");

	if (xmlValue)
	{
		std::string value = TranscodeString(xmlValue);

		if (value == "xml")
		{
			stCfg.networkSource = NETSRC_XML;
		}
		else if (value == "database")
		{
			stCfg.networkSource = NETSRC_DATABASE;
		}
		else
		{
			stringstream msg;
			msg << "Invalid value for <network_source value=\""
			    << value << "\">. Expected: \"database\" or \"xml\"";
			throw runtime_error(msg.str());
		}
	}
}

void ParseShortTermConfigFile::processDatabaseNode(DOMElement *node)
{
	cfg.networkDatabase.database = ParseString(GetNamedAttributeValue(node, "database"), "");
	cfg.networkDatabase.credentials = ParseString(GetNamedAttributeValue(node, "credentials"), "");
	cfg.networkDatabase.procedures = ParseString(GetNamedAttributeValue(node, "proc_map"), "");
}

void ParseShortTermConfigFile::processWorkersNode(xercesc::DOMElement* node)
{
	processWorkerPersonNode(GetSingleElementByName(node, "person", true));
	processWorkerSignalNode(GetSingleElementByName(node, "signal", true));
	processWorkerIntMgrNode(GetSingleElementByName(node, "intersection_manager", true));
	processWorkerCommunicationNode(GetSingleElementByName(node, "communication", true));
}

void ParseShortTermConfigFile::processWorkerPersonNode(xercesc::DOMElement* node)
{
	stCfg.workers.person.count = ParseUnsignedInt(GetNamedAttributeValue(node, "count"));
	stCfg.workers.person.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
}

void ParseShortTermConfigFile::processWorkerSignalNode(xercesc::DOMElement* node)
{
	stCfg.workers.signal.count = ParseUnsignedInt(GetNamedAttributeValue(node, "count"));
	stCfg.workers.signal.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
}

void ParseShortTermConfigFile::processWorkerIntMgrNode(xercesc::DOMElement* node)
{
	if (node)
	{
		stCfg.workers.intersectionMgr.count = ParseUnsignedInt(GetNamedAttributeValue(node, "count"));
		stCfg.workers.intersectionMgr.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
	}
}

void ParseShortTermConfigFile::processWorkerCommunicationNode(xercesc::DOMElement* node)
{
	if (node)
	{
		stCfg.workers.communication.count = ParseUnsignedInt(GetNamedAttributeValue(node, "count"));
		stCfg.workers.communication.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
	}
}

void ParseShortTermConfigFile::processPersonCharacteristicsNode(DOMElement *node)
{
	if (!node)
	{
		return;
	}

	//Loop through all children
	int count = 0;
	for (DOMElement* item = node->getFirstElementChild(); item; item = item->getNextElementSibling())
	{
		if (TranscodeString(item->getNodeName()) != "person")
		{
			Warn() << "\nWARNING! Invalid value for \'person_characteristics\': \""
			       << TranscodeString(item->getNodeName()) << "\" in file " << inFilePath
			       << ". Expected: \'person\'\n";
			continue;
		}

		///Retrieve properties, add a new item to the vector.
		PersonCharacteristics res;
		res.lowerAge = ParseUnsignedInt(GetNamedAttributeValue(item, "lowerAge"), (unsigned int) 0);
		res.upperAge = ParseUnsignedInt(GetNamedAttributeValue(item, "upperAge"), (unsigned int) 0);
		res.lowerSecs = ParseInteger(GetNamedAttributeValue(item, "lowerSecs"), (int) 0);
		res.upperSecs = ParseInteger(GetNamedAttributeValue(item, "upperSecs"), (int) 0);
		res.walkSpeed = ParseFloat(GetNamedAttributeValue(item, "walkSpeed_kmph"), (float) 0.0);
		
		//Convert walking speed to m/s (from km/h)
		res.walkSpeed *= 0.277778;
		
		cfg.personCharacteristicsParams.personCharacteristics[count++] = res;
	}

	std::map<int, PersonCharacteristics> personCharacteristics = cfg.personCharacteristicsParams.personCharacteristics;
	
	/// calculate lowest age and highest age in the ranges
	for (std::map<int, PersonCharacteristics>::const_iterator iter = personCharacteristics.begin(); iter != personCharacteristics.end(); ++iter)
	{
		if (cfg.personCharacteristicsParams.lowestAge > iter->second.lowerAge)
		{
			cfg.personCharacteristicsParams.lowestAge = iter->second.lowerAge;
		}
		if (cfg.personCharacteristicsParams.highestAge < iter->second.upperAge)
		{
			cfg.personCharacteristicsParams.highestAge = iter->second.upperAge;
		}
	}
}

void ParseShortTermConfigFile::processGenericPropsNode(DOMElement *node)
{
	std::vector<DOMElement*> properties = GetElementsByName(node, "property");
	for (std::vector<DOMElement*>::const_iterator it = properties.begin(); it != properties.end(); ++it)
	{
		std::string key = ParseString(GetNamedAttributeValue(*it, "key"), "");
		std::string val = ParseString(GetNamedAttributeValue(*it, "value"), "");
		if (!(key.empty() && val.empty()))
		{
			stCfg.genericProps[key] = val;
		}
	}
}

void ParseShortTermConfigFile::processVehicleTypesNode(DOMElement *node)
{
	std::vector<DOMElement*> vehicles = GetElementsByName(node, "vehicleType");

	for (std::vector<DOMElement*>::const_iterator it = vehicles.begin(); it != vehicles.end(); it++)
	{
		VehicleType vehicleType;
		vehicleType.name = ParseString(GetNamedAttributeValue(*it, "name", true));
		vehicleType.length = ParseFloat(GetNamedAttributeValue(*it, "length"), 4.0);
		vehicleType.width = ParseFloat(GetNamedAttributeValue(*it, "width"), 2.0);
		vehicleType.capacity = ParseInteger(GetNamedAttributeValue(*it, "capacity"), 4);

		stCfg.vehicleTypes.push_back(vehicleType);
	}

	if (stCfg.vehicleTypes.empty())
	{
		stringstream msg;
		msg << "<vehicleTypes> node is empty";
		throw runtime_error(msg.str());
	}
}

void ParseShortTermConfigFile::processTripFilesNode(DOMElement *node)
{
	if (node)
	{
		std::vector<DOMElement*> tripFiles = GetElementsByName(node, "tripFile");
		for (std::vector<DOMElement*>::const_iterator it = tripFiles.begin(); it != tripFiles.end(); it++)
		{
			std::string name = ParseString(GetNamedAttributeValue(*it, "name"), "");
			std::string file = ParseString(GetNamedAttributeValue(*it, "fileName"), "");
			if (!(name.empty() && file.empty()))
			{
				stCfg.tripFiles[name] = file;
			}
		}

		for (std::map<std::string, std::string>::const_iterator it = stCfg.tripFiles.begin(); it != stCfg.tripFiles.end(); it++)
		{
			ParseShortTermTripFile parse(it->second, it->first, stCfg);
		}
	}
}

void ParseShortTermConfigFile::processBusControllerNode(DOMElement *node)
{
	if (node)
	{
		cfg.busController.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
		cfg.busController.busLineControlType = ParseString(GetNamedAttributeValue(node, "busline_control_type"), "");
	}
}

void ParseShortTermConfigFile::processBusCapacityNode(xercesc::DOMElement* node)
{
	if(node)
	{
		stCfg.defaultBusCapacity = ParseUnsignedInt(GetNamedAttributeValue(node, "value"), 50);
	}
}

void ParseShortTermConfigFile::processPublicTransit(xercesc::DOMElement* node)
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
				stringstream msg;
				msg << "Public transit is enabled, but stored procedures \"pt_vertices\" and / or "
				    << " \"pt_edges\" not defined";
				throw runtime_error(msg.str());
			}
		}
	}
}

void ParseShortTermConfigFile::processOutputStatistics(xercesc::DOMElement* node)
{
	processJourneyTimeNode(GetSingleElementByName(node, "journey_time"));
	processWaitingTimeNode(GetSingleElementByName(node, "waiting_time"));
	processWaitingCountsNode(GetSingleElementByName(node, "waiting_count"));
	processTravelTimeNode(GetSingleElementByName(node, "travel_time"));
	processPT_StopStatsNode(GetSingleElementByName(node, "pt_stop_stats"));
	processODTravelTimeNode(GetSingleElementByName(node, "od_travel_time"));
	processSegmentTravelTimeNode(GetSingleElementByName(node, "segment_travel_time"));
	processSegmentDensityNode(GetSingleElementByName(node, "segment_density"));
	processLoopDetectorCountNode(GetSingleElementByName(node, "loop-detector_counts"));
	processAssignmentMatrixNode(GetSingleElementByName(node, "assignment_matrix"));
}

void ParseShortTermConfigFile::processPathSetFileName(DOMElement* node)
{
	try
	{
		cfg.pathsetFile = ParseNonemptyString(GetNamedAttributeValue(node, "value"));
	}
	catch(runtime_error &ex)
	{
		stringstream msg;
		msg << "Error parsing <path-set-config-file>. " << ex.what();
		throw runtime_error(msg.str());
	}
}

void ParseShortTermConfigFile::processTT_Update(xercesc::DOMElement* node)
{
	cfg.getPathSetConf().interval = ParseInteger(GetNamedAttributeValue(node, "interval"), 300);
}

void ParseShortTermConfigFile::processJourneyTimeNode(xercesc::DOMElement* node)
{
	if(node)
	{		
		cfg.setJourneyTimeStatsFilename(ParseString(GetNamedAttributeValue(node, "file"), "journey_time.csv"));
	}
}

void ParseShortTermConfigFile::processWaitingTimeNode(xercesc::DOMElement* node)
{
	if(node)
	{
		cfg.setWaitingTimeStatsFilename(ParseString(GetNamedAttributeValue(node, "file"), "waiting_time.csv"));
	}
}

void ParseShortTermConfigFile::processWaitingCountsNode(xercesc::DOMElement* node)
{
	if(node)
	{
		cfg.setWaitingCountStatsFilename(ParseString(GetNamedAttributeValue(node, "file"), "waiting_count.csv"));
		cfg.setWaitingCountStatsInterval(ParseUnsignedInt(GetNamedAttributeValue(node, "interval"), 900000));
	}
}

void ParseShortTermConfigFile::processTravelTimeNode(xercesc::DOMElement* node)
{
	if(node)
	{
		cfg.setTravelTimeStatsFilename(ParseString(GetNamedAttributeValue(node, "file"), "travel_time.csv"));
	}
}

void ParseShortTermConfigFile::processPT_StopStatsNode(xercesc::DOMElement* node)
{
	if(node)
	{
		cfg.setPT_StopStatsFilename(ParseString(GetNamedAttributeValue(node, "file"), "pt_stop_stats.csv"));
	}
}

void ParseShortTermConfigFile::processAssignmentMatrixNode(xercesc::DOMElement* node)
{
	if(node)
	{
		bool enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

		if (enabled)
		{
			stCfg.outputStats.assignmentMatrix.enabled = true;
			stCfg.outputStats.assignmentMatrix.fileName = ParseString(GetNamedAttributeValue(node, "file-name"),
			                                                          "assignment_matrix.csv");
		}
	}
}

void ParseShortTermConfigFile::processODTravelTimeNode(xercesc::DOMElement* node)
{
	if(node)
	{
		bool enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
		if (enabled)
		{
			cfg.odTTConfig.enabled = true;
			cfg.odTTConfig.intervalMS = ParseUnsignedInt(GetNamedAttributeValue(node, "interval"), 300000);
			cfg.odTTConfig.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "od_travel_time.csv");
		}
	}
}

void ParseShortTermConfigFile::processSegmentTravelTimeNode(xercesc::DOMElement* node)
{
	if(node)
	{
		bool enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
		if (enabled)
		{
			cfg.rsTTConfig.enabled = true;
			cfg.rsTTConfig.intervalMS = ParseUnsignedInt(GetNamedAttributeValue(node, "interval"), 300000);
			cfg.rsTTConfig.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "segment_travel_time.csv");
		}
	}
}

ParseShortTermTripFile::ParseShortTermTripFile(const std::string &tripFileName, const std::string &tripName_, ST_Config &stConfig) :
ParseConfigXmlBase(tripFileName), cfg(stConfig), tripName(tripName_)
{
	parseXmlAndProcess();
}

void ParseShortTermTripFile::processXmlFile(XercesDOMParser &parser)
{
	DOMElement* rootNode = parser.getDocument()->getDocumentElement();

	//Verify that the root node is "trips"
	if (TranscodeString(rootNode->getTagName()) != "trips")
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". Root node must be \'config\'";
		throw runtime_error(msg.str());
	}

	try
	{
		processTrips(rootNode);
	}
	catch(runtime_error &ex)
	{
		stringstream msg;
		msg << "Error parsing file: " << inFilePath << ". " << ex.what();
		throw runtime_error(msg.str());
	}
}

void ParseShortTermTripFile::processTrips(DOMElement *node)
{
	if (node)
	{
		typedef std::vector<DOMElement*> DOMList;
		typedef std::vector<DOMElement*>::const_iterator DOMListIter;

		DOMList trips = GetElementsByName(node, "trip");
		unsigned int defaultTripId = 1;
		
		for (DOMListIter it = trips.begin(); it != trips.end(); ++it, ++defaultTripId)
		{
			defaultTripId = ParseUnsignedInt(GetNamedAttributeValue(*it, "id", false), defaultTripId);
			unsigned int personId = ParseUnsignedInt(GetNamedAttributeValue(*it, "personId", false), static_cast<unsigned int> (0));
			std::stringstream tripIdStr;
			tripIdStr << defaultTripId;
			
			DOMList subTrips = GetElementsByName(*it, "subTrip");
			unsigned int defaultSubTripId = 1;
			
			for (DOMListIter stIter = subTrips.begin(); stIter != subTrips.end(); ++stIter, ++defaultSubTripId)
			{				
				defaultSubTripId = ParseUnsignedInt(GetNamedAttributeValue(*stIter, "id", false), defaultSubTripId);
				EntityTemplate ent;
				
				ent.startTimeMs = ParseUnsignedInt(GetNamedAttributeValue(*stIter, "time", true), static_cast<unsigned int> (0));
				ent.startLaneIndex = ParseInteger(GetNamedAttributeValue(*stIter, "startLaneIndex", false), -1);
				ent.agentId = personId;
				ent.startSegmentId = ParseUnsignedInt(GetNamedAttributeValue(*stIter, "startSegmentId", false), static_cast<unsigned int> (0));
				ent.segmentStartOffset = ParseUnsignedInt(GetNamedAttributeValue(*stIter, "segmentStartOffset", false), static_cast<unsigned int> (0));
				ent.initialSpeed = ParseInteger(GetNamedAttributeValue(*stIter, "initialSpeed", false), (int) 0);
				ent.originNode = ParseInteger(GetNamedAttributeValue(*stIter, "originNode", true), (int) 0);
				ent.destNode = ParseInteger(GetNamedAttributeValue(*stIter, "destNode", true), (int) 0);
				ent.tripId = std::make_pair(defaultTripId, defaultSubTripId);
				ent.mode = ParseString(GetNamedAttributeValue(*stIter, "mode", false), "");
				
				if(!ent.mode.empty())
				{
					//Check if travel mode is public transport
					if(ent.mode != "PT" && ent.mode != "BusTravel" && ent.mode != "MRT" &&
							ent.mode != "Sharing" && ent.mode != "PrivateBus" && ent.mode != "Walk")
					{
						//Identify vehicle type from the mode of travel
						std::vector<VehicleType>::iterator vehTypeIter = std::find(cfg.vehicleTypes.begin(), cfg.vehicleTypes.end(), ent.mode);

						if (vehTypeIter == cfg.vehicleTypes.end())
						{
							stringstream msg;
							msg << "Travel mode \"" << ent.mode << "\" for trip \"" << defaultTripId
								<< "\" is not defined";
							throw std::runtime_error(msg.str());
						}
					}
				}
				else
				{
					stringstream msg;
					msg << "Travel mode \"" << ent.mode << "\" for trip \"" << defaultTripId
					    << "\" is not defined";
					throw std::runtime_error(msg.str());
				}
				
				cfg.futureAgents[tripIdStr.str()].push_back(ent);
			}
		}
	}
}

}
