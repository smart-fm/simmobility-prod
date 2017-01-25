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


	TaxiDriver::TaxiDriver(Person_MT* parent, const MutexStrategy& mtxStrat,TaxiDriverBehavior* behavior,
				TaxiDriverMovement* movement, std::string roleName, Role<Person_MT>::Type roleType) :
				Driver(parent, behavior, movement, roleName, roleType)
	{
			taxiPassenger = nullptr;
			taxiDriverMovement=movement;
			taxiDriverBehaviour = behavior;
	}

	TaxiDriver::TaxiDriver(Person_MT* parent, const MutexStrategy& mtx):Driver(parent,nullptr,nullptr,"",RL_TAXIDRIVER)
	{

	}

	bool TaxiDriver::addPassenger(Passenger *passenger)
	{
		if(taxiPassenger == nullptr)
		{
			taxiPassenger = passenger;
			personBoarded = true;
			return true;
		}
		else
		{
			return false;
		}
	}

	Passenger* TaxiDriver::getPassenger()
	{
		return taxiPassenger;
	}

	Passenger * TaxiDriver::alightPassenger()
	{
		if(taxiPassenger != nullptr)
		{
			Passenger *passenger = taxiPassenger;
			taxiPassenger = nullptr;
			Person_MT *parentPerson = passenger->getParent();
			if(parentPerson)
			{
				MesoPathMover &pathMover = taxiDriverMovement->getMesoPathMover();
				const SegmentStats* segStats = pathMover.getCurrSegStats();
				Conflux *parentConflux = segStats->getParentConflux();
				parentConflux->dropOffTaxiTraveler(parentPerson);
			}
		}
	}

	void TaxiDriver::runRouteChoiceModel(const Node *origin,const Node *destination,SubTrip &currSubTrip,std::vector<WayPoint> &currentRouteChoice)
	{
		std::vector<WayPoint> res;
		bool useInSimulationTT = parent->usesInSimulationTravelTime();
		SubTrip dummySubTrip;
		currSubTrip.origin = WayPoint(origin);
		currSubTrip.destination = WayPoint(destination);
		const Lane *currentLane = taxiDriverMovement->getCurrentlane();
		currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPathAfterPassengerPickup(currSubTrip,false, nullptr,currentLane,useInSimulationTT);
	}

	const  DriverMode & TaxiDriver::getDriverMode() const
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

	void TaxiDriver::checkPersonsAndPickUpAtNode(Conflux *parentConflux,bool isFirstPickupAttempt)
	{
		//pick up the first person from the node and advance its trip chain
		if(!parentConflux)
		{
			return;
		}
		std::deque<Person_MT*> & travellingPersons = parentConflux->getTravellingPersons();
		Person_MT *personToPickUp = parentConflux->pickupTaxiTraveler();
		//if the route of the person to destination is not found then just ignore him for now
		//else need to check the route choice path before pickup
		if(personToPickUp)
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
					if(!isFirstPickupAttempt)
					{
						currentNode = destinationNode;
						destinationNode = personDestinationNode;
						if (currentNode == destinationNode)
						{
							return;
						}
						taxiDriverMovement->setCurrentNode(currentNode);
						taxiDriverMovement->setDestinationNode(destinationNode);
						runRouteChoiceModel(currentNode,destinationNode,*subTripItr,currentRouteChoice);
					}
					else
					{
						if( currentNode == destinationNode)
						{
							return;
						}
						destinationNode = personDestinationNode;
						taxiDriverMovement->setDestinationNode(destinationNode);
						taxiDriverMovement->eraseEntirePath();
						runRouteChoiceModel(currentNode,destinationNode,*subTripItr,currentRouteChoice);
					}

					if(currentRouteChoice.size() > 0)
					{
						bool isAdded = addPassenger(passenger);
						if(isAdded)
						{
							taxiDriverMovement->addRouteChoicePath(currentRouteChoice,parentConflux);
							//passenger->setStartPoint(WayPoint(currentNode));
							//passenger->Movement()->startTravelTimeMetric();
							const DriverMode &mode = DRIVE_WITH_PASSENGER;
							taxiDriverMode = mode;
							driverMode = mode;
							pickupNode = currentNode;
						}
					}
				}
			}
		}

	Role<Person_MT>* TaxiDriver::clone(Person_MT *parent) const
	{
		if(parent)
		{
			TaxiDriverBehavior* behavior = new TaxiDriverBehavior();
			TaxiDriverMovement* movement = new TaxiDriverMovement();
			TaxiDriver* driver = new TaxiDriver(parent, parent->getMutexStrategy(),behavior, movement, "TaxiDriver_");
			behavior->setParentDriver(driver);
			movement->setParentDriver(driver);
			movement->setParentTaxiDriver(driver);
			return driver;
		}
	}

	void TaxiDriver::setCurrentNode(const Node *currNode)
	{
		currentNode = currNode;
	}

	void TaxiDriver::setDestinationNode(const Node *destinationNode)
	{
		this->destinationNode = destinationNode;
	}

	const Node * TaxiDriver::getDestinationNode()
	{
		return destinationNode;
	}

	const Node * TaxiDriver::getCurrentNode()
	{
		return currentNode;
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

