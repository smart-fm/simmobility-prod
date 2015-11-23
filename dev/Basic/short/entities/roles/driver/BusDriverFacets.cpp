//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)


#include "BusDriverFacets.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusStopAgent.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/roles/waitBusActivityRole/WaitBusActivityRole.hpp"
#include "entities/UpdateParams.hpp"
#include "logging/Log.hpp"
#include "path/PathSetManager.hpp"

using namespace sim_mob;
using namespace std;

BusDriverMovement::BusDriverMovement() :
DriverMovement(), parentBusDriver(nullptr)
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

		if (bustrip && (*(parent->currTripChainItem))->itemType == TripChainItem::IT_BUSTRIP)
		{
			pathOfSegments = bustrip->getBusRouteInfo().getRoadSegments();
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
			vehicle = new Vehicle(VehicleBase::BUS, vehicleId, length, width);
			buildPath(pathOfSegments);
		}

		//Indicate that the path to next activity is already planned
		parent->setNextPathPlanned(true);
	}

	return vehicle;
}

void BusDriverMovement::buildPath(const std::vector<const RoadSegment *> &pathOfSegments)
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
				msg << "No turning between the links " << currLink->getLinkId() << " and " << nextLinkId << "!\nInvalid Path!!!";
				throw std::runtime_error(msg.str());
			}
		}
		
		++itSegments;
	}
	
	fwdDriverMovement.setPath(path);
}


void BusDriverMovement::frame_init()
{
	Vehicle* newVehicle = NULL;
	const Person_ST *parent = parentBusDriver->getParent();

	if (parent)
	{
		newVehicle = initialiseBusPath(true);
	}

	if (newVehicle)
	{
		BusRoute nullRoute;
		TripChainItem *tripChainItem = *(parent->currTripChainItem);
		BusTrip *busTrip = dynamic_cast<BusTrip *> (tripChainItem);

		if (!busTrip && busTrip->itemType == TripChainItem::IT_BUSTRIP)
		{
			throw std::runtime_error("BusDriver created without an appropriate BusTrip item.");
		}

		//Use the vehicle to build a bus, then delete the old vehicle.

		Vehicle *bus = new Bus(nullRoute, newVehicle, busTrip->getBusLine()->getBusLineID());
		parentBusDriver->setVehicle(bus);
		delete newVehicle;

		//Set the initial values of the parameters
		setOrigin(parentBusDriver->getParams());

		//Retrieve the bus stops for the bus
		busStops = busTrip->getBusRouteInfo().getBusStops();

		//Set initial speed of bus to 0
		parentBusDriver->getParams().initialSpeed = 0;
	}

	//Add the bus stops to the stop point pool
	for (int i = 0; i < busStops.size(); ++i)
	{
		const BusStop *stop = busStops[i];
		double dwelltime = 10;
		StopPoint stopPoint(stop->getParentSegment()->getRoadSegmentId(), stop->getOffset(), dwelltime);
		parentBusDriver->getParams().insertStopPoint(stopPoint);
	}
}

void BusDriverMovement::frame_tick()
{
	DriverMovement::frame_tick();
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

		Bus *bus = dynamic_cast<Bus *> (parentBusDriver->getVehicle());
		unsigned int passengerCount = 0;

		if (bus)
		{
			passengerCount = bus->getPassengerCount();
		}

		stringstream output;
		output << setprecision(8);
		output << "(\"BusDriver\"" << "," << params.now.frame() << "," << parentBusDriver->getParent()->getId()
				<< ",{" << "\"xPos\":\"" << parentBusDriver->getPositionX()
				<< "\",\"yPos\":\"" << parentBusDriver->getPositionY()
				<< "\",\"angle\":\"" << (360 - (baseAngle * 180 / M_PI))
				<< "\",\"length\":\"" << bus->getLengthInM()
				<< "\",\"width\":\"" << bus->getWidthInM()
				<< "\",\"passengers\":\"" << passengerCount				
				<< "\",\"buslineID\":\"" << bus->getBusLineID()
				<< addLine.str()
				<< "\",\"info\":\"" << params.debugInfo
				<< "\"})" << std::endl;
		
		return output.str();
	}
}
