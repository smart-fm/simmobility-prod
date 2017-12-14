//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "OnHailDriver.hpp"
#include "entities/controllers/MobilityServiceController.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/TaxiStandAgent.hpp"

using namespace sim_mob;
using namespace medium;
using namespace std;

OnHailDriver::OnHailDriver(Person_MT *parent, const MutexStrategy &mtx, OnHailDriverBehaviour *behaviour,
                           OnHailDriverMovement *movement, string roleName, Type roleType) :
		Driver(parent, behaviour, movement, roleName, roleType), passenger(nullptr), movement(movement),
		behaviour(behaviour), toBeRemovedFromTaxiStand(false), isExitingTaxiStand(false)
{
}

OnHailDriver::OnHailDriver(Person_MT *parent) : Driver(parent)
{
}

OnHailDriver::~OnHailDriver()
{
	//The driver should not be destroyed when it has a passenger. If it is being destroyed, then this
	//is an error scenario
#ifndef NDEBUG
	if(passenger)
	{
		stringstream msg;
		msg << "OnHailDriver " << parent->getDatabaseId() << " is being destroyed, but it has a passenger.";
		throw runtime_error(msg.str());
	}
#endif
}

Role<Person_MT>* OnHailDriver::clone(Person_MT *person) const
{
#ifndef NDEBUG
	if(person == nullptr)
	{
		return nullptr;
	}
#endif

	OnHailDriverMovement *driverMvt = new OnHailDriverMovement();
	OnHailDriverBehaviour *driverBhvr = new OnHailDriverBehaviour();
	OnHailDriver *driver = new OnHailDriver(person, person->getMutexStrategy(), driverBhvr, driverMvt, "OnHailDriver");

	driverBhvr->setParentDriver(driver);
	driverBhvr->setOnHailDriver(driver);
	driverMvt->setParentDriver(driver);
	driverMvt->setOnHailDriver(driver);

	return driver;
}

void OnHailDriver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message &msg)
{
}

const Node* OnHailDriver::getCurrentNode() const
{
	return movement->getCurrentNode();
}

const vector<MobilityServiceController *>& OnHailDriver::getSubscribedControllers() const
{
	return subscribedControllers;
}

Schedule OnHailDriver::getAssignedSchedule() const
{
	stringstream msg;
	msg << "Driver " << parent->getDatabaseId() << " is an OnHailDriver. It will never have a schedule, "
	    << "this method should not be called on an OnHailDriver";
	throw runtime_error(msg.str());
}

void OnHailDriver::collectTravelTime()
{
	//Do nothing, as we do not collect travel times for on hail drivers
}

unsigned long OnHailDriver::getPassengerCount() const
{
	return (passenger ? 1 : 0);
}

Person_MT *OnHailDriver::tryTaxiStandPickUp()
{
	Person_MT *passenger = nullptr;
	TaxiStandAgent *taxiStandAgent = TaxiStandAgent::getTaxiStandAgent(movement->getChosenTaxiStand());

	//Only drivers at the head of the queue can pick up passengers
	if(taxiStandAgent->isTaxiFirstInQueue(parent))
	{
		passenger = taxiStandAgent->pickupOneWaitingPerson();
	}

	return passenger;
}

bool OnHailDriver::tryEnterTaxiStand(const SegmentStats *currSegStats, const TaxiStand *taxiStand)
{
	bool enteredStand = false;

	//Check if the current segment has the given taxi stand
	if(taxiStand && currSegStats->hasTaxiStand(taxiStand))
	{
		//Get the taxi stand agent and query it to check if the taxi can be accommodated in
		//the stand
		TaxiStandAgent *taxiStandAgent = TaxiStandAgent::getTaxiStandAgent(taxiStand);

#ifndef NDEBUG
		if(!taxiStandAgent)
		{
			stringstream msg;
			msg << "TaxiStand " << taxiStand->getStandId() << " does not have a TaxiStandAgent!";
			throw runtime_error(msg.str());
		}
#endif

		enteredStand = taxiStandAgent->acceptTaxiDriver(parent);
	}

	return enteredStand;
}

Person_MT* OnHailDriver::tryPickUpPassengerAtNode(const Node *node, const string& personId)
{
	Conflux *conflux = Conflux::getConfluxFromNode(node);

#ifndef NDEBUG
	if(!conflux)
	{
		stringstream msg;
		msg << "Node " << node->getNodeId() << " does not have a Conflux associated with it!";
		throw runtime_error(msg.str());
	}
#endif

	return conflux->pickupTraveller(personId);
}

void OnHailDriver::addPassenger(Person_MT *person)
{
#ifndef NDEBUG
	if(passenger)
	{
		stringstream msg;
		msg << "OnHailDriver " << parent->getDatabaseId() << " attempting to pick-up another passenger! "
		    << passenger->getParent()->getDatabaseId() << " is already in the car";
		throw runtime_error(msg.str());
	}
#endif

	Role<Person_MT> *personRole = person->getRole();
	passenger = dynamic_cast<Passenger *>(personRole);

#ifndef NDEBUG
	if(!passenger)
	{
		stringstream msg;
		msg << "Person " << person->getDatabaseId() << " picked up by driver " << parent->getDatabaseId()
		    << " does not have a passenger role!";
		throw runtime_error(msg.str());
	}

	ControllerLog() << parent->currTick.ms() << "ms: OnHailDriver " << parent->getDatabaseId() << ": Picked-up "
	                << person->getDatabaseId() << " while " << getDriverStatusStr() << endl;
#endif
}

void OnHailDriver::alightPassenger()
{
#ifndef NDEBUG
	if(!passenger)
	{
		stringstream msg;
		msg << "OnHailDriver " << parent->getDatabaseId() << " is trying to alight a passenger, but doesn't have "
		    << " a passenger";
		throw runtime_error(msg.str());
	}
#endif

	Person_MT *person = passenger->getParent();
	const SegmentStats *currSegStats = movement->getMesoPathMover().getCurrSegStats();
	Conflux *conflux = currSegStats->getParentConflux();

	conflux->dropOffTraveller(person);
	passenger = nullptr;

#ifndef NDEBUG
	ControllerLog() << parent->currTick.ms() << "ms: OnHailDriver " << parent->getDatabaseId() << ": Dropped-off "
	                << person->getDatabaseId() << " at node "
	                << currSegStats->getRoadSegment()->getParentLink()->getToNode()->getNodeId() << endl;
#endif
}