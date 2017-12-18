#include "OnCallDriverFacets.hpp"

#include "entities/controllers/MobilityServiceControllerManager.hpp"
#include "exceptions/Exceptions.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "OnCallDriver.hpp"
#include "path/PathSetManager.hpp"
#include "util/Utils.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "message/ST_Message.hpp"
using namespace sim_mob;
using namespace std;
using namespace messaging;

OnCallDriverMovement::OnCallDriverMovement() : currNode(nullptr)
{
}

OnCallDriverMovement::~OnCallDriverMovement()
{
}
void OnCallDriverMovement::frame_init()
{
#ifndef NDEBUG
    if (!MobilityServiceControllerManager::HasMobilityServiceControllerManager())
    {
        throw std::runtime_error("No controller manager exists");
    }
#endif


    //Create the vehicle and assign it to the role
    Vehicle* newVehicle = NULL;
    const Person_ST *parent = onCallDriver->getParent();

    if (parent)
    {
        newVehicle = initialisePath(true);
    }
    onCallDriver->setVehicle(newVehicle);
    onCallDriver->setResource(newVehicle);

    //Retrieve the starting node of the driver
    currNode = (*(onCallDriver->getParent()->currTripChainItem))->origin.node;
    onCallDriver->getParams().initialSpeed = 0;

    //Register with the controller to which the driver is subscribed
    auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();

    for(auto &ctrlr : controllers)
    {
        onCallDriver->subscribeToOrIgnoreController(controllers, ctrlr.second->getControllerId());
    }
//In the beginning there is nothing to do, yet we require a path to begin moving.
    //So cruise to a random node, by
    // creating a default schedule
    continueCruising(currNode);
    //Begin performing schedule.
    performScheduleItem();
    onCallDriver->sendWakeUpShiftEndMsg();
}
void OnCallDriverMovement::continueCruising(const Node *fromNode)
{
    //Cruise to a random node, by creating a default schedule
    ScheduleItem cruise(CRUISE, onCallDriver->behaviour->chooseDownstreamNode(fromNode));
    Schedule schedule;
    schedule.push_back(cruise);
    onCallDriver->driverSchedule.setSchedule(schedule);
}
void OnCallDriverMovement::frame_tick()
{
    switch (onCallDriver->getDriverStatus())
    {
        case CRUISING:
        {
            if(fwdDriverMovement.isEndOfPath())
            {
                const Node *endOfPathNode = fwdDriverMovement.getCurrLink()->getToNode();
                continueCruising(endOfPathNode);
                performScheduleItem();
            }
            break;
        }
        case PARKED:
        {
            break;
        }
    }
   if(onCallDriver->getDriverStatus() != PARKED)
   {
       DriverMovement::frame_tick();
       DriverUpdateParams &params = onCallDriver->getParams();
       switch (params.stopPointState)
       {
           case DriverUpdateParams::ARRIVED_AT_STOP_POINT:

               break;
       }
   }

}
std::string OnCallDriverMovement::frame_tick_output()
{

    return DriverMovement::frame_tick_output();
}

void OnCallDriverMovement::performScheduleItem()
{
    try {
        if (onCallDriver->driverSchedule.isScheduleCompleted())
        {
            const Node *endOfPathNode = fwdDriverMovement.getCurrLink()->getToNode();
            continueCruising(endOfPathNode);
        }

        //Get the current schedule item
        auto itScheduleItem = onCallDriver->driverSchedule.getCurrScheduleItem();
        bool hasShiftEnded = onCallDriver->behaviour->hasDriverShiftEnded();
        switch (itScheduleItem->scheduleItemType) {
            case CRUISE: {
                beginCruising(itScheduleItem->nodeToCruiseTo);

                //We need to call the beginCruising method above even if the shift has ended,
                //this is to allow the driver to notify the controller and receive a unsubscribe successful
                //reply
                if (hasShiftEnded && !onCallDriver->isWaitingForUnsubscribeAck) {
                    onCallDriver->endShift();
                }
                break;
            }

            case PICKUP:
            {
                //Drive to pick up point
                beginDriveToPickUpPoint(itScheduleItem->tripRequest.startNode);
                break;
            }

        }
    }
    catch(no_path_error &ex)
    {
        //Log the error
        Warn() << ex.what() << endl;

        //We need to recover from this error. Let controller know of the error and get another
        //schedule?
    }
}
const Node* OnCallDriverBehaviour::chooseDownstreamNode(const Node *fromNode) const
{
    const RoadNetwork *rdNetwork = RoadNetwork::getInstance();
    auto downstreamLinks = rdNetwork->getDownstreamLinks(fromNode->getNodeId());
    DriverPathMover &pathMover = onCallDriver->movement->fwdDriverMovement;
    const Lane *currLane = pathMover.getCurrLane();

    vector<const Node *> reachableNodes;

    //If we are continuing from an existing path, we need to check for connectivity
    //from the current lane
    if(pathMover.isDrivingPathSet() && currLane)
    {
        auto mapTurningsVsLanes = rdNetwork->getTurningPathsFromLanes();
        auto itTurningsFromCurrLane = mapTurningsVsLanes.find(currLane);

#ifndef NDEBUG
        if(itTurningsFromCurrLane == mapTurningsVsLanes.end())
        {
            stringstream msg;
            msg << "No downstream nodes are reachable from node " << fromNode->getNodeId();
            throw runtime_error(msg.str());
        }
#endif

        //Add all nodes that are reachable from the current lane to vector
        for(auto it = itTurningsFromCurrLane->second.begin(); it != itTurningsFromCurrLane->second.end(); ++it)
        {
            reachableNodes.push_back(it->second->getToLane()->getParentSegment()->getParentLink()->getToNode());
        }
    }
    else
    {
        //We are starting from a node and currently have no lane, all downstream nodes are
        //reachable
        for(auto link : downstreamLinks)
        {
            reachableNodes.push_back(link->getToNode());
        }
    }

#ifndef NDEBUG
    if(reachableNodes.empty())
    {
        stringstream msg;
        msg << "No downstream nodes are reachable from node " << fromNode->getNodeId()
            << " and lane id " << (currLane ? currLane->getLaneId() : 0);
        throw runtime_error(msg.str());
    }
#endif

    //Select one node from the reachable nodes at random
    unsigned int random = Utils::generateInt(0, reachableNodes.size() - 1);
    return reachableNodes[random];

}
bool OnCallDriverBehaviour::hasDriverShiftEnded() const
{
    return (onCallDriver->getParent()->currTick.ms() / 1000) >= onCallDriver->getParent()->getServiceVehicle().endTime;
}

void OnCallDriverMovement::beginCruising(const Node *node)
{
    //Create a sub-trip for the route choice
    SubTrip subTrip;
    subTrip.origin = WayPoint(currNode);
    subTrip.destination = WayPoint(node);

    const Link *currLink = nullptr;
    const Lane *currLane;
    bool useInSimulationTT = onCallDriver->getParent()->usesInSimulationTravelTime();

    //If the driving path has already been set, we must find path to the node from
    //the current segment
    if(fwdDriverMovement.isDrivingPathSet())
    {
        currLink = fwdDriverMovement.getCurrLink();
    }

    //Get route to the node
    auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

#ifndef NDEBUG
    if(route.empty())
    {
        stringstream msg;
        msg << "Path not found. Driver " << onCallDriver->getParent()->getDatabaseId()
            << " could not find a path to the cruising node " << node->getNodeId()
            << " from the current node " << currNode->getNodeId() << " and link ";
        msg << (currLink ? currLink->getLinkId() : 0);
        throw no_path_error(msg.str());
    }

    ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
                    << onCallDriver->getParent()->getDatabaseId() << ": Begin cruising from node "
                    << getCurrentNode()->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
                    << " to node " << node->getNodeId() << endl;
#endif
   const vector<WayPoint> path =route;
    rerouteWithPath(path);
    currNode=node;
    onCallDriver->setDriverStatus(MobilityServiceDriverStatus::CRUISING);
}
Vehicle* OnCallDriverMovement::initialisePath(bool createVehicle)
{
    Vehicle *vehicle = nullptr;
    Person_ST *parent = onCallDriver->getParent();


    //Check if the next path has already been planned
    if (parent && !parent->getNextPathPlanned())
    {
        std::vector<const RoadSegment *> pathOfSegments;
        unsigned int vehicleId = 0;

        const double length = 4.0;
        const double width = 2.0;

        if (createVehicle)
        {
            vehicle = new Vehicle(VehicleBase::TAXI, vehicleId, length, width, "Taxi");
        }

        //Indicate that the path to next activity is already planned
        parent->setNextPathPlanned(true);
    }

    return vehicle;
}
void OnCallDriverMovement::beginDriveToPickUpPoint(const Node *pickupNode)
{
    //Create a sub-trip for the route choice
    SubTrip subTrip;
    subTrip.origin = WayPoint(currNode);
    subTrip.destination = WayPoint(pickupNode);

    const Link *currLink = nullptr;
    bool useInSimulationTT = onCallDriver->getParent()->usesInSimulationTravelTime();

    //If the driving path has already been set, we must find path to the node from
    //the current segment
    if(fwdDriverMovement.isDrivingPathSet())
    {
        auto currWayPt = fwdDriverMovement.getCurrWayPoint();

        if(currWayPt.type == WayPoint::ROAD_SEGMENT)
        {
            currLink = currWayPt.roadSegment->getParentLink();
        }
        else
        {
            currLink = fwdDriverMovement.getCurrTurning()->getToLane()->getParentSegment()->getParentLink();
        }

        //If the pickup node is at the end of the current link, we do not need to go anywhere
        //We can pick the passenger up at this point

        if(currLink->getToNode() == pickupNode)
        {
           // onCallDriver->pickupPassenger();
            performScheduleItem();
            return;
        }
    }

    //Get route to the node
    auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);

#ifndef NDEBUG
    if(route.empty())
    {
        stringstream msg;
        msg << "Path not found. Driver " << onCallDriver->getParent()->getDatabaseId()
            << " could not find a path to the pickup node " << pickupNode->getNodeId()
            << " from the current node " << currNode->getNodeId() << " and link ";
        msg << (currLink ? currLink->getLinkId() : 0);
        throw no_path_error(msg.str());
    }
#endif
    const vector<WayPoint> path =route;
    rerouteWithPath(path);
    StopPoint stopPoint;
    auto lastSeg = fwdDriverMovement.getDrivingPath().back().roadSegment;
    double lastroadsegmentlength = lastSeg->getLength();
    stopPoint.distance=lastroadsegmentlength/3.0;
    stopPoint.dwellTime = 5;
    stopPoint.segmentId = lastSeg->getRoadSegmentId();
    insertStopPoint(stopPoint);
    onCallDriver->getParams().currentStopPoint = stopPoint;

    onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_ON_CALL);
    onCallDriver->sendScheduleAckMessage(true);

    ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
                    << onCallDriver->getParent()->getDatabaseId() << ": Begin driving from node "
                    << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
                    << " to pickup node " << pickupNode->getNodeId() << "Stop Point "<< stopPoint.segmentId <<endl;

    //Set vehicle to moving
    onCallDriver->getResource()->setMoving(true);


 //   currNode=pickupNode;
}
void OnCallDriverMovement::insertStopPoint(StopPoint stopPt)
{
    std::map<unsigned int, std::vector<StopPoint> >::iterator it = onCallDriver->getParams().stopPointPool.find(stopPt.segmentId);
    if (it !=  onCallDriver->getParams().stopPointPool.end())
    {
        it->second.push_back(stopPt);
    }
    else
    {
        std::vector<StopPoint> v;
        v.push_back(stopPt);
        onCallDriver->getParams().stopPointPool.insert(std::make_pair(stopPt.segmentId, v));
    }
}
