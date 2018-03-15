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

		onCallDriver->subscribeToController();

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
		case DRIVE_ON_CALL:
		{
			//Reached pick up node, pick-up passenger and perform next schedule item
			//Note: OnCallDriver::pickupPassenger marks the schedule item complete and moves to
			//the next item
                DriverUpdateParams &params = onCallDriver->getParams();
                switch (params.stopPointState)
                {
                    case DriverUpdateParams::ARRIVED_AT_STOP_POINT:
                    case DriverUpdateParams::WAITING_AT_STOP_POINT:
                    {
                     /*   if (!pickedUpPasssenger && !pickedUpAnotherPasssenger)
                        {
                            onCallDriver->pickupPassenger();
                            pickedUpPasssenger = true;
                            passengerWaitingtobeDroppedOff = true;
                        }
                       else if(pickedUpPasssenger && !pickedUpAnotherPasssenger)
                        {
                            if(!passengerWaitingtobeDroppedOff)
                            {
                                onCallDriver->pickupPassenger();
                                pickedUpAnotherPasssenger = true;
                                passengerWaitingtobeDroppedOff = true;
                            }
                        }*/
                        if(leftPickupStopPoint ==true)
                        {
                            onCallDriver->pickupPassenger();
                            leftPickupStopPoint =false;
                        }
                        break;
                    }
                    case DriverUpdateParams::LEAVING_STOP_POINT:
                    {
                       /* if((pickedUpPasssenger || pickedUpAnotherPasssenger) && passengerWaitingtobeDroppedOff)
                        {
                            passengerWaitingtobeDroppedOff = false;
                            params.stopPointPool.clear();
                            performScheduleItem();
                        }
                        */
                        if(leftPickupStopPoint == false)
                        {
                            params.stopPointPool.clear();
                            performScheduleItem();
                            leftPickupStopPoint = true;
                        }
                        break;
                    }
                }

			break;
		}

		case DRIVE_WITH_PASSENGER:
		{
			//Reached destination, drop-off passenger and perform next schedule item
			//Note: OnCallDriver::pickupPassenger marks the schedule item complete and moves to
			//the next item
                DriverUpdateParams &params = onCallDriver->getParams();

                /* if driver arrived at stop point then message the travelling person about his arrival*/
                switch (params.stopPointState) {
                    case DriverUpdateParams::ARRIVED_AT_STOP_POINT:
                    case DriverUpdateParams::WAITING_AT_STOP_POINT: {
/*                        if (pickedUpPasssenger) {

                            onCallDriver->dropoffPassenger();
                            droppedOffPassenger = true;
                            pickedUpPasssenger = false;
                            passengerWaitingtobeDroppedOff = true;
                        } else if (pickedUpAnotherPasssenger && !passengerWaitingtobeDroppedOff) {
                            onCallDriver->dropoffPassenger();
                            droppedOffAnotherPassenger = true;
                            pickedUpAnotherPasssenger = false;
                        }*/
                        if(leftDropOffStopPoint == true)
                        {
                            onCallDriver->dropoffPassenger();
                            leftDropOffStopPoint = false;
                        }
                        break;
                    }
                    case DriverUpdateParams::LEAVING_STOP_POINT: {
                   /*     if (droppedOffPassenger) {
                            droppedOffPassenger = false;

                            passengerWaitingtobeDroppedOff = false;
                            onCallDriver->scheduleItemCompleted();
                            params.stopPointPool.clear();
                            performScheduleItem();
                        } else if (droppedOffAnotherPassenger) {
                            droppedOffAnotherPassenger = false;

                            passengerWaitingtobeDroppedOff = false;
                            onCallDriver->scheduleItemCompleted();
                            params.stopPointPool.clear();
                            performScheduleItem();
                        }*/
                        if(leftDropOffStopPoint == false)
                        {
                            onCallDriver->scheduleItemCompleted();
                            params.stopPointPool.clear();
                            leftDropOffStopPoint = true;
                            performScheduleItem();
                        }
                        break;
                    }
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

   }
    /*
 if(fwdDriverMovement.isDoneWithEntireRoute())
    {
        //DriverMovement::frame_tick();
        cout<<"Curr lane: " <<fwdDriverMovement.getCurrLane()<<endl;
     //   DriverUpdateParams &params= onCallDriver->getParams();
      // reRouteToDestination(params,fwdDriverMovement.getCurrLane());
    }
    else*/
    {
        if (!(fwdDriverMovement.isDoneWithEntireRoute()) && fwdDriverMovement.getCurrWayPoint().type == WayPoint::ROAD_SEGMENT)
        {
           currNode = fwdDriverMovement.getCurrWayPoint().roadSegment->getParentLink()->getFromNode();
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

			 case DROPOFF:
			 {
				 //Drive to drop off point
				 beginDriveToDropOffPoint(itScheduleItem->tripRequest.destinationNode);
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
     const Node* nonblackListedNode = nullptr;

	 //If we are continuing from an existing path, we need to check for connectivity
	 //from the current lane
	 if(pathMover.isDrivingPathSet() && currLane)
	 {
		 auto itTurningsFromCurrLane = rdNetwork->getTurningPathsFromLanes().find(currLane);

 #ifndef NDEBUG
		 if(itTurningsFromCurrLane == rdNetwork->getTurningPathsFromLanes().end())
		 {
			 stringstream msg;
			 msg << "No downstream nodes are reachable from node " << fromNode->getNodeId();
			 throw runtime_error(msg.str());
		 }
 #endif

		 //Add all nodes that are reachable from the current lane to vector
		 for(auto it = itTurningsFromCurrLane->second.begin(); it != itTurningsFromCurrLane->second.end(); ++it)
         {
             if (onCallDriver->movement->blackListedNodes.find(
                     it->second->getToLane()->getParentSegment()->getParentLink()->getToNode()->getNodeId()) !=
                 onCallDriver->movement->blackListedNodes.end())
             {
                 nonblackListedNode = chooseRandomNode();
                 return nonblackListedNode;
             }
             else
             {
                 reachableNodes.push_back(it->second->getToLane()->getParentSegment()->getParentLink()->getToNode());
             }
         }
	 }
	 else
	 {
		 //We are starting from a node and currently have no lane, all downstream nodes are
		 //reachable
		 for(auto link : downstreamLinks)
		 {
             if(onCallDriver->movement->blackListedNodes.find(link->getToNode()->getNodeId()) !=
                onCallDriver->movement->blackListedNodes.end())
             {
                 nonblackListedNode = chooseRandomNode();
                 return nonblackListedNode;
             }
             else
             {
                 reachableNodes.push_back(link->getToNode());
             }
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
     const Node *selectedNode = reachableNodes[random];

     //Check if we've selected a node which is the same as the fromNode
     //This can happen when there are small loops in the network and we will fail to get an
     //updated path
     while(selectedNode == fromNode)
     {
         //Choose a random node anywhere in the network
         selectedNode = chooseRandomNode();
     }

     return selectedNode;
 }
const Node* OnCallDriverBehaviour::chooseRandomNode() const
{
    auto nodeMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();
    auto itRandomNode = nodeMap.begin();
    advance(itRandomNode, Utils::generateInt(0, nodeMap.size() - 1));

    const Node *result = itRandomNode->second;

    //Ensure chosen node is not a source/sink node
    if(result->getNodeType() == SOURCE_OR_SINK_NODE || onCallDriver->movement->blackListedNodes.find(result->getNodeId())!=onCallDriver->movement->blackListedNodes.end())
    {
        result = chooseRandomNode();
    }

    return result;
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
	 bool useInSimulationTT = onCallDriver->getParent()->usesInSimulationTravelTime();

	 //If the driving path has already been set, we must find path to the node from
	 //the current segment
	 if(fwdDriverMovement.isDrivingPathSet())
	 {
             currLink = fwdDriverMovement.getCurrLink();
	 }

	 //Get route to the node
	 auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);
     //Get shortest path if path is not found in the path-set
     if(route.empty())
     {
         if(currLink)
         {
             route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *node);
         }
         else
         {
             route = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*currNode, *node);
         }
     }
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
                     << currNode->getNodeId()<< " and link " << (currLink ? currLink->getLinkId() : 0)
                     << " to node " << node->getNodeId() << endl;


 #endif
	const vector<WayPoint> path =route;
	 rerouteWithPath(path);
     onCallDriver->getParent()->destNode = WayPoint(node);
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
             subTrip.origin = WayPoint(currLink->getFromNode());

             if(currLink->getToNode() == pickupNode)
             {
                 StopPoint stopPoint;
                 auto lastSeg = fwdDriverMovement.getDrivingPath().back().roadSegment;
                 double lastroadsegmentlength = lastSeg->getLength();
                 stopPoint.distance=2*(lastroadsegmentlength/3.0);
                 stopPoint.dwellTime = 5;
                 stopPoint.segmentId = lastSeg->getRoadSegmentId();
                 onCallDriver->getParams().insertStopPoint(stopPoint);
                 onCallDriver->getParams().currentStopPoint = stopPoint;
                 if(fwdDriverMovement.getDistCoveredOnCurrWayPt()< stopPoint.distance)
                 {
                     onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_ON_CALL);
                     onCallDriver->sendScheduleAckMessage(true);

                     ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
                                     << onCallDriver->getParent()->getDatabaseId() << ": Begin driving to pickup point" <<endl;
                     onCallDriver->getParent()->destNode = WayPoint(pickupNode);
                     //Set vehicle to moving
                     onCallDriver->getResource()->setMoving(true);
                     return;
                 }
             }
		 }
		 else {
             currLink = fwdDriverMovement.getCurrTurning()->getToLane()->getParentSegment()->getParentLink();
             subTrip.origin = WayPoint(currLink->getFromNode());
             if (currLink->getToNode() == pickupNode) {
                 StopPoint stopPoint;
                 auto lastSeg = fwdDriverMovement.getDrivingPath().back().roadSegment;
                 double lastroadsegmentlength = lastSeg->getLength();
                 stopPoint.distance = 2 * (lastroadsegmentlength / 3.0);
                 stopPoint.dwellTime = 5;
                 stopPoint.segmentId = lastSeg->getRoadSegmentId();
                 onCallDriver->getParams().insertStopPoint(stopPoint);
                 onCallDriver->getParams().currentStopPoint = stopPoint;
                 if (fwdDriverMovement.getDistCoveredOnCurrWayPt() < stopPoint.distance) {
                     onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_ON_CALL);
                     onCallDriver->sendScheduleAckMessage(true);

                     ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
                                     << onCallDriver->getParent()->getDatabaseId() << ": Begin driving to pickup point"
                                     << endl;
                     onCallDriver->getParent()->destNode = WayPoint(pickupNode);
                     //Set vehicle to moving
                     onCallDriver->getResource()->setMoving(true);
                     return;
                 }
             }
         }
	 }

	 //Get route to the node
	 auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);
     //Get shortest path if path is not found in the path-set
     if(route.empty())
     {
         if(currLink)
         {
             route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *pickupNode);
         }
         else
         {
             route = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*currNode, *pickupNode);
         }
     }
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
	 stopPoint.distance=2*(lastroadsegmentlength/3.0);
	 stopPoint.dwellTime = 5;
	 stopPoint.segmentId = lastSeg->getRoadSegmentId();
	 onCallDriver->getParams().insertStopPoint(stopPoint);
	 onCallDriver->getParams().currentStopPoint = stopPoint;

	 onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_ON_CALL);
	 onCallDriver->sendScheduleAckMessage(true);

	 ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
					 << onCallDriver->getParent()->getDatabaseId() << ": Begin driving from node "
					 << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
					 << " to pickup node " << pickupNode->getNodeId() << " Stop Point Segment Id "<< stopPoint.segmentId <<endl;
     onCallDriver->getParent()->destNode = WayPoint(pickupNode);

	 //Set vehicle to moving
	 onCallDriver->getResource()->setMoving(true);
 }

void OnCallDriverMovement::beginDriveToDropOffPoint(const Node *dropOffNode)
{
	//Create a sub-trip for the route choice
	SubTrip subTrip;
	subTrip.origin = WayPoint(currNode);
	subTrip.destination = WayPoint(dropOffNode);

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
            subTrip.origin = WayPoint(currLink->getFromNode());
            if(currLink->getToNode() == dropOffNode)
            {
                StopPoint stopPoint;
                auto lastSeg = fwdDriverMovement.getDrivingPath().back().roadSegment;
                double lastroadsegmentlength = lastSeg->getLength();
                stopPoint.distance=2*(lastroadsegmentlength/3.0);
                stopPoint.dwellTime = 5;
                stopPoint.segmentId = lastSeg->getRoadSegmentId();
                onCallDriver->getParams().insertStopPoint(stopPoint);
                onCallDriver->getParams().currentStopPoint = stopPoint;
                if(fwdDriverMovement.getDistCoveredOnCurrWayPt()< stopPoint.distance)
                {
                    onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_WITH_PASSENGER);
                    onCallDriver->sendScheduleAckMessage(true);

                    ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
                                    << onCallDriver->getParent()->getDatabaseId() << ": Begin driving with passenger from node "
                                    << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
                                    << " to drop off node " << dropOffNode->getNodeId() << endl;
                    onCallDriver->getParent()->destNode = WayPoint(dropOffNode);
                    //Set vehicle to moving
                    onCallDriver->getResource()->setMoving(true);
                    return;
                }
            }
        }
        else {
            currLink = fwdDriverMovement.getCurrTurning()->getToLane()->getParentSegment()->getParentLink();
            subTrip.origin = WayPoint(currLink->getFromNode());
            if (currLink->getToNode() == dropOffNode) {
                StopPoint stopPoint;
                auto lastSeg = fwdDriverMovement.getDrivingPath().back().roadSegment;
                double lastroadsegmentlength = lastSeg->getLength();
                stopPoint.distance = 2 * (lastroadsegmentlength / 3.0);
                stopPoint.dwellTime = 5;
                stopPoint.segmentId = lastSeg->getRoadSegmentId();
                onCallDriver->getParams().insertStopPoint(stopPoint);
                onCallDriver->getParams().currentStopPoint = stopPoint;
                if (fwdDriverMovement.getDistCoveredOnCurrWayPt() < stopPoint.distance) {
                    onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_WITH_PASSENGER);
                    onCallDriver->sendScheduleAckMessage(true);

                    ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
                                    << onCallDriver->getParent()->getDatabaseId() << ": Begin driving with passenger from node "
                                    << currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
                                    << " to drop off node " << dropOffNode->getNodeId() << endl;

                    onCallDriver->getParent()->destNode = WayPoint(dropOffNode);
                    //Set vehicle to moving
                    onCallDriver->getResource()->setMoving(true);
                    return;
                }
            }
        }
	}

	//Get route to the node
	auto route = PrivateTrafficRouteChoice::getInstance()->getPath(subTrip, false, currLink, useInSimulationTT);
    if(route.empty())
    {
        if(currLink)
        {
            route = StreetDirectory::Instance().SearchShortestDrivingPath<Link, Node>(*currLink, *dropOffNode);
        }
        else
        {
            route = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*currNode, *dropOffNode);
        }
    }
#ifndef NDEBUG
	if(route.empty())
	{
		stringstream msg;
		msg << "Path not found. Driver " << onCallDriver->getParent()->getDatabaseId()
			<< " could not find a path to the drop off node " << dropOffNode->getNodeId()
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
	stopPoint.distance=2*(lastroadsegmentlength/3.0);
	stopPoint.dwellTime = 5;
	stopPoint.segmentId = lastSeg->getRoadSegmentId();
	onCallDriver->getParams().insertStopPoint(stopPoint);
	onCallDriver->getParams().currentStopPoint = stopPoint;
	onCallDriver->setDriverStatus(MobilityServiceDriverStatus::DRIVE_WITH_PASSENGER);

	ControllerLog() << onCallDriver->getParent()->currTick.ms() << "ms: OnCallDriver "
					<< onCallDriver->getParent()->getDatabaseId() << ": Begin driving with passenger from node "
					<< currNode->getNodeId() << " and link " << (currLink ? currLink->getLinkId() : 0)
					<< " to drop off node " << dropOffNode->getNodeId() << endl;
    onCallDriver->getParent()->destNode = WayPoint(dropOffNode);
   // cout<<"set dropoffnode to :"<<dropOffNode->getNodeId()<<endl;
    //Set vehicle to moving
	onCallDriver->getResource()->setMoving(true);
}