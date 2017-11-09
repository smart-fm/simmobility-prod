//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "OnCallDriver.hpp"
#include "conf/ConfigManager.hpp"

using namespace sim_mob;
using namespace medium;
using namespace messaging;
using namespace std;

OnCallDriver::OnCallDriver(Person_MT *parent, const MutexStrategy &mtx, OnCallDriverBehaviour *behaviour,
                           OnCallDriverMovement *movement, string roleName, Type roleType) :
		Driver(parent, behaviour, movement, roleName, roleType), movement(movement), behaviour(behaviour),
		isWaitingForUnsubscribeAck(false)
{
}

OnCallDriver::OnCallDriver(Person_MT *parent) : Driver(parent)
{
}

OnCallDriver::~OnCallDriver()
{
	//The driver should not be destroyed when it has a passenger. If it is being destroyed, then this
	//is an error scenario
#ifndef NDEBUG
	if(!passengers.empty())
	{
		stringstream msg;
		msg << "OnCallDriver " << parent->getDatabaseId() << " is being destroyed, but it has "
		    << passengers.size() << " passenger(s).";
		throw runtime_error(msg.str());
	}
#endif
}

Role<Person_MT>* OnCallDriver::clone(Person_MT *person) const
{
#ifndef NDEBUG
	if(person == nullptr)
	{
		return nullptr;
	}
#endif

	OnCallDriverMovement *driverMvt = new OnCallDriverMovement();
	OnCallDriverBehaviour *driverBhvr = new OnCallDriverBehaviour();
	OnCallDriver *driver = new OnCallDriver(person, person->getMutexStrategy(), driverBhvr, driverMvt, "OnCallDriver");

	driverBhvr->setParentDriver(driver);
	driverBhvr->setOnCallDriver(driver);
	driverMvt->setParentDriver(driver);
	driverMvt->setOnCallDriver(driver);

	return driver;
}

void OnCallDriver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message &message)
{
	switch (type)
	{
	case MSG_SCHEDULE_PROPOSITION:
	{
		const SchedulePropositionMessage &msg = MSG_CAST(SchedulePropositionMessage, message);
		driverSchedule.setSchedule(msg.getSchedule());
		movement->performScheduleItem();
		break;
	}
	case MSG_SCHEDULE_UPDATE:
	{
		const SchedulePropositionMessage &msg = MSG_CAST(SchedulePropositionMessage, message);
		const Schedule &updatedSchedule = msg.getSchedule();

		//As this is an updated schedule, this will be a partial schedule. It contains only items that the controller
		//knows the driver has not completed. So, we check if the item we are performing currently has been
		//re-scheduled. If so, we must discontinue it and start performing the new sequence. Else, we continue
		//whatever we were doing. In either case, we must update the schedule
		if(currentItemRescheduled(updatedSchedule))
		{
			driverSchedule.setSchedule(updatedSchedule);
			movement->performScheduleItem();
		}
		else
		{
			driverSchedule.updateSchedule(updatedSchedule);
		}
		break;
	}
	case MSG_UNSUBSCRIBE_SUCCESSFUL:
	{
		parent->setToBeRemoved();
		break;
	}
	case MSG_WAKEUP_SHIFT_END:
	{
		//Only when the driver is parked we need to handle this message, in other cases the driver is already
		//awake and can end the shift
		if(driverStatus == PARKED)
		{
			reload();
			endShift();
		}
		break;
	}
	}
}

const Node* OnCallDriver::getCurrentNode() const
{
	return movement->getCurrentNode();
}

const vector<MobilityServiceController *>& OnCallDriver::getSubscribedControllers() const
{
	return subscribedControllers;
}

Schedule OnCallDriver::getAssignedSchedule() const
{
	return driverSchedule.getSchedule();
}

unsigned long OnCallDriver::getPassengerCount() const
{
	return passengers.size();
}

const MobilityServiceDriver* OnCallDriver::exportServiceDriver() const
{
	return this;
}

void OnCallDriver::subscribeToOrIgnoreController(const SvcControllerMap& controllers, MobilityServiceControllerType type)
{
	if (parent->getServiceVehicle().controllerSubscription & type)
	{
		auto range = controllers.equal_range(type);

#ifndef NDEBUG
		if (range.first == range.second)
		{
			std::stringstream msg;
			msg << "OnCallDriver " << parent->getDatabaseId() << " wants to subscribe to type "
			    << toString(type) << ", but no controller of that type is registered";
			throw std::runtime_error(msg.str());
		}
#endif

		for (auto itController = range.first; itController != range.second; ++itController)
		{
			MessageBus::PostMessage(itController->second, MSG_DRIVER_SUBSCRIBE,
			                        MessageBus::MessagePtr(new DriverSubscribeMessage(parent)));

#ifndef NDEBUG
			ControllerLog() << "OnCallDriver " << parent->getDatabaseId()
			                << " sent a subscription to the controller "
			                << itController->second->toString() << " at time " << parent->currTick;
			ControllerLog() << ". parentDriver pointer " << parent << endl;
#endif

			subscribedControllers.push_back(itController->second);
		}
	}
}

void OnCallDriver::scheduleItemCompleted()
{
	driverSchedule.itemCompleted();

	sendStatusMessage();

	if(driverSchedule.isScheduleCompleted())
	{
		sendAvailableMessage();
	}
}


bool OnCallDriver::currentItemRescheduled(const Schedule &updatedSchedule)
{
	auto currItem = *(driverSchedule.getCurrScheduleItem());

	for(auto schItem : updatedSchedule)
	{
		if(schItem == currItem)
		{
			return true;
		}
	}

	return false;
}

void OnCallDriver::sendScheduleAckMessage(bool success)
{
	auto tripRequest = driverSchedule.getCurrScheduleItem()->tripRequest;

	//Acknowledge the acceptance of the schedule
	SchedulePropositionReplyMessage *ackMsg = new SchedulePropositionReplyMessage(parent->currTick,
	                                                                               tripRequest.userId,
	                                                                               parent,
	                                                                               tripRequest.startNode,
	                                                                               tripRequest.destinationNode,
	                                                                               tripRequest.extraTripTimeThreshold,
	                                                                               success);

	MessageBus::PostMessage(tripRequest.GetSender(), MSG_SCHEDULE_PROPOSITION_REPLY, MessageBus::MessagePtr(ackMsg));
}

void OnCallDriver::sendAvailableMessage()
{
	//Notify the controller(s)
	for(auto ctrlr : subscribedControllers)
	{
		MessageBus::PostMessage(ctrlr, MSG_DRIVER_AVAILABLE,
		                        MessageBus::MessagePtr(new DriverAvailableMessage(parent)));
	}
}

void OnCallDriver::sendStatusMessage()
{
	//Notify the controller(s)
	for(auto ctrlr : subscribedControllers)
	{
		MessageBus::PostMessage(ctrlr, MSG_DRIVER_SCHEDULE_STATUS,
		                        MessageBus::MessagePtr(new DriverScheduleStatusMsg(parent)));
	}
}

void OnCallDriver::sendWakeUpShiftEndMsg()
{
	unsigned int timeToShiftEnd = (parent->getServiceVehicle().endTime * 1000) - parent->currTick.ms();
	unsigned int tick = ConfigManager::GetInstance().FullConfig().baseGranMS();

	Conflux *cflx = movement->getMesoPathMover().getCurrSegStats()->getParentConflux();
	MessageBus::PostMessage(cflx, MSG_WAKEUP_SHIFT_END, MessageBus::MessagePtr(new PersonMessage(parent)),
		                        false, timeToShiftEnd / tick);
}

void OnCallDriver::reload()
{
	//We are starting afresh from the parking node, so we need to set the current segment stats
	parent->setCurrSegStats(movement->getMesoPathMover().getCurrSegStats());
	Conflux *conflux = Conflux::getConfluxFromNode(movement->getCurrentNode());
	MessageBus::PostMessage(conflux, MSG_PERSON_LOAD, MessageBus::MessagePtr(new PersonMessage(parent)));
}

void OnCallDriver::pickupPassenger()
{
	//Get the conflux
	MesoPathMover &pathMover = movement->getMesoPathMover();
	const SegmentStats *currSegStats = pathMover.getCurrSegStats();
	Conflux *conflux = currSegStats->getParentConflux();

	//Get the passenger name from the schedule
	const string &passengerId = driverSchedule.getCurrScheduleItem()->tripRequest.userId;

	Person_MT *personPickedUp = conflux->pickupTaxiTraveler(passengerId);

#ifndef NDEBUG
	if (!personPickedUp)
	{
		stringstream msg;
		msg << "Pickup failed for " << personPickedUp << " at time " << parent->currTick
		    << ", and driverId " << parent->getDatabaseId() << ". personToPickUp is NULL" << std::endl;
		throw runtime_error(msg.str());
	}
#endif

	Role<Person_MT> *curRole = personPickedUp->getRole();
	Passenger *passenger = dynamic_cast<Passenger *>(curRole);

#ifndef NDEBUG
	if (!passenger)
	{
		stringstream msg;
		msg << "Pickup failed for " << passengerId << " at time " << parent->currTick
		    << ", and driverId " << parent->getDatabaseId() << ". personToPickUp is not a passenger"
		    << std::endl;
		throw runtime_error(msg.str());
	}
#endif

	//Add the passenger
	passengers[passengerId] = passenger;
	passenger->setDriver(this);
	passenger->setStartPoint(personPickedUp->currSubTrip->origin);
	passenger->setStartPointDriverDistance(movement->getTravelMetric().distance);
	passenger->setEndPoint(personPickedUp->currSubTrip->destination);
	passenger->Movement()->startTravelTimeMetric();

	ControllerLog() << "Pickup succeeded for " << passengerId << " at time " << parent->currTick
	                << " with startNodeId " << conflux->getConfluxNode()->getNodeId() << ", destinationNodeId "
	                << personPickedUp->currSubTrip->destination.node->getNodeId()
	                << ", and driverId " << parent->getDatabaseId() << std::endl;

	//Mark schedule item as completed
	scheduleItemCompleted();
}

void OnCallDriver::dropoffPassenger()
{
	//Get the passenger to be dropped off
	const string &passengerId = driverSchedule.getCurrScheduleItem()->tripRequest.userId;
	auto itPassengers = passengers.find(passengerId);

#ifndef NDEBUG
	if(itPassengers == passengers.end())
	{
		stringstream msg;
		msg << "Dropoff failed for " << passengerId << " at time " << parent->currTick
		    << ", and driverId " << parent->getDatabaseId() << ". Passenger not present in vehicle"
		    << std::endl;
		throw runtime_error(msg.str());
	}
#endif

	Passenger *passengerToBeDroppedOff = itPassengers->second;
	Person_MT *person = passengerToBeDroppedOff->getParent();

	MesoPathMover &pathMover = movement->getMesoPathMover();
	const SegmentStats *segStats = pathMover.getCurrSegStats();
	Conflux *conflux = segStats->getParentConflux();
	passengerToBeDroppedOff->setFinalPointDriverDistance(movement->getTravelMetric().distance);
	conflux->dropOffTaxiTraveler(person);

	//Remove passenger from vehicle
	passengers.erase(itPassengers);

	ControllerLog() << "Drop-off of user " << person->getDatabaseId() << " at time "
	                << person->currTick << ", destinationNodeId " << conflux->getConfluxNode()->getNodeId()
	                << ", and driverId " << getParent()->getDatabaseId() << std::endl;

	//Mark schedule item as completed
	scheduleItemCompleted();
}

void OnCallDriver::endShift()
{
	//Notify the controller(s)
	/*for(auto ctrlr : subscribedControllers)
	{
		MessageBus::PostMessage(ctrlr, MSG_DRIVER_SHIFT_END,
		                        MessageBus::MessagePtr(new DriverShiftCompleted(parent)));
	}

	isWaitingForUnsubscribeAck = true;*/

#ifndef NDEBUG
	ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
	                << parent->getDatabaseId() << ": Shift ended"  << endl;
#endif
}

