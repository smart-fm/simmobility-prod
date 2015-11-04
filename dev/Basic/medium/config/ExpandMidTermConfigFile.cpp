#include "ExpandMidTermConfigFile.hpp"

#include <boost/lexical_cast.hpp>
#include <map>
#include <vector>
#include "entities/params/PT_NetworkEntities.hpp"
#include "entities/BusController.hpp"
#include "entities/BusControllerMT.hpp"
#include "entities/conflux/Conflux.hpp"
#include "geospatial/Incident.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/network/NetworkLoader.hpp"
#include "metrics/Length.hpp"
#include "path/PathSetManager.hpp"
#include "path/PT_PathSetManager.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

ExpandMidTermConfigFile::ExpandMidTermConfigFile(MT_Config &mtCfg, ConfigParams &cfg, std::set<Entity*>& active_agents) :
		cfg(cfg), mtCfg(mtCfg), active_agents(active_agents)
{
	processConfig();
}

void ExpandMidTermConfigFile::processConfig()
{
    cfg.simMobRunMode = ConfigParams::MID_TERM;
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

    if (mtCfg.RunningMidSupply() && mtCfg.isRegionRestrictionEnabled())
    {
        RestrictedRegion::getInstance().populate();
    }

    if (cfg.isPublicTransitEnabled())
    {
        loadPublicTransitNetworkFromDatabase();
    }

    if (ConfigManager::GetInstance().FullConfig().getPathSetConf().privatePathSetMode == "generation")
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
    if (ConfigManager::GetInstance().FullConfig().getPathSetConf().publicPathSetMode == "generation")
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
        Conflux::ProcessConfluxes();
        std::cout << mtCfg.getConfluxes().size() << " Confluxes created" << std::endl;
    }

    //register and initialize BusController
	if (cfg.busController.enabled)
	{
		BusControllerMT::RegisterBusController(-1, cfg.mutexStategy());
		BusController* busController = BusController::GetInstance();
		busController->initializeBusController(active_agents);
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
    NetworkLoader::getInstance()->loadNetwork(cfg.getDatabaseConnectionString(false), cfg.getDatabaseProcMappings().procedureMappings);
}

void ExpandMidTermConfigFile::loadPublicTransitNetworkFromDatabase()
{
    PT_Network::getInstance().init();
}

void ExpandMidTermConfigFile::verifyIncidents()
{
	std::vector<IncidentParams>& incidents = mtCfg.getIncidents();
	const unsigned int baseGranMS = cfg.simulation.simStartTime.getValue();

	for (std::vector<IncidentParams>::iterator incIt = incidents.begin(); incIt != incidents.end(); ++incIt)
	{
		const std::map<unsigned int, RoadSegment*>& segLookup = RoadNetwork::getInstance()->getMapOfIdVsRoadSegments();
		const std::map<unsigned int, RoadSegment*>::const_iterator segIt = segLookup.find((*incIt).segmentId);
		if (segIt == segLookup.end())
		{
			throw std::runtime_error("invalid segment id supplied for incident injection");
		}
		const RoadSegment* roadSeg = segIt->second;

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
				LaneItem lane;
				lane.laneId = laneIt->laneId;
				lane.speedLimit = laneIt->speedLimit;
				item->laneItems.push_back(lane);
				if (lane.laneId < lanes.size() && lane.laneId < incIt->laneParams.size())
				{
					incIt->laneParams[lane.laneId].xLaneStartPos = lanes[lane.laneId]->getPolyLine()->getFirstPoint().getX();
					incIt->laneParams[lane.laneId].yLaneStartPos = lanes[lane.laneId]->getPolyLine()->getFirstPoint().getY();
					incIt->laneParams[lane.laneId].xLaneEndPos = lanes[lane.laneId]->getPolyLine()->getLastPoint().getX();
					incIt->laneParams[lane.laneId].yLaneEndPos = lanes[lane.laneId]->getPolyLine()->getLastPoint().getY();
				}
			}

			RoadSegment* rs = const_cast<RoadSegment*>(roadSeg);
			float length = rs->getLength();
			centimeter_t pos = length * item->position / 100.0;
			rs->addObstacle(pos, item);
		}
	}
}

void ExpandMidTermConfigFile::setRestrictedRegionSupport()
{
    PrivateTrafficRouteChoice::getInstance()->setRegionRestrictonEnabled(mtCfg.isRegionRestrictionEnabled());
}

void ExpandMidTermConfigFile::checkGranularities()
{
    //Granularity check
    const unsigned int baseGranMS = cfg.simulation.baseGranMS;
    const WorkerParams& workers = mtCfg.getWorkerParams();

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
	if (!setTickFromBaseGran(mtCfg.granPersonTicks, mtCfg.getWorkerParams().person.granularityMs))
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
    //PrintNetwork(cfg, cfg.outNetworkFileName);
    std::cout << "------------------\n";
}
