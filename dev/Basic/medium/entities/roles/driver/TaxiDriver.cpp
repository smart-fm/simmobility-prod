/*
 * TaxiDriver.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#include <entities/roles/driver/TaxiDriver.hpp>
#include "path/PathSetManager.hpp"
#include "Driver.hpp"
#include "geospatial/network/RoadNetwork.hpp"

namespace sim_mob
{
namespace medium
{


TaxiDriver::TaxiDriver(Person_MT* parent, const MutexStrategy& mtxStrat,
		TaxiDriverBehavior* behavior, TaxiDriverMovement* movement,
		std::string roleName, Role<Person_MT>::Type roleType) :
		Driver(parent, behavior, movement, roleName, roleType)
{
	taxiPassenger = nullptr;
	taxiDriverMovement = movement;
	taxiDriverBehaviour = behavior;
}

TaxiDriver::TaxiDriver(Person_MT* parent, const MutexStrategy& mtx) :
		Driver(parent, nullptr, nullptr, "", RL_TAXIDRIVER)
{

}

bool TaxiDriver::addPassenger(Passenger *passenger)
{
	if (taxiPassenger == nullptr)
	{
		taxiPassenger = passenger;
		return true;
	}
	return false;
}

Passenger* TaxiDriver::getPassenger()
{
	return taxiPassenger;
}

void TaxiDriver::alightPassenger()
{
	if (taxiPassenger != nullptr)
	{
		Passenger *passenger = taxiPassenger;
		taxiPassenger = nullptr;
		Person_MT *parentPerson = passenger->getParent();
		if (parentPerson)
		{
			MesoPathMover &pathMover = taxiDriverMovement->getMesoPathMover();
			const SegmentStats* segStats = pathMover.getCurrSegStats();
			Conflux *parentConflux = segStats->getParentConflux();
			parentConflux->dropOffTaxiTraveler(parentPerson);
		}
	}
}

void TaxiDriver::passengerChoiceModel(const Node *origin,const Node *destination, std::vector<WayPoint> &currentRouteChoice)
{
	std::vector<WayPoint> res;
	bool useInSimulationTT = parent->usesInSimulationTravelTime();
	SubTrip currSubTrip;
	currSubTrip.origin = WayPoint(origin);
	currSubTrip.destination = WayPoint(destination);
	const Lane * currentLane = taxiDriverMovement->getCurrentlane();
	currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPathAfterPassengerPickup(currSubTrip, false, nullptr, currentLane,useInSimulationTT);
}

void TaxiDriver::setTaxiDriveMode(const DriverMode &mode)
{
	taxiDriverMode = mode;
	driverMode = mode;
}

const DriverMode & TaxiDriver::getDriverMode() const
{
	return taxiDriverMode;
}

Person_MT *TaxiDriver::getParent()
{
	return parent;
}

TaxiDriverMovement *TaxiDriver::getMovementFacet()
{
	return taxiDriverMovement;
}

void TaxiDriver::pickUpPassngerAtNode(Conflux *parentConflux)
{
	if (!parentConflux)
	{
		return;
	}
	Person_MT *personToPickUp = parentConflux->pickupTaxiTraveler();
	if (personToPickUp)
	{
		std::string id = personToPickUp->getDatabaseId();
		Role<Person_MT>* curRole = personToPickUp->getRole();
		sim_mob::medium::Passenger* passenger = dynamic_cast<sim_mob::medium::Passenger*>(curRole);
		if (passenger)
		{
			std::vector<SubTrip>::iterator subTripItr = personToPickUp->currSubTrip;
			WayPoint personTravelDestination = (*subTripItr).destination;
			const Node * personDestinationNode = personTravelDestination.node;
			std::vector<WayPoint> currentRouteChoice;
			const Node * currentNode = taxiDriverMovement->getDestinationNode();
			if (currentNode == personDestinationNode)
			{
				return;
			}
			passengerChoiceModel(currentNode, personDestinationNode, currentRouteChoice);
			if (currentRouteChoice.size() > 0)
			{
				bool isAdded = addPassenger(passenger);
				if (isAdded)
				{
					const Lane * currentLane = taxiDriverMovement->getCurrentlane();
					const Link* currentLink = currentLane->getParentSegment()->getParentLink();
					currentRouteChoice.insert(currentRouteChoice.begin(), WayPoint(currentLink));
					taxiDriverMovement->setDestinationNode(personDestinationNode);
					taxiDriverMovement->setCurrentNode(currentNode);;
					taxiDriverMovement->addRouteChoicePath(currentRouteChoice);
					//passenger->setService(currentRouteChoice);
					passenger->setStartPoint(WayPoint(taxiDriverMovement->getCurrentNode()));
					passenger->setEndPoint(WayPoint(taxiDriverMovement->getDestinationNode()));
					setTaxiDriveMode(DRIVE_WITH_PASSENGER);
				}
			}
			/*else
			{
				sim_mob::BasicLogger& ptMoveLogger = sim_mob::Logger::log("nopathAfterPickupInCruising.csv");
				const SegmentStats* currentStats = taxiDriverMovement->getMesoPathMover().getCurrSegStats();
				if(currentStats)
				{
					ptMoveLogger << passenger->getParent()->getDatabaseId()<<",";
					ptMoveLogger << currentStats->getRoadSegment()->getLinkId()<<",";
					ptMoveLogger << currentStats->getRoadSegment()->getRoadSegmentId()<<",";
					ptMoveLogger << taxiDriverMovement->getCurrentNode()->getNodeId()<<",";
					ptMoveLogger << personDestinationNode->getNodeId()<<std::endl;
				}
			}*/
		}
	}
}

Role<Person_MT>* TaxiDriver::clone(Person_MT *parent) const
{
	if (parent)
	{
		TaxiDriverBehavior* behavior = new TaxiDriverBehavior();
		TaxiDriverMovement* movement = new TaxiDriverMovement();
		TaxiDriver* driver = new TaxiDriver(parent, parent->getMutexStrategy(),behavior, movement, "TaxiDriver_");
		behavior->setParentDriver(driver);
		movement->setParentDriver(driver);
		movement->setParentTaxiDriver(driver);
		return driver;
	}
	return nullptr;
}

void TaxiDriver::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}

std::vector<BufferedBase*> TaxiDriver::getSubscriptionParams()
{
	return std::vector<BufferedBase*>();
}

TaxiDriver::~TaxiDriver()
{
}
}
}

