#include "ExpandShortTermConfigFile.hpp"
#include "conf/NetworkPrinter.hpp"
#include "conf/SimulationInfoPrinter.hpp"
#include "entities/amodController/AMODController.hpp"
#include "entities/BusController.hpp"
#include "entities/BusControllerST.hpp"
#include "entities/fmodController/FMOD_Controller.hpp"
#include "entities/IntersectionManager.hpp"
#include "entities/Person_ST.hpp"
#include "entities/signal/Signal.hpp"
#include "geospatial/network/SOCI_Converters.hpp"
#include "geospatial/streetdir/KShortestPathImpl.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "partitions/PartitionManager.hpp"
#include "path/PT_PathSetManager.hpp"
#include "util/Utils.hpp"
#include "geospatial/streetdir/KShortestPathImpl.hpp"

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
	subTrip.travelMode = mode;
	subTrip.ptLineId = "";

	//Add it to the Trip; return this value.
	res->addSubTrip(subTrip);
	return res;
}

Trip* makeTrip(soci::row &r)
{
	//Create a new trip
	Trip *trip = new Trip();
	trip->itemType = TripChainItem::IT_TRIP;
	trip->tripID = r.get<string>(0);
	trip->setPersonID(r.get<string>(1));
	trip->startTime = DailyTime(r.get<string>(2));
	trip->originType = (TripChainItem::LocationType)r.get<int>(5);
	trip->destinationType = (TripChainItem::LocationType)r.get<int>(6);
	trip->startLocationId = r.get<string>(7);
	trip->endLocationId = r.get<string>(8);
	trip->load_factor = r.get<unsigned int>(9);
	trip->travelMode = r.get<string>(11);
	
	if (trip->originType == TripChainItem::LT_NODE)
	{
		unsigned int nodeId = atoi(trip->startLocationId.c_str());
		const RoadNetwork *rdNetwork = RoadNetwork::getInstance();

		trip->origin = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	}
	else if (trip->originType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		trip->origin = WayPoint(BusStop::findBusStop(trip->startLocationId));
	}

	if (trip->destinationType == TripChainItem::LT_NODE)
	{
		unsigned int nodeId = atoi(trip->endLocationId.c_str());
		const RoadNetwork *rdNetwork = RoadNetwork::getInstance();

		trip->destination = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	}
	else if (trip->destinationType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		trip->destination = WayPoint(BusStop::findBusStop(trip->endLocationId));
	}
	
	//Create the sub-trip with-in the trip
	SubTrip subtrip;
	subtrip.itemType = TripChainItem::IT_TRIP;
	subtrip.tripID = trip->tripID;
	subtrip.startTime = trip->startTime;
	subtrip.originType = trip->originType;
	subtrip.origin = trip->origin;
	subtrip.sequenceNumber = r.get<unsigned int>(10);
	subtrip.travelMode = r.get<string>(11);
	subtrip.ptLineId = r.get<string>(12);
	subtrip.cbdTraverseType = (TravelMetric::CDB_TraverseType)r.get<int>(13);
	subtrip.destinationType = (TripChainItem::LocationType)r.get<int>(15);
	subtrip.endLocationId = r.get<string>(17);
	
	if (subtrip.destinationType == TripChainItem::LT_NODE)
	{
		unsigned int nodeId = atoi(subtrip.endLocationId.c_str());
		const RoadNetwork *rdNetwork = RoadNetwork::getInstance();

		subtrip.destination = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	}
	else if (subtrip.destinationType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		subtrip.destination = WayPoint(BusStop::findBusStop(subtrip.endLocationId));
	}
	
	//Add the sub-trip
	trip->addSubTrip(subtrip);
	
	return trip;
}

Activity* makeActivity(soci::row &r)
{
	Activity *activity = new Activity();
	activity->itemType = TripChainItem::IT_ACTIVITY;
	activity->setPersonID(r.get<string>(1));
	activity->startTime = DailyTime(r.get<string>(3));
	activity->endTime = DailyTime(r.get<string>(4));
	activity->destinationType = (TripChainItem::LocationType)r.get<int>(6);
	activity->endLocationId = r.get<string>(8);
	
	if (activity->destinationType == TripChainItem::LT_NODE)
	{
		unsigned int nodeId = atoi(activity->endLocationId.c_str());
		const RoadNetwork *rdNetwork = RoadNetwork::getInstance();
		activity->destination = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	}
	else if (activity->destinationType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		activity->destination = WayPoint(BusStop::findBusStop(activity->endLocationId));
	}
	
	return activity;
}

SubTrip* makeSubTrip(soci::row &r)
{
	SubTrip *subtrip = new SubTrip();
	subtrip->itemType = TripChainItem::IT_TRIP;
	subtrip->tripID = r.get<string>(0);	
	subtrip->sequenceNumber = r.get<unsigned int>(10);
	subtrip->travelMode = r.get<string>(11);
	subtrip->ptLineId = r.get<string>(12);
	subtrip->cbdTraverseType = (TravelMetric::CDB_TraverseType)r.get<int>(13);
	subtrip->originType = (TripChainItem::LocationType)r.get<int>(14);
	subtrip->destinationType = (TripChainItem::LocationType)r.get<int>(15);
	subtrip->startLocationId = r.get<string>(16);
	subtrip->endLocationId = r.get<string>(17);
	
	if (subtrip->originType == TripChainItem::LT_NODE)
	{
		unsigned int nodeId = atoi(subtrip->startLocationId.c_str());
		const RoadNetwork *rdNetwork = RoadNetwork::getInstance();

		subtrip->origin = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	}
	else if (subtrip->originType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		subtrip->origin = WayPoint(BusStop::findBusStop(subtrip->startLocationId));
	}

	if (subtrip->destinationType == TripChainItem::LT_NODE)
	{
		unsigned int nodeId = atoi(subtrip->endLocationId.c_str());
		const RoadNetwork *rdNetwork = RoadNetwork::getInstance();

		subtrip->destination = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	}
	else if (subtrip->destinationType == TripChainItem::LT_PUBLIC_TRANSIT_STOP)
	{
		subtrip->destination = WayPoint(BusStop::findBusStop(subtrip->endLocationId));
	}
	
	return subtrip;
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
	//Instantiating K_ShortestPathImpl before any thread is spawned (in path-set generation)
	K_ShortestPathImpl::getInstance();
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
		amod::AMODController::registerController(-1, cfg.mutexStategy());
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
	soci::session sql;
	const map<string, string> &storedProcs = cfg.getDatabaseProcMappings().procedureMappings;
	
	map<string, string>::const_iterator itProcedure = storedProcs.find("trip_chains");
	
	if(itProcedure != storedProcs.end())
	{
		//Mapping between person id, trip id and the trip-chain
		//map<personId, map<tripId, trip-chain> >
		map<const string, map<const string, vector<TripChainItem *> > > tripChains;
		
		//Mapping between person id and person
		map<const string, Person_ST *> persons;
		
		stringstream query;
		query << "select * from " << itProcedure->second << "('" << cfg.simulation.simStartTime.getStrRepr() << "','" 
				<< (DailyTime(cfg.simulation.totalRuntimeMS) + cfg.simulation.simStartTime).getStrRepr() << "')";
		
		//Connect to the db and retrieve the trip chains
		sql.open(soci::postgresql, cfg.getDatabaseConnectionString(false));

		soci::rowset<soci::row> rows = (sql.prepare << query.str());

		for (soci::rowset<soci::row>::const_iterator itRows = rows.begin(); itRows != rows.end(); ++itRows)
		{
			//Load only if the load factor is 1
			if((*itRows).get<unsigned int>(9) == 1)
			{
				//Get the person id and trip id
				const string tripId = (*itRows).get<string>(0);
				const string personId = (*itRows).get<string>(1);

				//The trip belonging to the person
				vector<TripChainItem *> &personTripChain = tripChains[personId][tripId];

				if(personTripChain.empty())
				{
					//Create a trip and the corresponding activity
					Trip *trip = makeTrip(*itRows);
					personTripChain.push_back(trip);

					//Create activity if end time is provided
					if(!(*itRows).get<string>(4).empty())
					{
						Activity *activity = makeActivity(*itRows);
						personTripChain.push_back(activity);
					}
				}
				else
				{
					//Create a sub-trip and add it to the trip
					SubTrip *subTrip = makeSubTrip(*itRows);

					Trip *trip = dynamic_cast<Trip *>(personTripChain[0]);
					trip->addSubTrip(*subTrip);
				}
				
				//If the person is not created, create it and add it to the map
				if(persons.find(personId) == persons.end())
				{
					Person_ST *person = new Person_ST("XML_TripChain", cfg.mutexStategy(), -1);
					person->setDatabaseId(personId);
					//Set the usage of in-simulation travel times
					//Generate random number between 0 and 100 (indicates percentage)
					int randomInt = Utils::generateInt(0, 100);

					if(randomInt <= cfg.simulation.inSimulationTTUsage)
					{
						person->setUseInSimulationTravelTime(true);
					}

					persons.insert(make_pair(personId, person));
				}
			}
		}
		
		//Close the connection
		//sql.close();
		
		//For every person, consolidate the the trips, set and initialise the trip chain and add/stash the person
		for(map<const string, Person_ST *>::iterator itPerson = persons.begin(); itPerson != persons.end(); ++itPerson)
		{
			map<const string, vector<TripChainItem *> > &personTripChains = tripChains[itPerson->first];
			
			std::vector<TripChainItem *> personTripChain;
			
			for(map<const string, vector<TripChainItem *> >::iterator itPersonTripChains = personTripChains.begin(); 
					itPersonTripChains != personTripChains.end(); ++itPersonTripChains)
			{
				personTripChain.insert(personTripChain.end(), itPersonTripChains->second.begin(), itPersonTripChains->second.end());
			}
			
			itPerson->second->setTripChain(personTripChain);
			itPerson->second->setNextPathPlanned(false);
			
			//Add it or stash it
			addOrStashEntity(itPerson->second, active_agents, pending_agents);
		}
	}
	else
	{
		throw std::runtime_error("Stored-procedure 'trip_chains' not found in the configuration file");
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
	
	//Set the usage of in-simulation travel times
	int randomInt = Utils::generateInt(0, 100);

	if (randomInt < cfg.simulation.inSimulationTTUsage)
	{
		person->setUseInSimulationTravelTime(true);
	}
	
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
	SimulationInfoPrinter siPrinter(cfg, cfg.outSimInfoFileName);
	siPrinter.printSimulationInfo();
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
			out << "\"id\":\"" << it->second->getTrafficLightId() << "\",";			
			out << "}}\n";
		}
	}

	return out.str();
}

}
