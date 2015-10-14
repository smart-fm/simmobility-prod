#include "ExpandMidTermConfigFile.hpp"

#include <boost/lexical_cast.hpp>
#include <map>
#include <vector>
#include "entities/params/PT_NetworkEntities.hpp"
#include "entities/BusController.hpp"
#include "entities/BusControllerMT.hpp"
#include "path/PathSetManager.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
const double SHORT_SEGMENT_LENGTH_LIMIT = 5 * sim_mob::PASSENGER_CAR_UNIT; // 5 times a car's length
}

ExpandMidTermConfigFile::ExpandMidTermConfigFile(MT_Config &mtCfg, ConfigParams &cfg,
												std::set<Entity*>& active_agents,
												StartTimePriorityQueue& pending_agents) :
    cfg(cfg), mtCfg(mtCfg), active_agents(active_agents), pending_agents(pending_agents)
{
    processConfig();
}

void ExpandMidTermConfigFile::processConfig()
{
    cfg.simMobRunMode = ConfigParams::SimMobRunMode::MID_TERM;
    cfg.setWorkerPublisherEnabled(false);

    //Set the auto-incrementing ID.
    if (cfg.simulation.startingAutoAgentID < 0)
    {
	    throw std::runtime_error("Agent auto-id must start from >0.");
    }

    Agent::setIncrementIDStartValue(cfg.simulation.startingAutoAgentID, true);

    //Ensure granularities are multiples of each other. Then set the "ticks" based on each granularity.
    checkGranularities();
    setTicks();

    //Maintain unique/non-colliding IDs.
    ConfigParams::AgentConstraints constraints;
    constraints.startingAutoAgentID = cfg.simulation.startingAutoAgentID;

    loadNetworkFromDatabase();

    if (mtCfg.RunningMidSupply())
    {
        RestrictedRegion::getInstance().populate();
    }

    //Detect sidewalks in the middle of the road.
    WarnMidroadSidewalks();

    if (mtCfg.publicTransitEnabled)
    {
        loadPublicTransitNetworkFromDatabase();
    }

    if (ConfigManager::GetInstance().FullConfig().pathSet().privatePathSetMode == "generation")
    {
        Print() << "bulk profiler start: " << std::endl;
        Profiler profile("bulk profiler start", true);
        //	This mode can be executed in the main function also but we need the street directory to be initialized first
        //	to be least intrusive to the rest of the code, we take a safe approach and run this mode from here, although a lot of
        //	unnecessary code will be executed.
        PrivatePathsetGenerator::getInstance()->bulkPathSetGenerator();
        Print() << "Bulk Generation Done " << profile.tick().first.count() << std::endl;
        exit(1);
    }
    if (ConfigManager::GetInstance().FullConfig().pathSet().publicPathSetMode == "generation")
    {
        Print() << "Public Transit bulk pathSet Generation started: " << std::endl;
        PT_PathSetManager::Instance().PT_BulkPathSetGenerator();
        Print() << "Public Transit bulk pathSet Generation Done: " << std::endl;
        exit(1);
    }

    //TODO: put its option in config xml
    //generateOD("/home/fm-simmobility/vahid/OD.txt", "/home/fm-simmobility/vahid/ODs.xml");
    //Process Confluxes if required
    if (mtCfg.RunningMidSupply())
    {
        size_t sizeBefore = mtCfg.getConfluxes().size();
        ExpandMidTermConfigFile::ProcessConfluxes(ConfigManager::GetInstance().FullConfig().getNetwork());
        std::cout << mtCfg.getConfluxes().size() << " Confluxes created" << std::endl;
    }

    //register and initialize BusController
	if (cfg.busController.enabled)
	{
		BusControllerMT::RegisterBusController(-1, cfg.mutexStategy());
		BusController* busController = BusController::GetInstance();
		busController->initializeBusController(active_agents, cfg.getPT_BusDispatchFreq());
		active_agents.insert(busController);
	}

    /// Enable/Disble restricted region support based on configuration
    setRestrictedRegionSupport();

    //combine incident information to road network
    verifyIncidents();

    //Print some of the settings we just generated.
    printSettings();
}

void ExpandMidTermConfigFile::loadNetworkFromDatabase()
{
    std::cout << "Loading Road Network from the database.\n";
    aimsun::Loader::LoadNetwork(cfg.getDatabaseConnectionString(false),
    		cfg.getDatabaseProcMappings().procedureMappings,
					 cfg.getNetworkRW(), cfg.getTripChains(), nullptr);
}

void ExpandMidTermConfigFile::loadPublicTransitNetworkFromDatabase()
{
    PT_Network::getInstance().init();
}

void ExpandMidTermConfigFile::WarnMidroadSidewalks()
{
    const std::vector<Link*>& links = cfg.getNetwork().getLinks();
    for (std::vector<Link*>::const_iterator linkIt = links.begin(); linkIt != links.end(); ++linkIt)
    {
	const std::set<RoadSegment*>& segs = (*linkIt)->getUniqueSegments();
	for (std::set<RoadSegment*>::const_iterator segIt = segs.begin(); segIt != segs.end(); ++segIt)
	{
	    const std::vector<Lane*>& lanes = (*segIt)->getLanes();
	    for (std::vector<Lane*>::const_iterator laneIt = lanes.begin(); laneIt != lanes.end(); ++laneIt)
	    {
		//Check it.
		if ((*laneIt)->is_pedestrian_lane() &&
			    ((*laneIt) != (*segIt)->getLanes().front()) &&
			    ((*laneIt) != (*segIt)->getLanes().back()))
		{
		    Warn() << "Pedestrian lane is located in the middle of segment" << (*segIt)->getId() << "\n";
		}
	    }
	}
    }
}

void ExpandMidTermConfigFile::verifyIncidents()
{
    std::vector<IncidentParams>& incidents = mtCfg.getIncidents();
    const unsigned int baseGranMS = cfg.simulation.simStartTime.getValue();

    for (std::vector<IncidentParams>::iterator incIt = incidents.begin(); incIt != incidents.end(); ++incIt)
    {
	const RoadSegment* roadSeg = StreetDirectory::instance().getRoadSegment((*incIt).segmentId);

	if (roadSeg)
	{
	    Incident* item = new Incident();
	    item->accessibility = (*incIt).accessibility;
	    item->capFactor = (*incIt).capFactor;
	    item->compliance = (*incIt).compliance;
	    item->duration = (*incIt).duration;
	    item->incidentId = (*incIt).incidentId;
	    item->position = (*incIt).position;
	    item->segmentId = (*incIt).segmentId;
	    item->length = (*incIt).length;
	    item->severity = (*incIt).severity;
	    item->startTime = (*incIt).startTime - baseGranMS;
	    item->visibilityDistance = (*incIt).visibilityDistance;

	    const std::vector<Lane*>& lanes = roadSeg->getLanes();
	    for (std::vector<IncidentParams::LaneParams>::iterator laneIt = incIt->laneParams.begin(); laneIt != incIt->laneParams.end(); ++laneIt)
	    {
		Incident::LaneItem lane;
		lane.laneId = laneIt->laneId;
		lane.speedLimit = laneIt->speedLimit;
		item->laneItems.push_back(lane);
		if (lane.laneId < lanes.size() && lane.laneId < incIt->laneParams.size())
		{
		    incIt->laneParams[lane.laneId].xLaneStartPos = lanes[lane.laneId]->polyline_[0].getX();
		    incIt->laneParams[lane.laneId].yLaneStartPos = lanes[lane.laneId]->polyline_[0].getY();
		    if (lanes[lane.laneId]->polyline_.size() > 0)
		    {
			unsigned int sizePoly = lanes[lane.laneId]->polyline_.size();
			incIt->laneParams[lane.laneId].xLaneEndPos = lanes[lane.laneId]->polyline_[sizePoly - 1].getX();
			incIt->laneParams[lane.laneId].yLaneEndPos = lanes[lane.laneId]->polyline_[sizePoly - 1].getY();
		    }
		}
	    }

	    RoadSegment* rs = const_cast<RoadSegment*> (roadSeg);
	    float length = rs->getLengthOfSegment();
	    centimeter_t pos = length * item->position / 100.0;
	    rs->addObstacle(pos, item);
	}
    }
}

void ExpandMidTermConfigFile::setRestrictedRegionSupport()
{
    PrivateTrafficRouteChoice::getInstance()->setRegionRestrictonEnabled(mtCfg.cbd);
}

void ExpandMidTermConfigFile::checkGranularities()
{
    //Granularity check
    const unsigned int baseGranMS = cfg.simulation.baseGranMS;
    const WorkerParams& workers = mtCfg.workers;

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
}

bool ExpandMidTermConfigFile::setTickFromBaseGran(unsigned int& res, unsigned int tickLenMs)
{
	res = tickLenMs / cfg.simulation.baseGranMS;
	return tickLenMs % cfg.simulation.baseGranMS == 0;
}

void ExpandMidTermConfigFile::setTicks()
{
    if (!setTickFromBaseGran(cfg.totalRuntimeTicks, cfg.simulation.totalRuntimeMS))
    {
	Warn() << "Total runtime will be truncated by the base granularity\n";
    }
    if (!setTickFromBaseGran(cfg.totalWarmupTicks, cfg.simulation.totalWarmupMS))
    {
	Warn() << "Total warm-up will be truncated by the base granularity\n";
    }
	if (!setTickFromBaseGran(mtCfg.granPersonTicks, mtCfg.workers.person.granularityMs))
    {
	throw std::runtime_error("Person granularity not a multiple of base granularity.");
    }
}

void ExpandMidTermConfigFile::printSettings()
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
    std::cout << "  Person Granularity: " << mtCfg.granPersonTicks << " " << "ticks" << "\n";
    std::cout << "  Start time: " << cfg.simStartTime().getStrRepr() << "\n";
    std::cout << "  Mutex strategy: " << (cfg.mutexStategy() == MtxStrat_Locked ? "Locked" : cfg.mutexStategy() == MtxStrat_Buffered ? "Buffered" : "Unknown") << "\n";

//Output Database details
//	if (cfg.system.networkSource == SystemParams::NETSRC_XML)
//	{
//		std::cout << "Network details loaded from xml file: " << cfg.system.networkXmlInputFile << "\n";
//	}
//	if (cfg.system.networkSource == SystemParams::NETSRC_DATABASE)
//	{
//		std::cout << "Network details loaded from database connection: " << cfg.getDatabaseConnectionString() << "\n";
//	}

    std::cout << "Network details loaded from database connection: " << cfg.getDatabaseConnectionString() << "\n";

    //Print the network (this will go to a different output file...)
    std::cout << "------------------\n";
    PrintNetwork(cfg, cfg.outNetworkFileName);
    std::cout << "------------------\n";
}

void ExpandMidTermConfigFile::CreateSegmentStats(const RoadSegment* rdSeg, Conflux* conflux, std::list<SegmentStats*>& splitSegmentStats) {
	if(!rdSeg)
	{
		throw std::runtime_error("CreateSegmentStats(): NULL RoadSegment was passed");
	}
	std::stringstream debugMsgs;
	const std::map<centimeter_t, const RoadItem*>& obstacles = rdSeg->obstacles;
	double lengthCoveredInSeg = 0;
	double segStatLength;
	double rdSegmentLength = rdSeg->getPolylineLength();
	// NOTE: std::map implements strict weak ordering which defaults to less<key_type>
	// This is precisely the order in which we want to iterate the stops to create SegmentStats
	for(std::map<centimeter_t, const RoadItem*>::const_iterator obsIt = obstacles.begin(); obsIt != obstacles.end(); obsIt++)
	{
		const BusStop* busStop = dynamic_cast<const BusStop*>(obsIt->second);
		if(busStop)
		{
			double stopOffset = (double) (obsIt->first);
			if(stopOffset <= 0)
			{
				SegmentStats* segStats = new SegmentStats(rdSeg, conflux, rdSegmentLength);
				segStats->addBusStop(busStop);
				//add the current stop and the remaining stops (if any) to the end of the segment as well
				while(++obsIt != obstacles.end())
				{
					busStop = dynamic_cast<const BusStop*>(obsIt->second);
					if(busStop)
					{
						segStats->addBusStop(busStop);
					}
				}
				splitSegmentStats.push_back(segStats);
				lengthCoveredInSeg = rdSegmentLength;
				break;
			}
			if(stopOffset < lengthCoveredInSeg) {
				debugMsgs<<"bus stops are iterated in wrong order"
						<<"|seg: "<<rdSeg->getStartEnd()
						<<"|seg length: "<<rdSegmentLength
						<<"|curr busstop offset: "<<obsIt->first
						<<"|prev busstop offset: "<<lengthCoveredInSeg
						<<"|busstop: " << busStop->getBusstopno_()
						<<std::endl;
				throw std::runtime_error(debugMsgs.str());
			}
			if(stopOffset >= rdSegmentLength) {
				//this is probably due to error in data and needs manual fixing
				segStatLength = rdSegmentLength - lengthCoveredInSeg;
				lengthCoveredInSeg = rdSegmentLength;
				SegmentStats* segStats = new SegmentStats(rdSeg, conflux, segStatLength);
				segStats->addBusStop(busStop);
				//add the current stop and the remaining stops (if any) to the end of the segment as well
				while(++obsIt != obstacles.end()) {
					busStop = dynamic_cast<const BusStop*>(obsIt->second);
					if(busStop) {
						segStats->addBusStop(busStop);
					}
				}
				splitSegmentStats.push_back(segStats);
				break;
			}
			//the relation (lengthCoveredInSeg < stopOffset < rdSegmentLength) holds here
			segStatLength = stopOffset - lengthCoveredInSeg;
			lengthCoveredInSeg = stopOffset;
			SegmentStats* segStats = new SegmentStats(rdSeg, conflux, segStatLength);
			segStats->addBusStop(busStop);
			splitSegmentStats.push_back(segStats);
		}
	}

	// manually adjust the position of the stops to avoid short segments
	if(!splitSegmentStats.empty()) { // if there are stops in the segment
		//another segment stats has to be created for the remaining length.
		//this segment stats does not contain a bus stop
		//adjust the length of the last segment stats if the remaining length is short
		double remainingSegmentLength = rdSegmentLength - lengthCoveredInSeg;
		if(remainingSegmentLength < 0) {
			debugMsgs<<"Lengths of segment stats computed incorrectly\n";
			debugMsgs<<"segmentLength: "<<rdSegmentLength<<"|stat lengths: ";
			double totalStatsLength = 0;
			for(std::list<SegmentStats*>::iterator statsIt=splitSegmentStats.begin();
					statsIt!=splitSegmentStats.end(); statsIt++){
				debugMsgs<<(*statsIt)->length<<"|";
				totalStatsLength = totalStatsLength + (*statsIt)->length;
			}
			debugMsgs<<"totalStatsLength: "<<totalStatsLength<<std::endl;
			throw std::runtime_error(debugMsgs.str());
		}
		else if(remainingSegmentLength == 0) {
			// do nothing
		}
		else if(remainingSegmentLength < SHORT_SEGMENT_LENGTH_LIMIT) {
			// if the remaining length creates a short segment,
			// add this length to the last segment stats
			remainingSegmentLength = splitSegmentStats.back()->length + remainingSegmentLength;
			splitSegmentStats.back()->length = remainingSegmentLength;
		}
		else {
			// if the remaining length is long enough create a new SegmentStats
			SegmentStats* segStats = new SegmentStats(rdSeg, conflux, remainingSegmentLength);
			splitSegmentStats.push_back(segStats);
		}

		// if there is atleast 1 bus stop in the segment and the length of the
		// created segment stats is short, we will try to adjust the lengths to
		// avoid short segments
		bool noMoreShortSegs = false;
		while (!noMoreShortSegs && splitSegmentStats.size() > 1) {
			noMoreShortSegs = true; //hopefully
			SegmentStats* lastStats = splitSegmentStats.back();
			std::list<SegmentStats*>::iterator statsIt = splitSegmentStats.begin();
			while((*statsIt)!=lastStats) {
				SegmentStats* currStats = *statsIt;
				std::list<SegmentStats*>::iterator nxtStatsIt = statsIt; nxtStatsIt++; //get a copy and increment for next
				SegmentStats* nextStats = *nxtStatsIt;
				if(currStats->length < SHORT_SEGMENT_LENGTH_LIMIT) {
					noMoreShortSegs = false; //there is a short segment
					if(nextStats->length >= SHORT_SEGMENT_LENGTH_LIMIT) {
						double lengthDiff = SHORT_SEGMENT_LENGTH_LIMIT - currStats->length;
						currStats->length = SHORT_SEGMENT_LENGTH_LIMIT;
						nextStats->length = nextStats->length - lengthDiff;
					}
					else {
						// we will merge i-th SegmentStats with i+1-th SegmentStats
						// and add both bus stops to the merged SegmentStats
						nextStats->length = currStats->length + nextStats->length;
						for(std::vector<const BusStop*>::iterator stopIt=currStats->busStops.begin(); stopIt!=currStats->busStops.end(); stopIt++) {
							nextStats->addBusStop(*stopIt);
						}
						statsIt = splitSegmentStats.erase(statsIt);
						safe_delete_item(currStats);
						continue;
					}
				}
				statsIt++;
			}
		}
		if(splitSegmentStats.size() > 1) {
			// the last segment stat is handled separately
			std::list<SegmentStats*>::iterator statsIt = splitSegmentStats.end();
			statsIt--;
			SegmentStats* lastSegStats = *(statsIt);
			statsIt--;
			SegmentStats* lastButOneSegStats = *(statsIt);
			if(lastSegStats->length < SHORT_SEGMENT_LENGTH_LIMIT) {
				lastSegStats->length = lastButOneSegStats->length + lastSegStats->length;
				for(std::vector<const BusStop*>::iterator stopIt=lastButOneSegStats->busStops.begin();
						stopIt!=lastButOneSegStats->busStops.end(); stopIt++) {
					lastSegStats->addBusStop(*stopIt);
				}
				splitSegmentStats.erase(statsIt);
				safe_delete_item(lastButOneSegStats);
			}
		}
	}
	else {
		// if there are no stops in the segment, we create a single SegmentStats for this segment
		SegmentStats* segStats = new SegmentStats(rdSeg, conflux, rdSegmentLength);
		splitSegmentStats.push_back(segStats);
	}

	uint16_t statsNum = 1;
	std::set<SegmentStats*>& segmentStatsWithStops = MT_Config::getInstance().getSegmentStatsWithBusStops();
	for(std::list<SegmentStats*>::iterator statsIt=splitSegmentStats.begin(); statsIt!=splitSegmentStats.end(); statsIt++) {
		SegmentStats* stats = *statsIt;
		//number the segment stats
		stats->statsNumberInSegment = statsNum;
		statsNum++;

		//add to segmentStatsWithStops if there is a bus stop in stats
		if (!(stats->getBusStops().empty())) {
			segmentStatsWithStops.insert(stats);
		}
	}
}

/*
 * iterates multinodes and creates confluxes for all of them
 */
// TODO: Remove debug messages
void ExpandMidTermConfigFile::ProcessConfluxes(const RoadNetwork& rdnw)
{
	std::stringstream debugMsgs(std::stringstream::out);
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	MT_Config& mtCfg = MT_Config::getInstance();
	Conflux::updateInterval = boost::lexical_cast<uint32_t>(cfg.genericProps.at("update_interval"));
	std::set<Conflux*>& confluxes = mtCfg.getConfluxes();
	const MutexStrategy& mtxStrat = cfg.mutexStategy();
	std::map<const MultiNode*, Conflux*>& multinodeConfluxesMap = mtCfg.getConfluxNodes();

	//Make a temporary map of <multi node, set of road-segments directly connected to the multinode>
	//TODO: This should be done automatically *before* it's needed.
	std::map<const MultiNode*, std::set<const RoadSegment*> > roadSegmentsAt;
	for (std::vector<Link*>::const_iterator it = rdnw.links.begin(); it != rdnw.links.end(); it++)
	{
		MultiNode* start = dynamic_cast<MultiNode*>((*it)->getStart());
		MultiNode* end = dynamic_cast<MultiNode*>((*it)->getEnd());
		if ((!start) || (!end))
		{
			throw std::runtime_error("Link start/ends must be MultiNodes (in Conflux).");
		}
		roadSegmentsAt[start].insert((*it)->getSegments().front());
		roadSegmentsAt[end].insert((*it)->getSegments().back());
		end->addRoadSegmentAt((*it)->getSegments().back()); //tag upstream segments for each multinode
	}

	for (std::vector<MultiNode*>::const_iterator i = rdnw.nodes.begin(); i != rdnw.nodes.end(); i++)
	{
		// we create a conflux for each multinode
		Conflux* conflux = new Conflux(*i, mtxStrat);
		try
		{
			std::set<const RoadSegment*>& segmentsAtNode = roadSegmentsAt.at(*i);
			if (!segmentsAtNode.empty())
			{
				for (std::set<const RoadSegment*>::iterator segmtIt = segmentsAtNode.begin(); segmtIt != segmentsAtNode.end(); segmtIt++)
				{
					Link* lnk = (*segmtIt)->getLink();
					std::vector<SegmentStats*> upSegStatsList;
					if (lnk->getStart() == (*i))
					{
						//lnk is downstream to the multinode and doesn't belong to this conflux
						std::vector<RoadSegment*>& downSegs = lnk->getSegments();
						conflux->downstreamSegments.insert(downSegs.begin(), downSegs.end());
						if (lnk->getStart() != lnk->getEnd())
						{
							continue;
						} // some links can start and end at the same section
					}
					//else
					//lnk *ends* at the multinode of this conflux.
					//lnk is upstream to the multinode and belongs to this conflux
					std::vector<RoadSegment*>& upSegs = lnk->getSegments();
					//set conflux pointer to the segments and create SegmentStats for the segment
					for (std::vector<RoadSegment*>::iterator segIt = upSegs.begin(); segIt != upSegs.end(); segIt++)
					{
						RoadSegment* rdSeg = *segIt;
						double rdSegmentLength = rdSeg->getPolylineLength();

						std::list<SegmentStats*> splitSegmentStats;
						CreateSegmentStats(rdSeg, conflux, splitSegmentStats);
						if (splitSegmentStats.empty())
						{
							debugMsgs << "no segment stats created for segment." << "|segment: " << rdSeg->getStartEnd() << "|conflux: " << conflux->multiNode << std::endl;
							throw std::runtime_error(debugMsgs.str());
						}
						std::vector<SegmentStats*>& rdSegSatsList = conflux->segmentAgents[rdSeg];
						rdSegSatsList.insert(rdSegSatsList.end(), splitSegmentStats.begin(), splitSegmentStats.end());
						upSegStatsList.insert(upSegStatsList.end(), splitSegmentStats.begin(), splitSegmentStats.end());
					}
					conflux->upstreamSegStatsMap.insert(std::make_pair(lnk, upSegStatsList));
					conflux->virtualQueuesMap.insert(std::make_pair(lnk, std::deque<Person*>()));
				} // end for
			} //end if
		}
		catch (const std::out_of_range& oor)
		{
			debugMsgs << "Loader::ProcessConfluxes() : No segments were found at multinode: " << (*i)->getID() << "|location: " << (*i)->getLocation()
					<< std::endl;
			Print() << debugMsgs.str();
			debugMsgs.str(std::string());
			continue;
		}
		conflux->resetOutputBounds();
		confluxes.insert(conflux);
		multinodeConfluxesMap.insert(std::make_pair(*i, conflux));
	} // end for each multinode
	CreateLaneGroups();
}

void ExpandMidTermConfigFile::CreateLaneGroups()
{
	std::set<Conflux*>& confluxes = MT_Config::getInstance().getConfluxes();
	if(confluxes.empty()) { return; }

	typedef std::vector<SegmentStats*> SegmentStatsList;
	typedef std::map<const Lane*, LaneStats* > LaneStatsMap;
	typedef std::map<Link*, const SegmentStatsList> UpstreamSegmentStatsMap;

	for(std::set<Conflux*>::const_iterator cfxIt=confluxes.begin(); cfxIt!=confluxes.end(); cfxIt++)
	{
		UpstreamSegmentStatsMap& upSegsMap = (*cfxIt)->upstreamSegStatsMap;
		const MultiNode* cfxMultinode = (*cfxIt)->getMultiNode();
		for(UpstreamSegmentStatsMap::const_iterator upSegsMapIt=upSegsMap.begin(); upSegsMapIt!=upSegsMap.end(); upSegsMapIt++)
		{
			const SegmentStatsList& segStatsList = upSegsMapIt->second;
			if(segStatsList.empty()) { throw std::runtime_error("No segment stats for link"); }

			//assign downstreamLinks to the last segment stats
			SegmentStats* lastStats = segStatsList.back();
			const std::set<LaneConnector*>& lcs = cfxMultinode->getOutgoingLanes(lastStats->getRoadSegment());
			std::set<const Lane*> connLanes;
			for (std::set<LaneConnector*>::const_iterator lcIt = lcs.begin(); lcIt != lcs.end(); lcIt++)
			{
				const Lane* fromLane = (*lcIt)->getLaneFrom();
				connLanes.insert(fromLane);
				const Link* downStreamLink = (*lcIt)->getLaneTo()->getRoadSegment()->getLink();
				lastStats->laneStatsMap.at(fromLane)->addDownstreamLink(downStreamLink); //duplicates are eliminated by the std::set containing the downstream links
			}

			//construct inverse lookup for convenience
			for (LaneStatsMap::const_iterator lnStatsIt = lastStats->laneStatsMap.begin(); lnStatsIt != lastStats->laneStatsMap.end(); lnStatsIt++)
			{
				if(lnStatsIt->second->isLaneInfinity()) { continue; }
				const std::set<const Link*>& downstreamLnks = lnStatsIt->second->getDownstreamLinks();
				for(std::set<const Link*>::const_iterator dnStrmIt = downstreamLnks.begin(); dnStrmIt != downstreamLnks.end(); dnStrmIt++)
				{
					lastStats->laneGroup[*dnStrmIt].push_back(lnStatsIt->second);
				}
			}

			//extend the downstream links assignment to the segmentStats upstream to the last segmentStats
			SegmentStatsList::const_reverse_iterator upSegsRevIt = segStatsList.rbegin();
			upSegsRevIt++; //lanestats of last segmentstats is already assigned with downstream links... so skip the last segmentstats
			const SegmentStats* downstreamSegStats = lastStats;
			for(; upSegsRevIt!=segStatsList.rend(); upSegsRevIt++)
			{
				SegmentStats* currSegStats = (*upSegsRevIt);
				const RoadSegment* currSeg = currSegStats->getRoadSegment();
				const std::vector<Lane*>& currLanes = currSeg->getLanes();
				if(currSeg == downstreamSegStats->getRoadSegment())
				{	//currSegStats and downstreamSegStats have the same parent segment
					//lanes of the two segstats are same
					for (std::vector<Lane*>::const_iterator lnIt = currLanes.begin(); lnIt != currLanes.end(); lnIt++)
					{
						const Lane* ln = (*lnIt);
						if(ln->is_pedestrian_lane()) { continue; }
						const LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(ln);
						LaneStats* currLnStats = currSegStats->laneStatsMap.at(ln);
						currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
					}
				}
				else
				{
					const UniNode* uninode = dynamic_cast<const UniNode*>(currSeg->getEnd());
					if(!uninode) { throw std::runtime_error("Multinode found in the middle of a link"); }
					for (std::vector<Lane*>::const_iterator lnIt = currLanes.begin(); lnIt != currLanes.end(); lnIt++)
					{
						const Lane* ln = (*lnIt);
						if(ln->is_pedestrian_lane()) { continue; }
						LaneStats* currLnStats = currSegStats->laneStatsMap.at(ln);
						const UniNode::UniLaneConnector uniLnConnector = uninode->getForwardLanes(*ln);
						if(uniLnConnector.left)
						{
							const LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(uniLnConnector.left);
							currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
						}
						if(uniLnConnector.right)
						{
							const LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(uniLnConnector.right);
							currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
						}
						if(uniLnConnector.center)
						{
							const LaneStats* downStreamLnStats = downstreamSegStats->laneStatsMap.at(uniLnConnector.center);
							currLnStats->addDownstreamLinks(downStreamLnStats->getDownstreamLinks());
						}
					}
				}

				//construct inverse lookup for convenience
				for (LaneStatsMap::const_iterator lnStatsIt = currSegStats->laneStatsMap.begin(); lnStatsIt != currSegStats->laneStatsMap.end(); lnStatsIt++)
				{
					if(lnStatsIt->second->isLaneInfinity()) { continue; }
					const std::set<const Link*>& downstreamLnks = lnStatsIt->second->getDownstreamLinks();
					for(std::set<const Link*>::const_iterator dnStrmIt = downstreamLnks.begin(); dnStrmIt != downstreamLnks.end(); dnStrmIt++)
					{
						currSegStats->laneGroup[*dnStrmIt].push_back(lnStatsIt->second);
					}
				}

				downstreamSegStats = currSegStats;
			}

//			*********** the commented for loop below is to print the lanes which do not have lane groups ***
//			for(SegmentStatsList::const_reverse_iterator statsRevIt=segStatsList.rbegin(); statsRevIt!=segStatsList.rend(); statsRevIt++)
//			{
//				const LaneStatsMap lnStatsMap = (*statsRevIt)->laneStatsMap;
//				unsigned int segId = (*statsRevIt)->getRoadSegment()->getSegmentAimsunId();
//				uint16_t statsNum = (*statsRevIt)->statsNumberInSegment;
//				const std::vector<Lane*>& lanes = (*statsRevIt)->getRoadSegment()->getLanes();
//				unsigned int numLanes = 0;
//				for(std::vector<Lane*>::const_iterator lnIt = lanes.begin(); lnIt!=lanes.end(); lnIt++)
//				{
//					if(!(*lnIt)->is_pedestrian_lane()) { numLanes++; }
//				}
//				for (LaneStatsMap::const_iterator lnStatsIt = lnStatsMap.begin(); lnStatsIt != lnStatsMap.end(); lnStatsIt++)
//				{
//					if(lnStatsIt->second->isLaneInfinity() || lnStatsIt->first->is_pedestrian_lane()) { continue; }
//					if(lnStatsIt->second->getDownstreamLinks().empty())
//					{
//						Print() << "~~~ " << segId << "," << statsNum << "," << lnStatsIt->first->getLaneID() << "," << numLanes << std::endl;
//					}
//				}
//			}
		}
	}
}
