/*
 * TaxiDriver.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#include <entities/roles/driver/TaxiDriver.hpp>
#include "Driver.hpp"
#include "entities/controllers/MobilityServiceControllerManager.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"

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

MobilityServiceDriver::ServiceStatus TaxiDriver::getServiceStatus()
{
	if(getDriverMode()==CRUISE)
	{
		return MobilityServiceDriver::SERVICE_FREE;
	}
	return MobilityServiceDriver::SERVICE_UNKNOWN;
}

const Node* TaxiDriver::getCurrentNode()
{
	if(taxiDriverMovement)
	{
		return taxiDriverMovement->getCurrentNode();
	}
	return nullptr;
}
MobilityServiceDriver* TaxiDriver::exportServiceDriver()
{
	return this;
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

			ControllerLog() << "Drop-off for " << parentPerson->getDatabaseId() << " at time " << parentPerson->currTick.frame()
				<< ". Message was sent at null with startNodeId null, destinationNodeId " << parentConflux->getConfluxNode()->getNodeId()
				<< ", and driverId null" << std::endl;
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

void TaxiDriver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
		case MSG_SCHEDULE_PROPOSITION:
		{
			const SchedulePropositionMessage& msg = MSG_CAST(SchedulePropositionMessage, message);

			ControllerLog() << "Assignment received for " << msg.personId << " at time " << parent->currTick.frame()
				<< ". Message was sent at " << msg.currTick.frame() << " with startNodeId " << msg.startNodeId
				<< ", destinationNodeId " << msg.destinationNodeId << ", and driverId null" << std::endl;

			std::map<unsigned int, Node*> nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

			std::map<unsigned int, Node*>::iterator it = nodeIdMap.find(msg.startNodeId); 
			if (it == nodeIdMap.end()) {
				ControllerLog() << "Message contains bad start node " << msg.startNodeId << std::endl;

				if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
				{
					std::map<unsigned int, MobilityServiceController*> controllers = MobilityServiceControllerManager::GetInstance()->getControllers();
				
					messaging::MessageBus::SendMessage(controllers[1], MSG_SCHEDULE_PROPOSITION_REPLY,
						messaging::MessageBus::MessagePtr(new SchedulePropositionReplyMessage(parent->currTick, msg.personId, parent,
							msg.startNodeId, msg.destinationNodeId, msg.extraTripTimeThreshold, false)));

					ControllerLog() << "Assignment response sent for " << msg.personId << " at time " << parent->currTick.frame()
						<< ". Message was sent at " << msg.currTick.frame() << " with startNodeId " << msg.startNodeId
						<< ", destinationNodeId " << msg.destinationNodeId << ", and driverId null" << std::endl;
				}

				return;
			}
			Node* node = it->second;

			const bool success = taxiDriverMovement->driveToNodeOnCall(msg.personId, node);

			if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
			{
				std::map<unsigned int, MobilityServiceController*> controllers = MobilityServiceControllerManager::GetInstance()->getControllers();

				messaging::MessageBus::SendMessage(controllers[1], MSG_SCHEDULE_PROPOSITION_REPLY,
					messaging::MessageBus::MessagePtr(new SchedulePropositionReplyMessage(parent->currTick, msg.personId, parent,
						msg.startNodeId, msg.destinationNodeId, msg.extraTripTimeThreshold, success)));

				ControllerLog() << "Assignment response sent for " << msg.personId << " at time " << parent->currTick.frame()
					<< ". Message was sent at " << msg.currTick.frame() << " with startNodeId " << msg.startNodeId
					<< ", destinationNodeId " << msg.destinationNodeId << ", and driverId " << parent->getDatabaseId() << std::endl;
			}

			break;
		}
		default:
		{
			break;
		}
	}
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

void TaxiDriver::pickUpPassngerAtNode(Conflux *parentConflux, std::string* personId)
{
	if (!parentConflux)
	{
		return;
	}
	Person_MT *personToPickUp = parentConflux->pickupTaxiTraveler(personId);
	if (personToPickUp)
	{
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
					passenger->setStartPoint(WayPoint(taxiDriverMovement->getCurrentNode()));
					passenger->setEndPoint(WayPoint(taxiDriverMovement->getDestinationNode()));
					setTaxiDriveMode(DRIVE_WITH_PASSENGER);
				}
			}
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

	if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
	{
		std::map<unsigned int, MobilityServiceController*> controllers = MobilityServiceControllerManager::GetInstance()->getControllers();

		messaging::MessageBus::SendMessage(controllers[1], MSG_DRIVER_UNSUBSCRIBE,
			messaging::MessageBus::MessagePtr(new DriverUnsubscribeMessage(parent)));
	}
}
}
}



