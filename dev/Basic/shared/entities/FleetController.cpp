//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <boost/date_time.hpp>

#include "FleetController.hpp"
#include "conf/ConfigManager.hpp"
#include "util/Utils.hpp"
#include "geospatial/network/RoadNetwork.hpp"

namespace bt = boost::posix_time;
using namespace sim_mob;

namespace
{
    enum TaxiFleetTableColumns
    {
        COLUMN_VEHICLE_NUMBER = 0,
        COLUMN_VEHICLE_TYPE = 1,
        COLUMN_DRIVER_ID = 2,
        COLUMN_START_SEGMENT_ID = 3,
        COLUMN_SHIFT_START_TIME = 4,
        COLUMN_CONTROLLER_SUBSCRIPTIONS = 5,
        COLUMN_SHIFT_DURATION = 6,
        COLUMN_PCU = 7,
        COLUMN_PASSENGER_CAPACITY = 8,
        COLUMN_AV = 9
    };
}

double getSecondFrmTimeString(const std::string& startTime)
{
    std::istringstream is(startTime);
    is.imbue(std::locale(is.getloc(),new bt::time_input_facet("%H:%M:%S")));
    bt::ptime pt;
    is >> pt;
    return (double)pt.time_of_day().ticks() / (double)bt::time_duration::rep_type::ticks_per_second;
}

bool FleetController::ifLoopedNode(unsigned int thisNodeId)
{
    auto loopNodesSet = RoadNetwork::getInstance()->getSetOfLoopNodesInNetwork();
    bool found = false;
    std::unordered_set<unsigned int>::const_iterator loopNodeItr;
    loopNodeItr = loopNodesSet.find(thisNodeId);
    if (loopNodeItr != loopNodesSet.end())
    {
        found = true;
    }
    return found;
}
const Node* FleetController::chooseRandomNode()
{
    auto nodeMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
    auto itRandomNode = nodeMap.begin();
    advance(itRandomNode, Utils::generateInt(0, nodeMap.size() - 1));

    const Node *result = itRandomNode->second;

    //Ensure chosen node is not a source/sink node
    if(result->getNodeType() == SOURCE_OR_SINK_NODE || result->getNodeType() == NETWORK_EXCLUDED_NODE || ifLoopedNode(result->getNodeId()))
    {
        result = chooseRandomNode();
    }

    return result;
}
void FleetController::LoadTaxiFleetFromDB()
{
    ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
    soci::session sql_(soci::postgresql,cfg.getDatabaseConnectionString(false));
    const std::map<std::string, std::string>& storedProcs = cfg.getDatabaseProcMappings().procedureMappings;
    std::map<std::string, std::string>::const_iterator spIt = storedProcs.find("taxi_fleet");

    if (spIt == storedProcs.end())
    {
        return;
    }

    std::map<std::string, std::vector<TripChainItem> > tripchains;
    std::stringstream query;

    const SimulationParams &simParams = ConfigManager::GetInstance().FullConfig().simulation;

    query << "select * from " << spIt->second << "('" << simParams.simStartTime.getStrRepr().substr(0, 5)
          << "','" << (DailyTime(simParams.totalRuntimeMS) + simParams.simStartTime).getStrRepr().substr(0, 5) << "')";
    soci::rowset<soci::row> rs = (sql_.prepare << query.str());
    try {
        for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
        {
            FleetItem fleetItem;
            const soci::row &r = (*it);
            fleetItem.vehicleNo = r.get<std::string>(COLUMN_VEHICLE_NUMBER);
            fleetItem.driverId = r.get<std::string>(COLUMN_DRIVER_ID);
            fleetItem.segmentId = r.get<int>(COLUMN_START_SEGMENT_ID);
            fleetItem.vehicleType = r.get<int>(COLUMN_VEHICLE_TYPE);
            fleetItem.passengerCapacity = r.get<int>(COLUMN_PASSENGER_CAPACITY);
            const RoadNetwork *rdnw = RoadNetwork::getInstance();
            const RoadSegment *roadSegment = rdnw->getById(rdnw->getMapOfIdVsRoadSegments(), fleetItem.segmentId);
            fleetItem.startNode = roadSegment->getParentLink()->getToNode();
            if (fleetItem.startNode->getNodeType() == SOURCE_OR_SINK_NODE ||
                fleetItem.startNode->getNodeType() == NETWORK_EXCLUDED_NODE || ifLoopedNode(fleetItem.startNode->getNodeId()))
            {
                Warn()<<"Driver: "<<fleetItem.driverId<<" is started from another random node as is started at the bad node: "<< fleetItem.startNode->getNodeId()<<endl;
                fleetItem.startNode = chooseRandomNode();
            }
            const std::string &startTime = r.get<std::string>(COLUMN_SHIFT_START_TIME);
            fleetItem.startTime = getSecondFrmTimeString(startTime);
            int shiftDuration = 3600 * r.get<int>(COLUMN_SHIFT_DURATION);
            fleetItem.endTime = fleetItem.startTime + shiftDuration;
            fleetItem.controllerSubscription = r.get<int>(COLUMN_CONTROLLER_SUBSCRIPTIONS);
            fleetItem.pcu = r.get<int>(COLUMN_PCU);
            fleetItem.av = r.get<int>(COLUMN_AV);
            std::map<unsigned int, MobilityServiceControllerConfig>::const_iterator controllerIdIt = ConfigManager::GetInstance().FullConfig().mobilityServiceController.enabledControllers.find(
                    fleetItem.controllerSubscription);
            if (controllerIdIt ==
                ConfigManager::GetInstance().FullConfig().mobilityServiceController.enabledControllers.end()) {
                throw std::runtime_error("Invalid Controller Subscription Id.");
            }
            taxiFleet.insert(std::pair<unsigned int, FleetItem>(fleetItem.controllerSubscription, fleetItem));
        }
    }
    catch (std::exception& ex)
    {
        throw std::runtime_error("Mismatch in the taxi fleet table columns with the one declared in the Taxi Fleet enum table.");
    }
}

Entity::UpdateStatus FleetController::frame_init(timeslice now)
{
    return Entity::UpdateStatus::Continue;
}

Entity::UpdateStatus FleetController::frame_tick(timeslice now)
{
    nextTimeTickToStage++;
    unsigned int nextTickMS = nextTimeTickToStage * ConfigManager::GetInstance().FullConfig().baseGranMS();

    //Stage any pending entities that will start during this time tick.
    while (!pendingChildren.empty() && pendingChildren.top()->getStartTime() <= nextTickMS)
    {
        //Ask the current worker's parent WorkGroup to schedule this Entity.
        Entity* child = pendingChildren.top();
        pendingChildren.pop();
        child->parentEntity = this;
        currWorkerProvider->scheduleForBred(child);
    }

    return Entity::UpdateStatus::Continue;
}
