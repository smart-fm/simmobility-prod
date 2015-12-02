#include "ExpandShortTermConfigFile.hpp"
#include "conf/NetworkPrinter.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/BusController.hpp"
#include "entities/BusControllerST.hpp"
#include "entities/fmodController/FMOD_Controller.hpp"
#include "entities/IntersectionManager.hpp"
#include "entities/Person_ST.hpp"
#include "entities/signal/Signal.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "partitions/PartitionManager.hpp"
#include "path/PT_PathSetManager.hpp"
#include "util/Utils.hpp"

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
			else if ((*it) == LoadAg_Database)
			{
				std::cout << "database";
			}
			else if ((*it) == LoadAg_Pedestrians)
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

Trip* MakePseudoTrip(unsigned int personId, const Node *origin, const Node *destination, unsigned int startTimeMS, const std::string &mode)
{
	//Make sure we have something to work with
	if (!(origin && destination))
	{
		std::stringstream msg;
		msg << "Can't make a pseudo-trip for an Agent with no origin and destination nodes!";
		throw std::runtime_error(msg.str().c_str());
	}

	//Make the trip itself
	Trip *res = new Trip();
	res->setPersonID(personId);
	res->itemType = TripChainItem::getItemType("Trip");
	res->sequenceNumber = 1;
	res->startTime = DailyTime(startTimeMS);
	res->endTime = res->startTime; //No estimated end time.
	res->tripID = "";
	res->origin = WayPoint(origin);
	res->originType = TripChainItem::getLocationType("node");
	res->destination = WayPoint(destination);
	res->destinationType = res->originType;
	res->travelMode = mode;

	//Make and assign a single sub-trip
	SubTrip subTrip;
	subTrip.setPersonID(-1);
	subTrip.itemType = TripChainItem::getItemType("Trip");
	subTrip.sequenceNumber = 1;
	subTrip.startTime = res->startTime;
	subTrip.endTime = res->startTime;
	subTrip.origin = res->origin;
	subTrip.originType = res->originType;
	subTrip.destination = res->destination;
	subTrip.destinationType = res->destinationType;
	subTrip.tripID = "";
	subTrip.mode = mode;
	subTrip.isPrimaryMode = true;
	subTrip.ptLineId = "";

	//Add it to the Trip; return this value.
	res->addSubTrip(subTrip);
	return res;
}

} //End un-named namespace


namespace sim_mob
{

ExpandShortTermConfigFile::ExpandShortTermConfigFile(ST_Config &stConfig, ConfigParams &cfg, std::set<Entity *> &active_agents,
													 StartTimePriorityQueue &pending_agents) :
stConfig(stConfig), cfg(cfg), active_agents(active_agents), pending_agents(pending_agents)
{
	processConfig();
}

void ExpandShortTermConfigFile::processConfig()
{
	cfg.simMobRunMode = ConfigParams::SimMobRunMode::SHORT_TERM;
	cfg.setWorkerPublisherEnabled(stConfig.commsim.enabled);

	//Inform of load order (drivers, database, pedestrians, etc.).
	informLoadOrder(stConfig.loadAgentsOrder);

	//Ensure granularities are multiples of each other. Then set the "ticks" based on each granularity.
	checkGranularities();
	setTicks();

	//Print schema file.
	const std::string schem = stConfig.getRoadNetworkXsdSchemaFile();
	Print() << "XML (road network) schema file: " << (schem.empty() ? "<default>" : schem) << std::endl;

	//Load from database or XML.
	loadNetworkFromDatabase();
	
	//Load travel times if path-set is enabled
	if (cfg.PathSetMode())
	{
		TravelTimeManager::getInstance()->loadTravelTimes();
	}

	//Set PartitionManager instance (if using MPI and it's enabled).
	if (cfg.MPI_Enabled() && cfg.using_MPI)
	{
		int partId = stConfig.partitioningSolutionId;
		PartitionManager::instance().partition_config->partition_solution_id = partId;
		std::cout << "partition_solution_id in configuration:" << partId << std::endl;
	}

	cfg.sealNetwork();
	std::cout << "Network Sealed" << std::endl;

	//Initialize the street directory.
	StreetDirectory::Instance().Init(*(RoadNetwork::getInstance()));
	std::cout << "Street Directory initialized  " << std::endl;

	std::map<std::string, std::string>::iterator itIntModel = stConfig.genericProps.find("intersection_driving_model");
	if (itIntModel != stConfig.genericProps.end())
	{
		if (itIntModel->second == "slot-based")
		{
			IntersectionManager::CreateIntersectionManagers(cfg.mutexStategy());
		}
	}	

	if (cfg.PathSetMode() && cfg.getPathSetConf().privatePathSetMode == "generation")
	{
		Profiler profile("bulk profiler start", true);

		//This mode can be executed in the main function also but we need the street directory to be initialised first
		//to be least intrusive to the rest of the code, we take a safe approach and run this mode from here, although a lot of
		//unnecessary code will be executed.
		PrivatePathsetGenerator::getInstance()->bulkPathSetGenerator();

		Print() << "Private traffic path-set generation done (in " << (profile.tick().first.count() / 1000000.0) << "s)" << std::endl;
		exit(1);
	}

	if (cfg.PathSetMode() && cfg.getPathSetConf().publicPathSetMode == "generation")
	{
		Profiler profile("bulk profiler start", true);

		PT_PathSetManager::Instance().PT_BulkPathSetGenerator();

		Print() << "Public transit path-set generation done (in " << (profile.tick().first.count() / 1000000.0) << "s)" << std::endl;
		exit(1);
	}

	//Maintain unique/non-colliding IDs
	ConfigParams::AgentConstraints constraints;
	constraints.startingAutoAgentID = cfg.simulation.startingAutoAgentID;

	loadAMOD_Controller();
	loadFMOD_Controller();

	//Load Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder
	loadAgentsInOrder(constraints);

	//Register and initialize BusController
	if (cfg.busController.enabled)
	{
		BusControllerST::RegisterBusController(-1, cfg.mutexStategy());
		BusController* busController = BusController::GetInstance();
		busController->initializeBusController(active_agents);
		active_agents.insert(busController);
	}

	printSettings();
}

void ExpandShortTermConfigFile::loadNetworkFromDatabase()
{
	//Load from the database or from XML
	if (ST_Config::getInstance().networkSource == NetworkSource::NETSRC_DATABASE)
	{
		Print() << "Loading Road Network from the database.\n";

		//The instance of the network loader
		NetworkLoader *loader = NetworkLoader::getInstance();

		//Load the road network
		loader->loadNetwork(cfg.getDatabaseConnectionString(false), cfg.getDatabaseProcMappings().procedureMappings);

		//Post processing on the network
		loader->processNetwork();

		//Create traffic signals
		Signal_SCATS::createTrafficSignals(cfg.mutexStategy());
	}
	else
	{
		Print() << "Loading Road Network from XML not yet implemented\n";
		exit(-1);
	}
}

void ExpandShortTermConfigFile::loadAMOD_Controller()
{
	if (stConfig.amod.enabled)
	{
		AMOD::AMODController::registerController(-1, cfg.mutexStategy());
	}
}

void ExpandShortTermConfigFile::loadFMOD_Controller()
{
	if (stConfig.fmod.enabled)
	{
		FMOD::FMOD_Controller::registerController(-1, cfg.mutexStategy());
		FMOD::FMOD_Controller::instance()->settings(stConfig.fmod.ipAddress, stConfig.fmod.port, stConfig.fmod.updateTimeMS, stConfig.fmod.mapfile,
													stConfig.fmod.blockingTimeSec);
		std::map<std::string, TripChainItem*>::iterator it;
		for (it = stConfig.fmod.allItems.begin(); it != stConfig.fmod.allItems.end(); it++)
		{
			FMOD::FMOD_Controller::instance()->insertFmodItems(it->first, it->second);
		}
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
		case LoadAg_Database:
			//Create an agent for each Trip Chain in the database.
			generateAgentsFromTripChain(constraints);
			std::cout << "Loaded agents from the database (Trip Chains).\n";
			break;
		
		case LoadAg_Drivers:
			for (std::map<std::string, std::vector<EntityTemplate> >::const_iterator it = stConfig.futureAgents.begin(); it != stConfig.futureAgents.end(); it++)
			{
				generateXMLAgents(it->second);
			}
			std::cout << "Loaded drivers from the configuration file).\n";
			break;
			
		case LoadAg_Pedestrians:
			//generateXMLAgents(stConfig.futureAgents["pedestrian"]);
			//std::cout << "Loaded pedestrians from the configuration file).\n";
			break;
		
		case LoadAg_Passengers:
			//generateXMLAgents(stConfig.futureAgents["passenger"]);
			//std::cout << "Loaded passengers from the configuration file).\n";
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
	
	int agentId = -1;

	//All the xml items within the vector have the same person id (if set)
	if (xmlItems.begin()->agentId != 0)
	{
		agentId = xmlItems.begin()->agentId;
	}
	
	//Create the Person agent with that given ID (or an auto-generated one)
	Person_ST *person = new Person_ST("XML_Def", cfg.mutexStategy(), agentId);	
	person->setStartTime(xmlItems.begin()->startTimeMs);
	
	std::vector<TripChainItem*> tripChain;
	const RoadNetwork *rn = RoadNetwork::getInstance();

	//Loop through all agents of this type.
	for (std::vector<EntityTemplate>::const_iterator it = xmlItems.begin(); it != xmlItems.end(); ++it)
	{
		std::string mode = it->mode;
		person->startLaneIndex = it->startLaneIndex;
		person->startSegmentId = it->startSegmentId;
		person->segmentStartOffset = it->segmentStartOffset;
		person->initialSpeed = it->initialSpeed;
		
		//Set the origin and destination nodes		
		const Node *originNd = rn->getById(rn->getMapOfIdvsNodes(), it->originNode);
		const Node *destinNd = rn->getById(rn->getMapOfIdvsNodes(), it->destNode);
		
		if(originNd && destinNd)
		{
			//Make the trip item
			Trip *trip = MakePseudoTrip(agentId, originNd, destinNd, it->startTimeMs, mode);
			tripChain.push_back(trip);
		}
		else
		{
			std::stringstream msg;
			msg << "Invalid nodes specified... Origin: " << it->originNode << ", Destination: " << it->destNode;
			throw std::runtime_error(msg.str());
		}
	}
	
	person->setNextPathPlanned(false);
	person->setTripChain(tripChain);
	person->initTripChain();
	
	//Add it or stash it
	addOrStashEntity(person, active_agents, pending_agents);
}

void ExpandShortTermConfigFile::checkGranularities()
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

bool ExpandShortTermConfigFile::setTickFromBaseGran(unsigned int& res, unsigned int tickLenMs)
{
	res = tickLenMs / cfg.simulation.baseGranMS;
	return tickLenMs % cfg.simulation.baseGranMS == 0;
}

void ExpandShortTermConfigFile::setTicks()
{
	if (!setTickFromBaseGran(cfg.totalRuntimeTicks, cfg.simulation.totalRuntimeMS))
	{
		Warn() << "Total runtime will be truncated by the base granularity\n";
	}
	if (!setTickFromBaseGran(cfg.totalWarmupTicks, cfg.simulation.totalWarmupMS))
	{
		Warn() << "Total warm-up will be truncated by the base granularity\n";
	}
	if (!setTickFromBaseGran(stConfig.granPersonTicks, stConfig.workers.person.granularityMs))
	{
		throw std::runtime_error("Person granularity not a multiple of base granularity.");
	}
	if (!setTickFromBaseGran(stConfig.granSignalsTicks, stConfig.workers.signal.granularityMs))
	{
		throw std::runtime_error("Signal granularity not a multiple of base granularity.");
	}
	if (!setTickFromBaseGran(stConfig.granIntMgrTicks, stConfig.workers.intersectionMgr.granularityMs))
	{
		throw std::runtime_error("Signal granularity not a multiple of base granularity.");
	}
	if (!setTickFromBaseGran(stConfig.granCommunicationTicks, stConfig.workers.communication.granularityMs))
	{
		throw std::runtime_error("Communication granularity not a multiple of base granularity.");
	}
}

void ExpandShortTermConfigFile::printSettings()
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
	std::cout << "  Person Granularity: " << stConfig.granPersonTicks << " " << "ticks" << "\n";
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
	NetworkPrinter nwPrinter(cfg, cfg.outNetworkFileName);
	nwPrinter.printSignals(getSignalsInfo(Signal::getMapOfIdVsSignals()));
	nwPrinter.printNetwork(RoadNetwork::getInstance());
	std::cout << "------------------\n";
}

const std::string ExpandShortTermConfigFile::getSignalsInfo(std::map<unsigned int, Signal*>& signals) const
{
	std::stringstream out;

	if (!cfg.OutputDisabled())
	{
		out << std::setprecision(8);

		for (std::map<unsigned int, Signal *>::const_iterator it = signals.begin(); it != signals.end(); ++it)
		{
			out << "{\"TrafficSignal\":" << "{";
			out << "\"id\":\"" << it->second->getNode()->getTrafficLightId() << "\",";
			out << "\"node\":\"" << it->second->getNode()->getNodeId() << "\",";
			out << "}}\n";
		}
	}

	return out.str();
}

}
