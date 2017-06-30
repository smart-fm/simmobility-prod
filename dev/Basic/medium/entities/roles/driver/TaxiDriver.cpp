/*
 * TaxiDriver.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#include "entities/roles/driver/TaxiDriver.hpp"
#include "Driver.hpp"
#include "entities/controllers/MobilityServiceControllerManager.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"
#include "path/PathSetManager.hpp"

using namespace sim_mob;
using namespace medium;
using namespace messaging;

TaxiDriver::TaxiDriver(Person_MT* parent, const MutexStrategy& mtxStrat, TaxiDriverBehavior* behavior,
                       TaxiDriverMovement* movement, std::string roleName, Role<Person_MT>::Type roleType) :
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



const Node *TaxiDriver::getCurrentNode() const
{
	if (taxiDriverMovement)
	{
		return taxiDriverMovement->getCurrentNode();
	}
	return nullptr;
}

const MobilityServiceDriver *TaxiDriver::exportServiceDriver() const
{
#ifndef NDEBUG
	return dynamic_cast<const MobilityServiceDriver*> (this);
#else
	return this;
#endif
}

Passenger *TaxiDriver::getPassenger()
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
			const SegmentStats *segStats = pathMover.getCurrSegStats();
			Conflux *parentConflux = segStats->getParentConflux();
			parentConflux->dropOffTaxiTraveler(parentPerson);

			if(!taxiDriverMovement->isSubscribedToOnHail())
			{
				ControllerLog() << "Drop-off of user" << parentPerson->getDatabaseId() << " at time "
				                << parentPerson->currTick
				                << ". Message was sent at ??? with startNodeId ???, destinationNodeId "
				                << parentConflux->getConfluxNode()->getNodeId()
				                << ", and driverId " << getParent()->getDatabaseId() << std::endl;
			}
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
	const Lane *currentLane = taxiDriverMovement->getCurrentlane();
	currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPathAfterPassengerPickup(currSubTrip, false,
	                                                                                           nullptr, currentLane,
	                                                                                           useInSimulationTT);
}

void TaxiDriver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
	switch (type)
	{
	case MSG_SCHEDULE_PROPOSITION:
	{
		const SchedulePropositionMessage &msg = MSG_CAST(SchedulePropositionMessage, message);
		const Schedule &schedule = msg.getSchedule();
		const Node *node = NULL;

		for (Schedule::const_iterator scheduleItem = schedule.begin(); scheduleItem < schedule.end();)
		{
			switch (scheduleItem->scheduleItemType)
			{
			case ScheduleItemType::PICKUP :
			{
				const TripRequestMessage request = scheduleItem->tripRequest;
				scheduleItem++; // Now scheduleItem should point a dropoff

#ifndef NDEBUG
				if (scheduleItem->scheduleItemType != ScheduleItemType::DROPOFF)
				{
					throw std::runtime_error(
							"I expect a dropoff here. If it is a shared trip schedule, implement it here");
				}
				if (scheduleItem->tripRequest != request)
				{
					throw std::runtime_error(
							"We expected a drop off related to the same request of the pick up here, unless this is a shared trip schedule. In that case, please write your implementation here");
				}
#endif
				scheduleItem++; // Now scheduleItem should point to the end

#ifndef NDEBUG
				if (scheduleItem != schedule.end())
				{
					throw std::runtime_error(
							"For the moment, I would expect to have an empty schedule after getting a pick up and a drop off. If yoiu want to start handling sharing, please write your impleentation here");
				}
#endif

				ControllerLog() << "Assignment received for " << request<<". This assignment is received by driver "<<
						this->getParent()->getDatabaseId() << " at time "<<parent->currTick << std::endl;

				const std::map<unsigned int, Node *> &nodeIdMap = RoadNetwork::getInstance()->getMapOfIdvsNodes();

				std::map<unsigned int, Node *>::const_iterator it = nodeIdMap.find(request.startNodeId);

#ifndef NDEBUG
				if (it == nodeIdMap.end())
				{
					std::stringstream msg;
					msg << "The schedule received start with node " << it->first << " which is not valid";
					throw std::runtime_error(msg.str());
				}
				if (!MobilityServiceControllerManager::HasMobilityServiceControllerManager())
				{
					throw std::runtime_error(
							"MobilityServiceControllerManager::HasMobilityServiceControllerManager() == false");
				}
#endif

				node = it->second;

				const bool success = taxiDriverMovement->driveToNodeOnCall(request.userId, node);

#ifndef NDEBUG
				if (!success)
				{
					std::stringstream msg;
					msg << __FILE__ << ":" << __LINE__ << ": taxiDriverMovement->driveToNodeOnCall("
					    << request.userId << "," << node->getNodeId() << ");" << std::endl;
					WarnOut(msg.str());
				}
#endif

				MessageBus::PostMessage(message.GetSender(), MSG_SCHEDULE_PROPOSITION_REPLY,
				                        MessageBus::MessagePtr(new SchedulePropositionReplyMessage(parent->currTick,
				                                                                                   request.userId,
				                                                                                   parent,
				                                                                                   request.startNodeId,
				                                                                                   request.destinationNodeId,
				                                                                                   request.extraTripTimeThreshold,
				                                                                                   success)));

				ControllerLog() << "Assignment response sent for " << request << ". This response is sent by driver "
				                << parent->getDatabaseId() << " at time " << parent->currTick << std::endl;

				break;
			}
			case ScheduleItemType::CRUISE:
			{
				try
				{
					taxiDriverMovement->cruiseToNode((*scheduleItem).nodeToCruiseTo);
				}
				catch (exception &ex)
				{
					ControllerLog() << ex.what();
					Print() << ex.what();
				}

				scheduleItem++;
				break;
			}

			default:
				throw runtime_error("Schedule Item is invalid");

			}
		}
	}
	default:
	{
		break;
	}
	}
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
		Role<Person_MT> *curRole = personToPickUp->getRole();
		sim_mob::medium::Passenger *passenger = dynamic_cast<sim_mob::medium::Passenger *>(curRole);
		if (passenger)
		{
			std::vector<SubTrip>::iterator subTripItr = personToPickUp->currSubTrip;
			WayPoint personTravelDestination = (*subTripItr).destination;
			const Node *personDestinationNode = personTravelDestination.node;
			std::vector<WayPoint> currentRouteChoice;
			const Node *currentNode = taxiDriverMovement->getDestinationNode();
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
					const Lane *currentLane = taxiDriverMovement->getCurrentlane();
					const Link *currentLink = currentLane->getParentSegment()->getParentLink();
					currentRouteChoice.insert(currentRouteChoice.begin(), WayPoint(currentLink));
					taxiDriverMovement->setDestinationNode(personDestinationNode);
					taxiDriverMovement->setCurrentNode(currentNode);;
					taxiDriverMovement->addRouteChoicePath(currentRouteChoice);
					passenger->setStartPoint(WayPoint(taxiDriverMovement->getCurrentNode()));
					passenger->setEndPoint(WayPoint(taxiDriverMovement->getDestinationNode()));
					setDriverStatus(DRIVE_WITH_PASSENGER);
					passenger->Movement()->startTravelTimeMetric();
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

const std::vector<MobilityServiceController*>& TaxiDriver::getSubscribedControllers() const
{
	return taxiDriverMovement->getSubscribedControllers();
}

TaxiDriver::~TaxiDriver()
{
	if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
	{
		for(auto it = taxiDriverMovement->getSubscribedControllers().begin();
			it != taxiDriverMovement->getSubscribedControllers().end(); ++it)
		{
			MessageBus::PostMessage(*it, MSG_DRIVER_UNSUBSCRIBE,
			                        MessageBus::MessagePtr(new DriverUnsubscribeMessage(parent)));
		}
	}
}
