#include "ExpandShortTermConfigFile.hpp"
#include "conf/PrintNetwork.hpp"
#include "entities/BusController.hpp"
#include "entities/BusControllerST.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/Person_ST.hpp"

namespace
{

using namespace sim_mob;

//ReactionTimeDist* GenerateReactionTimeDistribution(SimulationParams::ReactionTimeDistDescription rdist)
//{
//    if (rdist.typeId == 0)
//    {
//	return new NormalReactionTimeDist(rdist.mean, rdist.stdev);
//    }
//    else if (rdist.typeId == 1)
//    {
//	return new LognormalReactionTimeDist(rdist.mean, rdist.stdev);
//    }
//    else
//    {
//	throw std::runtime_error("Unknown reaction time magic number.");
//    }
//}

void informLoadOrder(const std::vector<LoadAgentsOrderOption>& order)
{
    std::cout << "Agent Load order: ";
    if (order.empty())
    {
	std::cout << "<N/A>";
    }
    else
    {
	for (std::vector<LoadAgentsOrderOption>::const_iterator it = order.begin(); it != order.end(); ++it)
	{
	    if ((*it) == LoadAg_Drivers)
	    {
		std::cout << "drivers";
	    }
	    else if ((*it) ==  LoadAg_Database)
	    {
		std::cout << "database";
	    }
	    else if ((*it) ==  LoadAg_Pedestrians)
	    {
		std::cout << "pedestrians";
	    }
	    else
	    {
		std::cout << "<unknown>";
	    }
	    std::cout << "  ";
	}
    }
    std::cout << std::endl;
}


////
//// TODO: Eventually, we need to re-write WorkGroup to encapsulate the functionality of "addOrStash()".
////       For now, just make sure that if you add something to all_agents manually, you call "load()" before.
////

void addOrStashEntity(Agent* p, std::set<Entity*>& active_agents, StartTimePriorityQueue& pending_agents)
{
	//Only agents with a start time of zero should start immediately in the all_agents list.
	if (p->getStartTime() == 0)
	{
		active_agents.insert(p);
	}
	else
	{
		//Start later.
		pending_agents.push(p);
	}
}

} //End un-named namespace


namespace sim_mob
{

ExpandShortTermConfigFile::ExpandShortTermConfigFile(ST_Config &stConfig, ConfigParams &cfg,
								     std::set<Entity *> &active_agents,
								     StartTimePriorityQueue &pending_agents) :
    stConfig(stConfig), cfg(cfg),
    active_agents(active_agents), pending_agents(pending_agents)
{
    processConfig();
}

void ExpandShortTermConfigFile::processConfig()
{
    cfg.simMobRunMode = ConfigParams::SimMobRunMode::SHORT_TERM;
    cfg.setWorkerPublisherEnabled(stConfig.commsim.enabled);

    //Inform of load order (drivers, database, pedestrians, etc.).
    informLoadOrder(stConfig.loadAgentsOrder);

    //Print schema file.
    const std::string schem = stConfig.getRoadNetworkXsdSchemaFile();
    Print() << "XML (road network) schema file: " << (schem.empty() ? "<default>" : schem) << std::endl;

    //Load from database or XML.
    loadNetworkFromDatabase();

    //Set PartitionManager instance (if using MPI and it's enabled).
    if (cfg.MPI_Enabled() && cfg.using_MPI)
    {
        int partId = stConfig.partitioningSolutionId;
        PartitionManager::instance().partition_config->partition_solution_id = partId;
        std::cout << "partition_solution_id in configuration:" << partId << std::endl;
    }

    //TEMP: Test network output via boost.
    //todo: enable/disble through cinfig
    //BoostSaveXML(stConfig.getNetworkXmlOutputFile(), cfg.getNetworkRW());

    cfg.sealNetwork();
    std::cout << "Network Sealed" << std::endl;

    //Initialize the street directory.
    StreetDirectory::instance().init(cfg.getNetwork(), true);
    std::cout << "Street Directory initialized  " << std::endl;

    std::map<std::string, std::string>::iterator itIntModel = stConfig.genericProps.find("intersection_driving_model");
    if (itIntModel != stConfig.genericProps.end())
    {
        if (itIntModel->second == "slot-based")
        {
            sim_mob::aimsun::Loader::CreateIntersectionManagers(cfg.getNetwork());
        }
    }

    loadAMOD_Controller();

    //Load Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder
    loadAgentsInOrder(constraints);

    //Load signals, which are currently agents
    generateXMLSignals();

    //register and initialize BusController
	if (cfg.busController.enabled)
	{
		sim_mob::BusControllerST::RegisterBusController(-1, cfg.mutexStategy());
		sim_mob::BusController* busController = sim_mob::BusController::GetInstance();
		busController->initializeBusController(active_agents);
		active_agents.insert(busController);
	}
}

void ExpandShortTermConfigFile::loadNetworkFromDatabase()
{
    //Load from the database or from XML, depending.
    if (stConfig.networkSource == NETSRC_DATABASE)
    {
		std::cout << "Loading Road Network from the database.\n";
		sim_mob::aimsun::Loader::LoadNetwork(cfg.getDatabaseConnectionString(false),
							 cfg.getDatabaseProcMappings().procedureMappings,
							 cfg.getNetworkRW(), cfg.getTripChains(), nullptr);
    }
    else
    {
        std::cout << "Loading Road Network from XML.\n";
        if (!sim_mob::xml::InitAndLoadXML(stConfig.getNetworkXmlInputFile(), cfg.getNetworkRW(), cfg.getTripChains()))
        {
            throw std::runtime_error("Error loading/parsing XML file (see stderr).");
        }
    }
}

void ExpandShortTermConfigFile::loadAMOD_Controller()
{
    if (stConfig.amod.enabled)
    {
    	sim_mob::AMOD::AMODController::registerController(-1, cfg.mutexStategy());
    }
}

void ExpandShortTermConfigFile::loadAgentsInOrder(ConfigParams::AgentConstraints& constraints)
{
    typedef std::vector<LoadAgentsOrderOption> LoadOrder;
    const LoadOrder& order = stConfig.loadAgentsOrder;
    for (LoadOrder::const_iterator it = order.begin(); it != order.end(); ++it)
    {
        switch (*it)
        {
        case LoadAg_Database: //fall-through
        case LoadAg_XmlTripChains:
            //Create an agent for each Trip Chain in the database.
            generateAgentsFromTripChain(constraints);
            std::cout << "Loaded Database Agents (from Trip Chains).\n";
            break;
        case LoadAg_Drivers:
            for(std::map<std::string, std::vector<EntityTemplate> >::const_iterator it = stConfig.futureAgents.begin();
            it != stConfig.futureAgents.end(); it++)
            {
                generateXMLAgents(it->second);
            }
            std::cout << "Loaded Driver Agents (from config file).\n";
            break;
        case LoadAg_Pedestrians:
            generateXMLAgents(stConfig.futureAgents["pedestrian"]);
            std::cout << "Loaded Pedestrian Agents (from config file).\n";
            break;
        case LoadAg_Passengers:
            generateXMLAgents(stConfig.futureAgents["passenger"]);
            std::cout << "Loaded Passenger Agents (from config file).\n";
            break;
        default:
            throw std::runtime_error("Unknown item in load_agents");
        }
    }
    std::cout << "Loading Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder: Success!\n";
}

void ExpandShortTermConfigFile::generateAgentsFromTripChain(ConfigParams::AgentConstraints &constraints)
{
    //NOTE: "constraints" are not used here, but they could be (for manual ID specification).
    typedef std::map<std::string, std::vector<TripChainItem*> > TripChainMap;
    const TripChainMap& tcs = cfg.getTripChains();

    //The current agent we are working on.
    for (TripChainMap::const_iterator it_map = tcs.begin(); it_map != tcs.end(); ++it_map)
    {
        TripChainItem *tc = it_map->second.front();
        Person *person = new Person_ST("XML_TripChain", cfg.mutexStategy(), it_map->second);
        person->setPersonCharacteristics();
        addOrStashEntity(person, active_agents, pending_agents);
    }
}

void ExpandShortTermConfigFile::generateXMLAgents(const std::vector<EntityTemplate>& xmlItems)
{
    //Do nothing for empty roles.
    if (xmlItems.empty())
    {
	    return;
    }

    //Loop through all agents of this type.
    for (std::vector<EntityTemplate>::const_iterator it = xmlItems.begin(); it != xmlItems.end(); ++it)
    {
        //Keep track of the properties we have found.
        std::map<std::string, std::string> props;
        props["lane"] = Utils::toStr<unsigned int>(it->laneIndex);
        props["initSegId"] = Utils::toStr<unsigned int>(it->initSegId);
        props["initDis"] = Utils::toStr<unsigned int>(it->initDis);
        props["initSpeed"] = Utils::toStr<unsigned int>(it->initSpeed);

        if (it->originNode > 0 && it->destNode > 0)
        {
            props["originNode"] = Utils::toStr<unsigned int>(it->originNode);
            props["destNode"] = Utils::toStr<unsigned int>(it->destNode);
        }
        else
        {
            {
                std::stringstream msg;
                msg << it->originPos.getX() << "," << it->originPos.getY();
                props["originPos"] = msg.str();
            }
            {
                std::stringstream msg;
                msg << it->destPos.getX() << "," << it->destPos.getY();
                props["destPos"] = msg.str();
            }
        }

        int agentId = -1;

        if (it->agentId != 0)
        {
            agentId = it->agentId;
        }

        props["#mode"] = it->mode;

        //Create the Person agent with that given ID (or an auto-generated one)
        Person* agent = new Person_ST("XML_Def", cfg.mutexStategy(), agentId);
        agent->setConfigProperties(props);
        agent->setStartTime(it->startTimeMs);

        //Add it or stash it
        addOrStashEntity(agent, active_agents, pending_agents);
    }
}

void ExpandShortTermConfigFile::generateXMLSignals()
{
    if (stConfig.futureAgents["signal"].empty())
    {
    	return;
    }
    StreetDirectory& streetDirectory = StreetDirectory::instance();

    //Loop through all agents of this type
    for (std::vector<EntityTemplate>::const_iterator it = stConfig.futureAgents["signal"].begin(); it != stConfig.futureAgents["signal"].end(); ++it)
    {
	//Find the nearest Node for this Signal.
	Node* road_node = cfg.getNetwork().locateNode(it->originPos, true);
	if (!road_node)
	{
	    Warn() << "xpos=\"" << it->originPos.getX() << "\" and ypos=\"" << it->originPos.getY()
		    << "\" are not suitable attributes for Signal because there is no node there; correct the config file."
		    << std::endl;
	    continue;
	}

	// See the comments in createSignals() in geospatial/aimsun/Loader.cpp.
	// At some point in the future, this function loadXMLSignals() will be removed
	// in its entirety, not just the following code fragment.
	std::set<const Link*> links;
	if (MultiNode const * multi_node = dynamic_cast<MultiNode const *> (road_node))
	{
	    std::set<RoadSegment*> const & roads = multi_node->getRoadSegments();
	    std::set<RoadSegment*>::const_iterator iter;
	    for (iter = roads.begin(); iter != roads.end(); ++iter)
	    {
		RoadSegment const * road = *iter;
		links.insert(road->getLink());
	    }
	}

	if (links.size() != 4)
	{
	    Warn() << "the multi-node at " << it->originPos << " does not have 4 links; "
		    << "no signal will be created here." << std::endl;
	    continue;
	}

	const Signal* signal = streetDirectory.signalAt(*road_node);
	if (signal)
	{
	    Warn() << "signal at node(" << it->originPos << ") already exists; "
		    << "skipping this config file entry" << std::endl;
	}
	else
	{
	    Warn() << "signal at node(" << it->originPos << ") was not found; No more action will be taken\n ";
	    //          // The following call will create and register the signal with the
	    //          // street-directory.
	    //          std::cout << "register signal again!" << std::endl;
	    //          Signal::signalAt(*road_node, ConfigParams::GetInstance().mutexStategy);
	}
    }
}

void ExpandShortTermConfigFile::CheckGranularities()
{
	//Granularity check
    const unsigned int baseGranMS = cfg.simulation.baseGranMS;
    const WorkerParams& workers = stConfig.workers;

    if (cfg.simulation.totalRuntimeMS < baseGranMS)
    {
	    throw std::runtime_error("Total Runtime cannot be smaller than base granularity.");
    }
    if (cfg.simulation.totalWarmupMS != 0 && cfg.simulation.totalWarmupMS < baseGranMS)
    {
	    Warn() << "Warning! Total Warmup is smaller than base granularity.\n";
    }
    if (workers.person.granularityMs < baseGranMS)
    {
	    throw std::runtime_error("Person granularity cannot be smaller than base granularity.");
    }
    if (workers.signal.granularityMs < baseGranMS)
    {
	    throw std::runtime_error("Signal granularity cannot be smaller than base granularity.");
    }
    if (workers.intersectionMgr.granularityMs < baseGranMS)
    {
	    throw std::runtime_error("Intersection Manager granularity cannot be smaller than base granularity.");
    }
    if (workers.communication.granularityMs < baseGranMS)
    {
	    throw std::runtime_error("Communication granularity cannot be smaller than base granularity.");
    }
}

bool ExpandShortTermConfigFile::SetTickFromBaseGran(unsigned int& res, unsigned int tickLenMs)
{
	res = tickLenMs / cfg.simulation.baseGranMS;
	return tickLenMs % cfg.simulation.baseGranMS == 0;
}

void ExpandShortTermConfigFile::SetTicks()
{
    if (!SetTickFromBaseGran(cfg.totalRuntimeTicks, cfg.simulation.totalRuntimeMS))
    {
	Warn() << "Total runtime will be truncated by the base granularity\n";
    }
    if (!SetTickFromBaseGran(cfg.totalWarmupTicks, cfg.simulation.totalWarmupMS))
    {
	Warn() << "Total warm-up will be truncated by the base granularity\n";
    }
    if (!SetTickFromBaseGran(stConfig.granPersonTicks, stConfig.workers.person.granularityMs))
    {
	throw std::runtime_error("Person granularity not a multiple of base granularity.");
    }
    if (!SetTickFromBaseGran(stConfig.granSignalsTicks, stConfig.workers.signal.granularityMs))
    {
	throw std::runtime_error("Signal granularity not a multiple of base granularity.");
    }
    if (!SetTickFromBaseGran(stConfig.granIntMgrTicks, stConfig.workers.intersectionMgr.granularityMs))
    {
	    throw std::runtime_error("Signal granularity not a multiple of base granularity.");
    }
    if (!SetTickFromBaseGran(stConfig.granCommunicationTicks, stConfig.workers.communication.granularityMs))
    {
	    throw std::runtime_error("Communication granularity not a multiple of base granularity.");
    }
}

void ExpandShortTermConfigFile::PrintSettings()
{
    std::cout << "Config parameters:\n";
    std::cout << "------------------\n";

    //Print the WorkGroup strategy.
    std::cout << "WorkGroup assignment: ";
    switch (cfg.defaultWrkGrpAssignment())
    {
        case WorkGroup::ASSIGN_ROUNDROBIN:
        std::cout << "roundrobin" << std::endl;
        break;
        case WorkGroup::ASSIGN_SMALLEST:
        std::cout << "smallest" << std::endl;
        break;
        default:
        std::cout << "<unknown>" << std::endl;
        break;
    }

    //Basic statistics
    std::cout << "  Base Granularity: " << cfg.baseGranMS() << " " << "ms" << "\n";
    std::cout << "  Total Runtime: " << cfg.totalRuntimeTicks << " " << "ticks" << "\n";
    std::cout << "  Total Warmup: " << cfg.totalWarmupTicks << " " << "ticks" << "\n";
    //std::cout << "  Person Granularity: " << cfg.granPersonTicks << " " << "ticks" << "\n";
    std::cout << "  Start time: " << cfg.simStartTime().getStrRepr() << "\n";
    std::cout << "  Mutex strategy: " << (cfg.mutexStategy() == MtxStrat_Locked ? "Locked" : cfg.mutexStategy() == MtxStrat_Buffered ? "Buffered" : "Unknown") << "\n";

    //Output Database details
    if (stConfig.networkSource == NETSRC_XML)
    {
        std::cout << "Network details loaded from xml file: " << stConfig.networkXmlInputFile << "\n";
    }
    if (stConfig.networkSource == NETSRC_DATABASE)
    {
        std::cout << "Network details loaded from database connection: " << cfg.getDatabaseConnectionString() << "\n";
    }

    //Print the network (this will go to a different output file...)
    std::cout << "------------------\n";
    PrintNetwork(cfg, cfg.outNetworkFileName);
    std::cout << "------------------\n";
}

}
