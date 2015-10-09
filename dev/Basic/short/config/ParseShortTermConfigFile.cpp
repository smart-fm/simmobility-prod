#include "ParseShortTermConfigFile.hpp"
#include "util/XmlParseHelper.hpp"
#include "entities/AuraManager.hpp"

using namespace xercesc;

namespace
{

sim_mob::NetworkSource ParseNetSourceEnum(const XMLCh* srcX, SystemParams::NetworkSource* defValue) {
    if (srcX) {
        std::string src = TranscodeString(srcX);
        if (src=="xml") {
            return SystemParams::NETSRC_XML;
        } else if (src=="database") {
            return SystemParams::NETSRC_DATABASE;
        }
        throw std::runtime_error("Expected SystemParams::NetworkSource value.");
    }

    ///Wasn't found.
    if (!defValue) {
        throw std::runtime_error("Mandatory SystemParams::NetworkSource variable; no default available.");
    }
    return *defValue;
}

sim_mob::AuraManager::AuraManagerImplementation ParseAuraMgrImplEnum(const XMLCh* srcX, AuraManager::AuraManagerImplementation* defValue) {
    if (srcX) {
        std::string src = TranscodeString(srcX);
        if (src=="simtree") {
            return AuraManager::IMPL_SIMTREE;
        } else if (src=="rdu") {
            return AuraManager::IMPL_RDU;
        } else if (src=="rstar") {
            return AuraManager::IMPL_RSTAR;
        }
        throw std::runtime_error("Expected AuraManager::AuraManagerImplementation value.");
    }

    ///Wasn't found.
    if (!defValue) {
        throw std::runtime_error("Mandatory AuraManager::AuraManagerImplementation variable; no default available.");
    }
    return *defValue;
}

sim_mob::AuraManager::AuraManagerImplementation ParseAuraMgrImplEnum(const XMLCh* srcX, sim_mob::AuraManager::AuraManagerImplementation defValue) {
    return ParseAuraMgrImplEnum(srcX, &defValue);
}

sim_mob::AuraManager::AuraManagerImplementation ParseAuraMgrImplEnum(const XMLCh* srcX) {
    return ParseAuraMgrImplEnum(srcX, nullptr);
}

void splitRoleString(std::string& roleString, std::vector<std::string>& roles)
{
    std::string delimiter = "|";
    size_t pos = 0;
    std::string token;
    while ((pos = roleString.find(delimiter)) != std::string::npos) {
        token = roleString.substr(0, pos);
        roles.push_back(token);
        roleString.erase(0, pos + delimiter.length());
    }
}

}

namespace sim_mob
{

ParseShortTermConfigFile::ParseShortTermConfigFile(const std::string &configFileName,
                           ST_Config &result):
    ParseConfigXmlBase(configFileName), cfg(result)
{
    parseXmlAndProcess();
}

void ParseShortTermConfigFile::processXmlFile(XercesDOMParser& parser)
{
    DOMElement* rootNode = parser.getDocument()->getDocumentElement();
    ///Verify that the root node is "config"
    if (TranscodeString(rootNode->getTagName()) != "config")
    {
        throw std::runtime_error("xml parse error: root node must be \"config\"");
    }

    processProcMapNode(GetSingleElementByName(rootNode, "db_proc_groups", true));
    processSystemNode(GetSingleElementByName(rootNode, "system",true));
    processWorkersNode(GetSingleElementByName(rootNode, "workers", true));
    processAmodControllerNode(GetSingleElementByName(rootNode, "amodcontroller"));
    processFmodControllerNode(GetSingleElementByName(rootNode, "fmodcontroller"));
    processVehicleTypesNode(GetSingleElementByName(rootNode, "vehicleTypes"));
    processTripFilesNode(GetSingleElementByName(rootNode, "tripFiles"));
    processSegmentDensityNode(GetSingleElementByName(rootNode, "short-term_density-map"));
    processLoopDetectorCountNode(GetSingleElementByName(rootNode, "loop-detector_counts"));
    processPersonCharacteristicsNode(GetSingleElementByName(rootNode, "personCharacteristics"));
}

void ParseShortTermConfigFile::processProcMapNode(DOMElement* node)
{
    ConfigParams& sharedConfig = ConfigManager::GetInstanceRW().FullConfig();
    for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling())
    {
        if (TranscodeString(item->getNodeName())!="proc_map")
        {
            Warn() <<"Invalid db_proc_groups child node.\n";
            continue;
        }

        ///Retrieve some attributes from the Node itself.
        StoredProcedureMap pm(ParseString(GetNamedAttributeValue(item, "id")));
        pm.dbFormat = ParseString(GetNamedAttributeValue(item, "format"), "");
        if (pm.dbFormat != "aimsun" && pm.dbFormat != "long-term")
        {
            throw std::runtime_error("Stored procedure map format not supported.");
        }

        ///Loop through and save child attributes.
        for (DOMElement* mapItem=item->getFirstElementChild(); mapItem; mapItem=mapItem->getNextElementSibling())
        {
            if (TranscodeString(mapItem->getNodeName())!="mapping")
            {
                Warn() <<"Invalid proc_map child node.\n";
                continue;
            }

            std::string key = ParseString(GetNamedAttributeValue(mapItem, "name"), "");
            std::string val = ParseString(GetNamedAttributeValue(mapItem, "procedure"), "");
            if (key.empty() || val.empty())
            {
                Warn() <<"Invalid mapping; missing \"name\" or \"procedure\".\n";
                continue;
            }

            pm.procedureMappings[key] = val;
        }

        sharedConfig.procedureMaps[pm.getId()] = pm;
    }
}

void ParseShortTermConfigFile::processAmodControllerNode(DOMElement* node)
{
    if(node)
    {
        ///Read the attribute value indicating whether AMOD is enabled or disabled
        cfg.amod.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);
    }
}

void ParseShortTermConfigFile::processFmodControllerNode(DOMElement* node)
{
    if(node)
    {
        ///The fmod tag has an attribute
        cfg.fmod.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

        ///Now set the rest.
        cfg.fmod.ipAddress = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "ip_address"), "value"), "");

        cfg.fmod.port = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "port"), "value"),
                                              static_cast<unsigned int>(0));

        cfg.fmod.mapfile = ParseString(GetNamedAttributeValue(GetSingleElementByName(node, "map_file"), "value"), "");

        cfg.fmod.blockingTimeSec = ParseUnsignedInt(GetNamedAttributeValue(GetSingleElementByName(node, "blocking_time_Sec"),
                                                                                "value"), static_cast<unsigned int>(0));
    }
}

void ParseShortTermConfigFile::processSegmentDensityNode(DOMElement* node)
{
    if(node)
    {
        cfg.segDensityMap.outputEnabled = ParseBoolean(GetNamedAttributeValue(node, "outputEnabled"), "false");
        if(cfg.segDensityMap.outputEnabled)
        {
            cfg.segDensityMap.updateInterval = ParseUnsignedInt(GetNamedAttributeValue(node, "updateInterval"), 1000);
            cfg.segDensityMap.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "private/DensityMap.csv");

            if(cfg.segDensityMap.updateInterval == 0)
            {
                throw std::runtime_error("ParseConfigFile::ProcessShortDensityMapNode - Update interval for aggregating density is 0");
            }

            if(cfg.segDensityMap.fileName.empty())
            {
                throw std::runtime_error("ParseConfigFile::ProcessShortDensityMapNode - File name is empty");
            }
        }
    }
}

void ParseShortTermConfigFile::processSystemNode(DOMElement *node)
{
    if(node)
    {
        processNetworkNode(GetSingleElementByName(rootNode, "network", true));
        processAuraManagerImpNode(GetElementsByName(node, "aura_manager_impl", true));
        processLoadAgentsOrder(GetElementsByName(node, "load_agents", true));
        processCommSimNode(GetElementsByName(node, "commsim"));
        processXmlSchemaFilesNode(GetSingleElementByName(rootNode, "xsd_schema_files"));
        processGenericPropsNode(GetSingleElementByName(node, "generic_props"));
    }
    else
    {
        throw std::runtime_error("processSystemNode : System node not defined");
    }
}

void ParseShortTermConfigFile::processNetworkNode(DOMElement *node)
{
    if(node)
    {
        processNetworkSourceNode(GetElementsByName(node, "network_source", true));
        processDatabaseNode(GetElementsByName(node, "network_database", true));
        processNetworkXmlInputNode(GetElementsByName(node, "network_xml_file_input", true));
        processNetworkXmlOutputNode(GetElementsByName(node, "network_xml_file_output", true));
    }
    else
    {
        throw std::runtime_error("processNetworkNode : network node not defined");
    }
}

void ParseShortTermConfigFile::processAuraManagerImpNode(DOMElement *node)
{
    cfg.auraManagerImplementation = ParseAuraMgrImplEnum(GetNamedAttributeValue(node, "value"), AuraManager::IMPL_RSTAR);
}

void ParseShortTermConfigFile::processLoadAgentsOrder(DOMElement *node)
{
    ///Separate into a string array.
    std::string value = ParseString(GetNamedAttributeValue(node, "order"), "");
    std::vector<std::string> valArray;
    boost::split(valArray, value, boost::is_any_of(", "), boost::token_compress_on);

    ///Now, turn into an enum array.
    for (std::vector<std::string>::const_iterator it=valArray.begin(); it!=valArray.end(); ++it)
    {
        LoadAgentsOrderOption opt(LoadAg_Database);
        if ((*it) == "database")
        {
            opt = LoadAg_Database;
        } else if ((*it) == "drivers")
        {
            opt = LoadAg_Drivers;
        } else if ((*it) == "pedestrians")
        {
            opt = LoadAg_Pedestrians;
        } else if ((*it) == "passengers")
        {
            opt = LoadAg_Passengers;
        } else if ((*it) == "xml-tripchains")
        {
            opt = LoadAg_XmlTripChains;
        } else
        {
            std::stringstream out;
            out.str("");
            out << "Unexpected load_agents order param." << "[" << *it << "]";
            throw std::runtime_error(out.str());
        }
        cfg.loadAgentsOrder.push_back(opt);
    }
}

void ParseShortTermConfigFile::processCommSimNode(DOMElement *node)
{
    if (!node)
    {
        return;
    }

    ///Enabled?
    cfg.commsim.enabled = ParseBoolean(GetNamedAttributeValue(node, "enabled"), false);

    ///Number of threads assigned to the boost I/O service that reads from Android clients.
    cfg.commsim.numIoThreads = ProcessValueInteger(GetSingleElementByName(node, "io_threads", true));

    ///Minimum clients
    cfg.commsim.minClients = ProcessValueInteger(GetSingleElementByName(node, "min_clients", true));

    ///Hold tick
    cfg.commsim.holdTick = ProcessValueInteger(GetSingleElementByName(node, "hold_tick", true));

    ///Use ns-3 for routing?
    cfg.commsim.useNs3 = ProcessValueBoolean(GetSingleElementByName(node, "use_ns3", true));
}

void ParseShortTermConfigFile::processLoopDetectorCountNode(DOMElement *node)
{
    if(node)
    {
        cfg.loopDetectorCounts.outputEnabled = ParseBoolean(GetNamedAttributeValue(node, "outputEnabled"), "false");
        if(cfg.loopDetectorCounts.outputEnabled)
        {
            cfg.loopDetectorCounts.frequency = ParseUnsignedInt(GetNamedAttributeValue(node, "frequency"), 600000);
            cfg.loopDetectorCounts.fileName = ParseString(GetNamedAttributeValue(node, "file-name"), "private/VehCounts.csv");

            if(cfg.loopDetectorCounts.frequency == 0)
            {
                throw std::runtime_error("ParseConfigFile::ProcessLoopDetectorCountsNode - "
                                         "Update frequency for aggregating vehicle counts is 0");
            }

            if(cfg.loopDetectorCounts.fileName.empty())
            {
                throw std::runtime_error("ParseConfigFile::ProcessLoopDetectorCountsNode - File name is empty");
            }
        }
    }
}

void ParseShortTermConfigFile::processXmlSchemaFilesNode(DOMElement *node)
{
    ///For now, only the Road Network has an XSD file (doing this for the config file from within it would be difficult).
    DOMElement* rn = GetSingleElementByName(node, "road_network");
    if (rn)
    {
        std::vector<DOMElement*> options = GetElementsByName(rn, "option");
        for (std::vector<DOMElement*>::const_iterator it=options.begin(); it!=options.end(); ++it)
        {
            std::string path = ParseString(GetNamedAttributeValue(*it, "value"), "");
            if (!path.empty())
            {
                ///See if the file exists.
                if (boost::filesystem::exists(path))
                {
                    ///Convert it to an absolute path.
                    boost::filesystem::path abs_path = boost::filesystem::absolute(path);
                    cfg.roadNetworkXsdSchemaFile = abs_path.string();
                    break;
                }
            }
        }

        ///Did we try and find nothing?
        if (!options.empty() && cfg.roadNetworkXsdSchemaFile.empty())
        {
            Warn() <<"Warning: No viable options for road_network schema file." <<std::endl;
        }
    }
}

void ParseShortTermConfigFile::processNetworkXmlOutputNode(DOMElement *node)
{
    cfg.networkXmlOutputFile = ParseNonemptyString(GetNamedAttributeValue(node, "value"), "");
}

void ParseShortTermConfigFile::processNetworkXmlInputNode(DOMElement *node)
{
    cfg.networkXmlInputFile = ParseNonemptyString(GetNamedAttributeValue(node, "value"), "private/SimMobilityInput.xml");
}

void ParseShortTermConfigFile::processNetworkSourceNode(DOMElement *node)
{
    cfg.networkSource = ParseNetSourceEnum(GetNamedAttributeValue(node, "value"), SystemParams::NETSRC_XML);
}

void ParseShortTermConfigFile::processDatabaseNode(DOMElement *node)
{
    if(node)
    {
        ConfigParams& sharedConfig = ConfigManager::GetInstanceRW().FullConfig();
        sharedConfig.networkDatabase.database = ParseString(GetNamedAttributeValue(node, "database"), "");
        sharedConfig.networkDatabase.credentials = ParseString(GetNamedAttributeValue(node, "credentials"), "");
        sharedConfig.networkDatabase.procedures = ParseString(GetNamedAttributeValue(node, "proc_map"), "");
    }
    else
    {
        throw std::runtime("processDatabaseNode : Network database configuration not defined");
    }
}

void ParseShortTermConfigFile::processWorkersNode(xercesc::DOMElement* node)
{
    if(node)
    {
        processWorkerPersonNode(GetSingleElementByName(node, "person", true));
        processWorkerSignalNode(GetSingleElementByName(node, "signal", true));
        processWorkerIntMgrNode(GetSingleElementByName(node, "intersection_manager", true));
        processWorkerCommunicationNode(GetSingleElementByName(node, "communication", true));
    }
    else
    {
        throw std::runtime_error("processWorkerParamsNode : Workers configuration not defined");
    }
}

void ParseShortTermConfigFile::processWorkerPersonNode(xercesc::DOMElement* node)
{
    if(node)
    {
        cfg.workers.person.count = ParseInteger(GetNamedAttributeValue(node, "count"));
        cfg.workers.person.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
    }
}

void ParseShortTermConfigFile::processWorkerSignalNode(xercesc::DOMElement* node)
{
    if(node)
    {
        cfg.workers.signal.count = ParseInteger(GetNamedAttributeValue(node, "count"));
        cfg.workers.signal.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
    }
}

void ParseShortTermConfigFile::processWorkerIntMgrNode(xercesc::DOMElement* node)
{
    if(node)
    {
        cfg.workers.intersectionMgr.count = ParseInteger(GetNamedAttributeValue(node, "count"));
        cfg.workers.intersectionMgr.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
    }
}

void ParseShortTermConfigFile::processWorkerCommunicationNode(xercesc::DOMElement* node)
{
    if(node)
    {
        cfg.workers.communication.count = ParseInteger(GetNamedAttributeValue(node, "count"));
        cfg.workers.communication.granularityMs = ParseGranularitySingle(GetNamedAttributeValue(node, "granularity"));
    }
}

void ParseShortTermConfigFile::processPersonCharacteristicsNode(DOMElement *node)
{
    if (!node) {
        return;
    }

    ConfigParams& sharedConfig = ConfigManager::GetInstanceRW().FullConfig();

    ///Loop through all children
    int count=0;
    for (DOMElement* item=node->getFirstElementChild(); item; item=item->getNextElementSibling()) {
        if (TranscodeString(item->getNodeName())!="person") {
            Warn() <<"Invalid personCharacteristics child node.\n";
            continue;
        }

        ///Retrieve properties, add a new item to the vector.
        PersonCharacteristics res;
        res.lowerAge = ParseUnsignedInt(GetNamedAttributeValue(item, "lowerAge"), static_cast<unsigned int>(0));
        res.upperAge = ParseUnsignedInt(GetNamedAttributeValue(item, "upperAge"), static_cast<unsigned int>(0));
        res.lowerSecs = ParseInteger(GetNamedAttributeValue(item, "lowerSecs"), static_cast<int>(0));
        res.upperSecs = ParseInteger(GetNamedAttributeValue(item, "upperSecs"), static_cast<int>(0));
        sharedConfig.personCharacteristicsParams.personCharacteristics[count++] = res;
    }

    std::map<int, PersonCharacteristics> personCharacteristics =  sharedConfig.personCharacteristicsParams.personCharacteristics;
    /// calculate lowest age and highest age in the ranges
    for(std::map<int, PersonCharacteristics>::const_iterator iter=personCharacteristics.begin();iter != personCharacteristics.end(); ++iter) {
        if(cfg.personCharacteristicsParams.lowestAge > iter->second.lowerAge) {
            sharedConfig.personCharacteristicsParams.lowestAge = iter->second.lowerAge;
        }
        if(cfg.personCharacteristicsParams.highestAge < iter->second.upperAge) {
            sharedConfig.personCharacteristicsParams.highestAge = iter->second.upperAge;
        }
    }
}

void ParseShortTermConfigFile::processGenericPropsNode(DOMElement *node)
{
    if (!node) {
        return;
    }

    std::vector<DOMElement*> properties = GetElementsByName(node, "property");
    for (std::vector<DOMElement*>::const_iterator it=properties.begin(); it!=properties.end(); ++it)
    {
        std::string key = ParseString(GetNamedAttributeValue(*it, "key"), "");
        std::string val = ParseString(GetNamedAttributeValue(*it, "value"), "");
        if (!(key.empty() && val.empty()))
        {
            cfg.genericProps[key] = val;
        }
    }
}

void ParseShortTermConfigFile::processVehicleTypesNode(DOMElement *node)
{
    if(node)
    {
        std::vector<DOMElement*> vehicles = GetElementsByName(node, "vehicleType");
        for(std::vector<DOMElement*>::const_iterator it = vehicles.begin(); it != vehicles.end(); it++)
        {
           VehicleType vehicleType;
           vehicleType.name = ParseString(GetNamedAttributeValue(*it, "name", ""));
           if(vehicleType.name.empty())
           {
               throw std::runtime_error("ProcessVehicleTypesNode : Vehicle name cannot be empty");
           }

           vehicleType.length = ParseFloat(GetNamedAttributeValue(*it, "length"), "4.0");
           vehicleType.width = ParseFloat(GetNamedAttributeValue(*it, "width"), "2.0");
           vehicleType.capacity = ParseInteger(GetNamedAttributeValue(*it, "capacity"), "4");
           splitRoleString(ParseString(GetNamedAttributeValue(*it, "associated_roles"), ""), vehicleType.associatedRoles);

           cfg.vehicleTypes.push_back(vehicleType);
        }

        if(cfg.vehicleTypes.empty())
        {
            throw std::runtime_error("ProcessVehicleTypesNode : No vehicle type is defined");
        }
    }
    else
    {
        throw std::runtime_error("ProcessVehicleTypeNode : VehicleTypes node not defined in the configuration");
    }
}

void ParseShortTermConfigFile::processTripFilesNode(DOMElement *node)
{
    if(node)
    {
        std::vector<DOMElement*> tripFiles = GetElementByName(node, "tripFile");
        for(std::vector<DOMElement*>::const_iterator it = tripFiles.begin(); it != tripFiles.end(); it++)
        {
            std::string name = ParseString(GetNamedAttributeValue(*it, "name"), "");
            std::string file = ParseString(GetNamedAttributeValue(*it, "fileName"), "");
            if (!(name.empty() && file.empty()))
            {
                cfg.tripFiles[name] = file;
            }
        }

        for(std::map<string, string>::const_iterator it = cfg.tripFiles.begin(); it != cfg.tripFiles.end(); it++)
        {
            ParseShortTermTripFile parse(it->second, it->first, cfg);
        }
    }
}

void ParseShortTermConfigFile::processBusControllerNode(DOMElement *node)
{
    if(node)
    {
        ConfigParams& sharedConfig = ConfigManager::GetInstanceRW().FullConfig();
        sharedConfig.busController.enabled = ParseBoolean(GetNameAttributeValue(node, "enabled"), "false");
        sharedConfig.busController.busLineControlType = ParseString(GetNamedAttributeValue(node, "busline_control_type"), "");
    }
}

void ParseShortTermConfigFile::processPathSetFileName(DOMElement* node)
{
    if (!node) {
        return;
    }
    ConfigManager::GetInstanceRW().FullConfig().pathsetFile = ParseString(GetNamedAttributeValue(node, "value"));
}

ParseShortTermTripFile::ParseShortTermTripFile(const std::string &tripFileName,
                           const std::string &tripName_,
                           ST_Config &stConfig) :
    ParseConfigXmlBase(tripFileName), cfg(stConfig), tripName(tripName_)
{
    parseXmlAndProcess();
}

void ParseShortTermTripFile::processXmlFile(XercesDOMParser &parser)
{
    DOMElement* rootNode = parser.getDocument()->getDocumentElement();
    ///Verify that the root node is "config"
    if (TranscodeString(rootNode->getTagName()) != "trips")
    {
    throw std::runtime_error("xml parse error: root node must be \"trips\"");
    }

    processTrips(rootNode);
}

void ParseShortTermTripFile::processTrips(DOMElement *node)
{
    if(node)
    {
    std::vector<DOMElement*> trips = GetElementByName(node, "trip");
    for(std::vector<DOMElement*>::const_iterator it = trips.begin(); it != trips.end(); it++)
    {
        unsigned int tripId = ParseInteger(GetNamedAttributeValue(*it, "id"), "0");
        unsigned int personId = ParseUnsignedInt(GetNamedAttributeValue(item, "personId", false), static_cast<unsigned int>(0));
        std::vector<DOMElement*> subTrips = GetElementsByName((*it), "subTrip");
        for(std::vector<DOMElement*> stIter = subTrips.begin(); stIter != subTrips.end(); stIter++)
        {
            EntityTemplate ent;
            ent.originPos = ParsePoint2D(GetNamedAttributeValue(stIter, "originPos", originReq),Point2D());
            ent.destPos = ParsePoint2D(GetNamedAttributeValue(stIter, "destPos", destReq), Point2D());
            ent.startTimeMs = ParseUnsignedInt(GetNamedAttributeValue(stIter, "time", timeReq), static_cast<unsigned int>(0));
            ent.laneIndex = ParseUnsignedInt(GetNamedAttributeValue(stIter, "lane", laneReq), static_cast<unsigned int>(0));
            ent.agentId = personId;
            ent.initSegId = ParseUnsignedInt(GetNamedAttributeValue(stIter, "initSegId", false), static_cast<unsigned int>(0));
            ent.initDis = ParseUnsignedInt(GetNamedAttributeValue(stIter, "initDis", false), static_cast<unsigned int>(0));
            ent.initSpeed = ParseUnsignedInt(GetNamedAttributeValue(stIter, "initSpeed", false), static_cast<double>(0));
            ent.originNode = ParseUnsignedInt(GetNamedAttributeValue(stIter, "originNode", false), static_cast<double>(0));
            ent.destNode = ParseUnsignedInt(GetNamedAttributeValue(stIter, "destNode", false), static_cast<double>(0));
            unsigned int stId = ParseUnsignedInt(GetNamedAttributeValue(stIter, "id", false), static_cast<unsigned int>(0));
            ent.tripId = std::make_pair(tripId, stId);
            ent.vehicleType = ParseString(GetNamedAttributeValue(stIter, "vehicleType"), "");
            if(ent.vehicleType.empty() || cfg.vehicleTypes.find(ent.vehicleType) == cfg.vehicleTypes.end())
            {
                throw std::runtime_error("ProcessTrips : Unknown vehicle type");
            }
            ent.roleName = ParseString(GetNamedAttributeValue(stIter, "roleName"), "");
            if(ent.roleName.empty() || !(cfg.vehicleTypes.find(ent.vehicleType))->isValidRole(ent.roleName))
            {
                throw std::runtime_error("ProcessTrips : Invalid Role Name");
            }
            cfg.futureAgents[tripName] = ent;
        }
    }
    }
}

}
