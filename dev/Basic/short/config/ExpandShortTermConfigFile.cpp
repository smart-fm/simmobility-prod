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

enum TripChainColumn
{
	COLUMN_TRIP_ID = 0,
	COLUMN_PERSON_ID = 1,
	COLUMN_TRIP_START_TIME = 2,
	COLUMN_ACTIVITY_START_TIME = 3,
	COLUMN_ACTIVITY_END_TIME = 4,
	COLUMN_ORIGIN_TYPE = 5,
	COLUMN_DESTINATION_TYPE = 6,
	COLUMN_ORIGIN_ID = 7,
	COLUMN_DESTINATION_ID = 8,
	COLUMN_LOAD_FACTOR = 9,
	COLUMN_SEQUENCE_NUMBER = 10,
	COLUMN_MODE = 11,
	COLUMN_PT_LINE_ID = 12,
	COLUMN_CBD_TRAVERSE_TYPE = 13,
	COLUMN_SUBTRIP_ORIGIN_TYPE = 14,
	COLUMN_SUBTRIP_DESTINATION_TYPE = 15,
	COLUMN_SUBTRIP_ORIGIN_ID = 16,
	COLUMN_SUBTRIP_DESTINATION_ID = 17
} ;

void informLoadOrder(const std::vector<LoadAgentsOrderOption>& order)
{
	std::cout << "Demand source: ";
	if (order.empty())
	{
		std::cout << "Not specified";
	}
	else
	{
		for (std::vector<LoadAgentsOrderOption>::const_iterator it = order.begin(); it != order.end(); ++it)
		{
			if ((*it) == LoadAg_XML)
			{
				std::cout << "XML Trip file";
			}
			else if ((*it) == LoadAg_Database)
			{
				std::cout << "Trip chains from database";
			}
			else
			{
				std::cout << "<unknown>";
			}
			std::cout << ", ";
		}
	}
	std::cout << std::endl << std::endl;
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
	//Validate origin and destination
	if (origin && destination)
	{
		//Make the trip itself
		Trip *res = new Trip();
		res->setPersonID(personId);
		res->itemType = TripChainItem::getItemType("Trip");
		res->sequenceNumber = 1;
		res->startTime = DailyTime(startTimeMS + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
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
	else
	{
		if(!origin)
		{
			std::stringstream msg;
			msg << "Invalid origin specified for person " << personId;
			throw std::runtime_error(msg.str());
		}
		
		if(!destination)
		{
			std::stringstream msg;
			msg << "Invalid destination specified for person " << personId;
			throw std::runtime_error(msg.str());
		}
	}
}

SubTrip makeSubTrip(soci::row &r)
{
	SubTrip subtrip;
	subtrip.itemType = TripChainItem::IT_TRIP;
	subtrip.tripID = r.get<string>(COLUMN_TRIP_ID);
	subtrip.startTime = DailyTime(r.get<string>(COLUMN_TRIP_START_TIME));
	subtrip.sequenceNumber = r.get<unsigned int>(COLUMN_SEQUENCE_NUMBER);
	subtrip.travelMode = r.get<string>(COLUMN_MODE);
	subtrip.ptLineId = r.get<string>(COLUMN_PT_LINE_ID);
	subtrip.cbdTraverseType = (TravelMetric::CDB_TraverseType)r.get<int>(COLUMN_CBD_TRAVERSE_TYPE);
	subtrip.originType = (TripChainItem::LocationType)r.get<int>(COLUMN_SUBTRIP_ORIGIN_TYPE);
	subtrip.destinationType = (TripChainItem::LocationType)r.get<int>(COLUMN_SUBTRIP_DESTINATION_TYPE);
	subtrip.startLocationId = r.get<string>(COLUMN_SUBTRIP_ORIGIN_ID);
	subtrip.endLocationId = r.get<string>(COLUMN_SUBTRIP_DESTINATION_ID);

	const RoadNetwork *rdNetwork = RoadNetwork::getInstance();

	unsigned int nodeId = atoi(subtrip.startLocationId.c_str());
	subtrip.origin = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	
	//Validate the origin
	if(subtrip.origin.node)
	{
		nodeId = atoi(subtrip.endLocationId.c_str());
		subtrip.destination = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
		
		//Validate the destination
		if(subtrip.destination.node)
		{
			return subtrip;
		}
		else
		{
			std::stringstream msg;
			msg << "Invalid destination specified for sub-trip " << subtrip.tripID << " sequence " << subtrip.sequenceNumber;
			throw std::runtime_error(msg.str());
		}
	}
	else
	{
		std::stringstream msg;
		msg << "Invalid origin specified for sub-trip " << subtrip.tripID << " sequence " << subtrip.sequenceNumber;
		throw std::runtime_error(msg.str());
	}
}

Trip* makeTrip(soci::row &r)
{
	//Create a new trip
	Trip *trip = new Trip();
	trip->itemType = TripChainItem::IT_TRIP;
	trip->tripID = r.get<string>(COLUMN_TRIP_ID);
	trip->setPersonID(r.get<string>(COLUMN_PERSON_ID));
	trip->startTime = DailyTime(r.get<string>(COLUMN_TRIP_START_TIME));
	trip->originType = (TripChainItem::LocationType)r.get<int>(COLUMN_ORIGIN_TYPE);
	trip->destinationType = (TripChainItem::LocationType)r.get<int>(COLUMN_DESTINATION_TYPE);
	trip->startLocationId = r.get<string>(COLUMN_ORIGIN_ID);
	trip->endLocationId = r.get<string>(COLUMN_DESTINATION_ID);
	trip->load_factor = r.get<unsigned int>(COLUMN_LOAD_FACTOR);
	trip->travelMode = r.get<string>(COLUMN_MODE);
	
	const RoadNetwork *rdNetwork = RoadNetwork::getInstance();
	const Node *node = nullptr;
	
	unsigned int nodeId = atoi(trip->startLocationId.c_str());
	trip->origin = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	
	//Validate the origin
	if(trip->origin.node)
	{
		nodeId = atoi(trip->endLocationId.c_str());
		trip->destination = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
		
		//Validate the destination
		if(trip->destination.node)
		{
			//Create the sub-trip with-in the trip
			SubTrip subtrip = makeSubTrip(r);

			//Add the sub-trip
			trip->addSubTrip(subtrip);

			return trip;
		}
		else
		{
			delete trip;
			
			std::stringstream msg;
			msg << "Invalid destination specified for trip " << trip->tripID;
			throw std::runtime_error(msg.str());
		}
	}
	else
	{
		delete trip;
		
		std::stringstream msg;
		msg << "Invalid origin specified for trip " << trip->tripID;
		throw std::runtime_error(msg.str());
	}
}

Activity* makeActivity(soci::row &r)
{
	Activity *activity = new Activity();
	activity->itemType = TripChainItem::IT_ACTIVITY;
	activity->setPersonID(r.get<string>(COLUMN_PERSON_ID));
	activity->startTime = DailyTime(r.get<string>(COLUMN_ACTIVITY_START_TIME));
	activity->endTime = DailyTime(r.get<string>(COLUMN_ACTIVITY_END_TIME));
	activity->destinationType = (TripChainItem::LocationType)r.get<int>(COLUMN_DESTINATION_TYPE);
	activity->endLocationId = r.get<string>(COLUMN_DESTINATION_ID);

	unsigned int nodeId = atoi(activity->endLocationId.c_str());
	const RoadNetwork *rdNetwork = RoadNetwork::getInstance();
	activity->destination = WayPoint(rdNetwork->getById(rdNetwork->getMapOfIdvsNodes(), nodeId));
	
	return activity;
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
	cfg.setWorkerPublisherEnabled(stConfig.commsim.enabled);

	//Inform of load order (drivers, database, pedestrians, etc.).
	informLoadOrder(stConfig.loadAgentsOrder);

	//Ensure granularities are multiples of each other. Then set the "ticks" based on each granularity.
	checkGranularities();
	setTicks();

	//Load from database or XML.
	loadNetworkFromDatabase();
	
	//Load travel times if path-set is enabled
	if (cfg.PathSetMode())
	{
		TravelTimeManager::getInstance()->loadTravelTimes();
	}
	
	if (cfg.isPublicTransitEnabled())
	{
		loadPublicTransitNetworkFromDatabase();
	}

	//Set PartitionManager instance (if using MPI and it's enabled).
	if (cfg.MPI_Enabled() && cfg.using_MPI)
	{
		int partId = stConfig.partitioningSolutionId;
		PartitionManager::instance().partition_config->partition_solution_id = partId;
		std::cout << "partition_solution_id in configuration:" << partId << std::endl;
	}

	cfg.sealNetwork();

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
	
	//Register and initialise BusController
	if (cfg.busController.enabled)
	{
		BusControllerST::RegisterBusController(-1, cfg.mutexStategy());
		BusController* busController = BusController::GetInstance();
		busController->initializeBusController(active_agents);
		active_agents.insert(busController);
	}

	//Load Agents, Pedestrians, and Trip Chains as specified in loadAgentOrder
	loadAgentsInOrder(constraints);

	printSettings();
}

void ExpandShortTermConfigFile::loadNetworkFromDatabase()
{
	//Load from the database or from XML
	if (ST_Config::getInstance().networkSource == NetworkSource::NETSRC_DATABASE)
	{
		//The instance of the network loader
		NetworkLoader *loader = NetworkLoader::getInstance();

		//Output Database details
		std::cout << "Database connection: " << cfg.getDatabaseConnectionString() << "\n\n";

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

void ExpandShortTermConfigFile::loadPublicTransitNetworkFromDatabase()
{
	PT_NetworkCreater::init();
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
		
		case LoadAg_XML:
			for (std::map<std::string, std::vector<EntityTemplate> >::const_iterator it = stConfig.futureAgents.begin(); it != stConfig.futureAgents.end(); it++)
			{
				generateXMLAgents(it->second);
			}
			std::cout << "\nLoaded drivers from the configuration file.\n";
			break;
		
		default:
			throw std::runtime_error("Unknown item in load_agents");
		}
	}
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
			if((*itRows).get<unsigned int>(COLUMN_LOAD_FACTOR) == 1)
			{
				//Get the person id and trip id
				const string tripId = (*itRows).get<string>(COLUMN_TRIP_ID);
				const string personId = (*itRows).get<string>(COLUMN_PERSON_ID);

				//The trip belonging to the person
				vector<TripChainItem *> &personTripChain = tripChains[personId][tripId];
				
				//Check if a trip of the same id is already associated with this person
				if(personTripChain.empty())
				{
					//Create a trip and the corresponding activity (if any)
					Trip *trip = makeTrip(*itRows);
					
					if(trip)
					{
						personTripChain.push_back(trip);
					}

					//Create activity if end time is provided
					if (!(*itRows).get<string>(COLUMN_ACTIVITY_END_TIME).empty())
					{
						Activity *activity = makeActivity(*itRows);
						personTripChain.push_back(activity);
					}
				}
				else
				{
					//A trip of the same id has been previously added. This means that this is a sub-trip in the sequence
					//Vector structure-> [Trip[Sub-trip][...][Sub-trip]][Activity]
					//So create the sub-trip
					SubTrip subtrip = makeSubTrip(*itRows);
					Trip *trip = static_cast<Trip *>(personTripChain.front());
					trip->addSubTrip(subtrip);
				}
				
				//If the person is not created, create it and add it to the map
				if(persons.find(personId) == persons.end())
				{
					Person_ST *person = new Person_ST("DAS_TripChain", cfg.mutexStategy(), -1);
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
	
	std::vector<TripChainItem*> tripChain;
	const RoadNetwork *rn = RoadNetwork::getInstance();

	//Loop through all agents of this type.
	for (std::vector<EntityTemplate>::const_iterator it = xmlItems.begin(); it != xmlItems.end(); ++it)
	{
		std::string mode = it->mode;		
		
		//Set the origin and destination nodes		
		const Node *originNd = rn->getById(rn->getMapOfIdvsNodes(), it->originNode);
		const Node *destinNd = rn->getById(rn->getMapOfIdvsNodes(), it->destNode);
		
		//Make the trip item
		Trip *trip = MakePseudoTrip(agentId, originNd, destinNd, it->startTimeMs, mode);
		tripChain.push_back(trip);
	}
	
	//Create the Person agent with that given ID (or an auto-generated one)
	Person_ST *person = new Person_ST("XML_TripChain", cfg.mutexStategy(), tripChain);
	string id = boost::lexical_cast<std::string>(person->GetId());
	person->setDatabaseId(id);
	
	//Set the start locations for the first sub-trip only (rest of are ignored)
	std::vector<EntityTemplate>::const_iterator it = xmlItems.begin();
	person->startLaneIndex = it->startLaneIndex;
	person->startSegmentId = it->startSegmentId;
	person->segmentStartOffset = it->segmentStartOffset;
	person->initialSpeed = it->initialSpeed;
	
	//Set the usage of in-simulation travel times
	int randomInt = Utils::generateInt(0, 100);

	if (randomInt < cfg.simulation.inSimulationTTUsage)
	{
		person->setUseInSimulationTravelTime(true);
	}
	
	person->setNextPathPlanned(false);
	
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
	std::cout << "\nConfiguration parameters:\n";
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

	//Print the network (this will go to a different output file...)
	std::cout << "------------------\n\n";
	NetworkPrinter nwPrinter(cfg, cfg.outNetworkFileName);
	nwPrinter.printSignals(getSignalsInfo(Signal::getMapOfIdVsSignals()));
	nwPrinter.printNetwork(RoadNetwork::getInstance());
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
