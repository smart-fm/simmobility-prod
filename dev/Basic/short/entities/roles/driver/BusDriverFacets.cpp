//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "BusDriverFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/roles/waitBusActivity/WaitBusActivity.hpp"
#include "entities/UpdateParams.hpp"
#include "logging/Log.hpp"
#include "message/MessageBus.hpp"
#include "path/PathSetManager.hpp"
#include "util/Utils.hpp"
#include "config/ST_Config.hpp"
#include "message/ST_Message.hpp"

using namespace sim_mob;
using namespace std;

const unsigned int ticksToProcessBoardingAlighting = 5;

BusDriverMovement::BusDriverMovement() :
DriverMovement(), parentBusDriver(nullptr), isBusStopNotified(false), tickCountBoardingAlighting(0)
{
}

BusDriverMovement::~BusDriverMovement()
{
}

Vehicle* BusDriverMovement::initialiseBusPath(bool createVehicle)
{
    Vehicle *vehicle = nullptr;
    Person_ST *parent = parentBusDriver->getParent();

    //Check if the next path has already been planned
    if (parent && !parent->getNextPathPlanned())
    {
        std::vector<const RoadSegment *> pathOfSegments;
        unsigned int vehicleId = 0;
        const BusTrip *bustrip = dynamic_cast<const BusTrip *> (*(parent->currTripChainItem));
        std::string busRoute;

        if (bustrip && (*(parent->currTripChainItem))->itemType == TripChainItem::IT_BUSTRIP)
        {
            pathOfSegments = bustrip->getBusRouteInfo().getRoadSegments();
            busRoute = bustrip->getBusRouteInfo().getBusRouteId();
            vehicleId = bustrip->getVehicleID();
        }
        else
        {
            Print() << "BusTrip path not initialised. Trip chain item type: " << (*(parent->currTripChainItem))->itemType
                    << std::endl;
        }

        const double length = 12.0;
        const double width = 2.0;

        if (createVehicle)
        {
            vehicle = new Vehicle(VehicleBase::BUS, vehicleId, length, width, "Bus");
            buildPath(busRoute, pathOfSegments);
        }

        //Indicate that the path to next activity is already planned
        parent->setNextPathPlanned(true);
    }

    return vehicle;
}

void BusDriverMovement::buildPath(const std::string &routeId, const std::vector<const RoadSegment *> &pathOfSegments)
{
    std::vector<WayPoint> path;
    std::vector<const RoadSegment *>::const_iterator itSegments = pathOfSegments.begin();
    
    while(itSegments != pathOfSegments.end())
    {
        //Create a Way-point for every road segment in the path of road-segments and add it to the final path
        path.push_back(WayPoint(*itSegments));
        
        //If there is a change in links between the segments, add the turning group that connects the two links
        if(itSegments + 1 != pathOfSegments.end() && (*itSegments)->getLinkId() != (*(itSegments + 1))->getLinkId())
        {
            const Link *currLink = (*itSegments)->getParentLink();
            unsigned int nextLinkId = (*(itSegments + 1))->getLinkId();
            
            //Get the turning group between this link and the next link and add it to the path
            
            const TurningGroup *turningGroup = currLink->getToNode()->getTurningGroup(currLink->getLinkId(), nextLinkId);
            
            if (turningGroup)
            {
                path.push_back(WayPoint(turningGroup));
            }
            else
            {
                stringstream msg;
                msg << __func__ << ": No turning between the links " << currLink->getLinkId() << " and " << nextLinkId;
                msg << "\nInvalid Path for Bus route " << routeId;
                throw std::runtime_error(msg.str());
            }
        }
        
        ++itSegments;
    }

    //Set invalid value as default, so that the vehicle loading model will choose a lane
    int laneIdx = -1;
    auto person = parentBusDriver->getParent();

    //If the first link has a bus stop, we simply set the slowest lane as the selected lane - as it is
    //the only one connected to the stop
    if(path.front().roadSegment->getParentLink() == (*busStopTracker)->getParentSegment()->getParentLink())
    {
        laneIdx = 0;
    }

    //Choose the starting lane and initial speed using the vehicle loading model
    vehLoadingModel->chooseStartingLaneAndSpeed(path, &laneIdx, person->startSegmentId, &(person->initialSpeed),
                                                parentBusDriver->getParams());
    fwdDriverMovement.setPath(path, laneIdx);
}


void BusDriverMovement::frame_init()
{
    Vehicle* newVehicle = NULL;
    const Person_ST *parent = parentBusDriver->getParent();

    if (parent)
    {
        TripChainItem *tripChainItem = *(parent->currTripChainItem);
        BusTrip *busTrip = dynamic_cast<BusTrip *> (tripChainItem);

        if (!busTrip && busTrip->itemType == TripChainItem::IT_BUSTRIP)
        {
            std::stringstream msg;
            msg << __func__ << ": BusDriver created without an appropriate BusTrip item.";
            throw std::runtime_error(msg.str());
        }

        const string &busLine = busTrip->getBusLine()->getBusLineID();

        parentBusDriver->setBusLineId(busLine);

        //Retrieve the bus stops for the bus
        busStops = busTrip->getBusRouteInfo().getBusStops();

        //Track the bus stops.
        busStopTracker = busStops.begin();

        newVehicle = initialiseBusPath(true);
    }

    if (newVehicle)
    {
        //Use the vehicle to build a bus, then delete the old vehicle.
        parentBusDriver->setVehicle(newVehicle);

        //Set the initial values of the parameters
        setOrigin(parentBusDriver->getParams());
    }
}

void BusDriverMovement::frame_tick()
{
    DriverMovement::frame_tick();
    
    DriverUpdateParams &params = parentBusDriver->getParams();
    
    switch(params.stopPointState)
    {
    case DriverUpdateParams::ARRIVED_AT_STOP_POINT:
    case DriverUpdateParams::WAITING_AT_STOP_POINT:
        
        //If the bus has arrived at the stop and the bus stop agent has not been notified, notify it
        if (!isBusStopNotified)
        {
            //Send bus arrival message to all passengers in the bus
            for (Person_ST *person : parentBusDriver->passengerList)
            {
                messaging::MessageBus::PostMessage(person, MSG_WAKEUP_BUS_PAX,
                        messaging::MessageBus::MessagePtr(new BusStopMessage(*busStopTracker, parentBusDriver)));
            }

            //Signal the bus stop agent to handle the bus arrival
            parentBusDriver->currBusStopAgent = BusStopAgent::getBusStopAgentForStop(*busStopTracker);
            messaging::MessageBus::PostMessage(parentBusDriver->currBusStopAgent, MSG_WAKEUP_WAITING_PERSON,
                    messaging::MessageBus::MessagePtr(new BusDriverMessage(parentBusDriver)));

            //Set default random dwell time and reset the current boarding & alighting times
            params.currentStopPoint.dwellTime = Utils::nRandom(10, 2);
            parentBusDriver->currBoardingTime = 0;
            parentBusDriver->currAlightingTime = 0;

            //Set bus stop notified as true
            isBusStopNotified = true;

            //Store bus arrival time
            DailyTime currTime(params.now.ms() + (params.elapsedSeconds / 1000) + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
            busArrivalTime = currTime.getStrRepr();

            break;
        }

        //Boarding and alighting times can be computed only after processing messages from boarding persons and alighting passengers
        //This takes at-most 3 frame ticks, so maintain count when bus arrives at stop or is waiting
        if (tickCountBoardingAlighting <= ticksToProcessBoardingAlighting)
        {
            tickCountBoardingAlighting++;           
        }

        //Compute the dwell time and send the bus arrival message for statistics collection.
        //Note: Sending the message out at this point only because dwell time was previously unknown. Also not the arrival time
        //used is the one stored when the bus actually arrived
        if (tickCountBoardingAlighting == ticksToProcessBoardingAlighting)
        {
            //Compute dwell time
            params.currentStopPoint.dwellTime += std::max(parentBusDriver->currBoardingTime, parentBusDriver->currAlightingTime);

            BusTrip *busTrip = static_cast<BusTrip *> (*(parentBusDriver->getParent()->currTripChainItem));

            //Send bus arrival message
            PT_ArrivalTime busArrivalInfo;
            busArrivalInfo.serviceLine = parentBusDriver->getBusLineId();
            busArrivalInfo.tripId = busTrip->tripID;
            busArrivalInfo.sequenceNo = parentBusDriver->sequenceNum++;
            busArrivalInfo.arrivalTime = busArrivalTime;
            busArrivalInfo.dwellTimeSecs = params.currentStopPoint.dwellTime;
            //DailyTime requires time in milli-seconds to create proper string representation, so convert to milli-seconds
            busArrivalInfo.dwellTime = (DailyTime(1000 * params.currentStopPoint.dwellTime)).getStrRepr();
            //Compute percentage of occupancy
            busArrivalInfo.pctOccupancy = (((double) parentBusDriver->passengerList.size()) / ST_Config::getInstance().defaultBusCapacity) * 100.0;
            busArrivalInfo.stopNo = (*busStopTracker)->getStopCode();

            messaging::MessageBus::PostMessage(PT_Statistics::getInstance(), STORE_BUS_ARRIVAL,
                    messaging::MessageBus::MessagePtr(new PT_ArrivalTimeMessage(busArrivalInfo)));
            
            break;
        }
        
        break;
    
    case DriverUpdateParams::LEAVING_STOP_POINT:
        
        if(isBusStopNotified)
        {
            //Reset
            isBusStopNotified = false;
            tickCountBoardingAlighting = 0;

            //Next bus stop
            ++busStopTracker;
        }
        
        break;
    }
}

std::string BusDriverMovement::frame_tick_output()
{
    DriverUpdateParams &params = parentBusDriver->getParams();
    
    if (this->getParentDriver()->IsVehicleInLoadingQueue() || fwdDriverMovement.isDoneWithEntireRoute())
    {
        return std::string();
    }

    if (ConfigManager::GetInstance().CMakeConfig().OutputEnabled())
    {
        double baseAngle = getAngle();

        //MPI-specific output.
        std::stringstream addLine;
        if (ConfigManager::GetInstance().FullConfig().using_MPI)
        {
            addLine << "\",\"fake\":\"" << (parentBusDriver->getParent()->isFake ? "true" : "false");
        }

        Vehicle *bus = parentBusDriver->getVehicle();

        stringstream output;
        output << setprecision(8);
        output << "(\"BusDriver\"" << "," << params.now.frame() << "," << parentBusDriver->getParent()->getId()
                << ",{" << "\"xPos\":\"" << parentBusDriver->getPositionX()
                << "\",\"yPos\":\"" << parentBusDriver->getPositionY()
                << "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI))
                << "\",\"length\":\"" << bus->getLengthInM()
                << "\",\"width\":\"" << bus->getWidthInM()
                << "\",\"passengers\":\"" << parentBusDriver->passengerList.size()
                << "\",\"buslineID\":\"" << parentBusDriver->getBusLineId()
                << addLine.str()
                << "\",\"info\":\"" << params.debugInfo
                << "\"})" << std::endl;
        
        return output.str();
    }
    return {};
}

void BusDriverMovement::checkForStops(DriverUpdateParams& params)
{
    if(busStopTracker != busStops.end())
    {
        //Get the distance to stopping point in the current link
        double distance = getDistanceToStopLocation(params.stopVisibilityDistance);
        params.distanceToStoppingPt = distance;

        if (distance > -10 || params.stopPointState == DriverUpdateParams::ARRIVED_AT_STOP_POINT)
        {
            //Leaving the stopping point
            if (distance < 0 && params.stopPointState == DriverUpdateParams::LEAVING_STOP_POINT)
            {
                return;
            }

            //Change state to Approaching stop point
            if (params.stopPointState == DriverUpdateParams::STOP_POINT_NOT_FOUND)
            {
                params.stopPointState = DriverUpdateParams::STOP_POINT_FOUND;
            }

            //Change state to stopping point is close
            if (distance >= 5 && distance <= 50)
            {
                // 10m-50m
                params.stopPointState = DriverUpdateParams::ARRIVING_AT_STOP_POINT;
            }

            //Change state to arrived at stop point
            if (params.stopPointState == DriverUpdateParams::ARRIVING_AT_STOP_POINT && abs(distance) < 5)
            {
                // 0m-10m
                params.stopPointState = DriverUpdateParams::ARRIVED_AT_STOP_POINT;
            }

            params.distToStop = distance;

            return;
        }

        if (distance < -10 && params.stopPointState == DriverUpdateParams::LEAVING_STOP_POINT)
        {
            params.stopPointState = DriverUpdateParams::STOP_POINT_NOT_FOUND;
        }
    }
}

double BusDriverMovement::getDistanceToStopLocation(double perceptionDistance)
{
    //Distance to stop
    double distance = -100;
    
    double scannedDist = 0;
    bool isStopFound = false;
    DriverUpdateParams &params = parentBusDriver->getParams();

    std::vector<WayPoint>::const_iterator wayPtIt = fwdDriverMovement.getCurrWayPointIt();
    std::vector<WayPoint>::const_iterator endOfPath = fwdDriverMovement.getDrivingPath().end();

    //Find the next road segment of the next stop
    const RoadSegment *rdSegWithStop = (*busStopTracker)->getParentSegment();
    
    //Check if the stop is in the current segment
    if(wayPtIt->type == WayPoint::ROAD_SEGMENT && wayPtIt->roadSegment == rdSegWithStop)
    {
        //Distance to stop is offset - distance covered on the way point;
        distance = (*busStopTracker)->getOffset() - fwdDriverMovement.getDistCoveredOnCurrWayPt();
        isStopFound = true;
    }

    scannedDist = fwdDriverMovement.getDistToEndOfCurrWayPt();

    //We have already considered the length (whatever is remaining) of the current way-point,
    //now we need to start from the next one
    ++wayPtIt;

    //Iterate through the path till the perception distance or the end (whichever is before)
    while (!isStopFound && wayPtIt != endOfPath && scannedDist < perceptionDistance)
    {
        if (wayPtIt->type == WayPoint::ROAD_SEGMENT)
        {
            //If the way point is a road segment, check if it contains the next stop
            if (wayPtIt->roadSegment == rdSegWithStop)
            {
                //Distance to the bus stop is the scanned distance plus the distance to the bus stop from start of the segment (offset)
                distance = scannedDist + (*busStopTracker)->getOffset();
                break;
            }
            else
            {
                //Segment doesn't contain the bus stop, just increment scanned distance with length of segment
                scannedDist += wayPtIt->roadSegment->getLength();
            }
        }
        else
        {
            //No bus stops on turning groups, just add the length to scanned distance
            scannedDist += wayPtIt->turningGroup->getLength();
        }
        
        ++wayPtIt;
    }

    return distance;
}
