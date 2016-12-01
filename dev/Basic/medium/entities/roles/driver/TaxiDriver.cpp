/*
 * TaxiDriver.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#include <entities/roles/driver/TaxiDriver.hpp>
#include "path/PathSetManager.hpp"
#include "Driver.hpp"

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
			return true;
		}
		else
		{
			return false;
		}
	}

	Passenger * TaxiDriver::alightPassenger()
	{
		Passenger *passenger = taxiPassenger;
		taxiPassenger = nullptr;
	}

	void TaxiDriver::runRouteChoiceModel(const Node *origin,const Node *destination,SubTrip &currSubTrip,std::vector<WayPoint> &currentRouteChoice)
	{
		std::vector<WayPoint> res;
		bool useInSimulationTT = parent->usesInSimulationTravelTime();
		currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);
	}

	const  DriverMode & TaxiDriver::getDriverMode() const
	{
		return taxiDriverMode;
	}

	Person *TaxiDriver::getParent()
	{
		return parent;
	}

	TaxiDriverMovement *TaxiDriver::getMovementFacet()
	{
		return taxiDriverMovement;
	}

	void TaxiDriver::checkPersonsAndPickUpAtNode(timeslice now,Conflux *parentConflux)
	{
		//pick up the first person from the node and advance its trip chain
		std::deque<Person_MT*> & travellingPersons = parentConflux->getTravellingPersons();
		if(travellingPersons.size()>0)
		{
			Person_MT *personToPickUp = *(travellingPersons.begin());
			//DailyTime current(DailyTime(now.ms()));//.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
			personToPickUp->checkTripChain(now.ms());
			Role<Person_MT>* curRole = personToPickUp->getRole();
			//curRole->setArrivalTime(now.ms());
			sim_mob::medium::Passenger* passenger = dynamic_cast<sim_mob::medium::Passenger*>(curRole);
			if (passenger)
			{
				bool isAdded = addPassenger(passenger);
				if(isAdded)
				{
					std::vector<SubTrip>::iterator subTripItr = personToPickUp->currSubTrip;
					WayPoint personTravelDestination = (*subTripItr).destination;
					const Node * personDestinationNode = personTravelDestination.node;
					currentNode = destinationNode;
					destinationNode = personDestinationNode;
					std::vector<WayPoint> currentRouteChoice;
					runRouteChoiceModel(currentNode,destinationNode,*subTripItr,currentRouteChoice);
					taxiDriverMovement->addRouteChoicePath(currentRouteChoice,parentConflux);
					passenger->setStartPoint(WayPoint(currentNode));
					passenger->Movement()->startTravelTimeMetric();
					const DriverMode &mode = DRIVE_WITH_PASSENGER;
					taxiDriverMode = mode;
					driverMode = mode;
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

