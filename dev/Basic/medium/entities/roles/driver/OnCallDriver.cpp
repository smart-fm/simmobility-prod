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
		isWaitingForUnsubscribeAck(false), isScheduleUpdated(false), toBeRemovedFromParking(false),
		isExitingParking(false)
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

		//Set the schedule updated to true, so that we perform the schedule item during the
		//frame tick
		isScheduleUpdated = true;

		//Set this to true so that the driver is removed from the parking
		//(even if it is not it doesn't matter)
		toBeRemovedFromParking = true;
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

			//Set the schedule updated to true, so that we perform the schedule item during the
			//frame tick
			isScheduleUpdated = true;
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
	case MSG_DELAY_SHIFT_END:
	{
		//As the driver has been assigned a schedule exactly at the time it sent a shift-end message,
		//the controller will not unsubscribe the driver immediately, but ask it to delay the shift-end.
		//Once the driver completes the schedule, it can perform the tasks in the endShift() method again
		//and then wait for the acknowledgement for the unsubscribe message.
		isWaitingForUnsubscribeAck = false;
		break;
	}
	case MSG_WAKEUP_SHIFT_END:
	{
		//Only when the driver is parked we need to handle this message, in other cases the driver is already
		//awake and can end the shift
		if(driverStatus == PARKED)
		{
			toBeRemovedFromParking = true;
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

void OnCallDriver::subscribeToController()
{
	auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();
	const unsigned int subscribedCtrlr = parent->getServiceVehicle().controllerSubscription;

	//Look for the driver's subscribed controller in the multi-index
	SvcControllerMap::index<ctrlrId>::type::iterator it = controllers.get<ctrlrId>().find(subscribedCtrlr);

#ifndef NDEBUG
	if (it == controllers.get<ctrlrId>().end())
	{
		std::stringstream msg;
		msg << "OnCallDriver " << parent->getDatabaseId() << " wants to subscribe to id "
		    << subscribedCtrlr << ", but no controller of that id is registered";
		throw std::runtime_error(msg.str());
	}
#endif

	MessageBus::PostMessage(*it, MSG_DRIVER_SUBSCRIBE, MessageBus::MessagePtr(new DriverSubscribeMessage(parent)));

#ifndef NDEBUG
	ControllerLog() << "OnCallDriver " << parent->getDatabaseId() << "(" << parent << ")"
	                << " sent a subscription to the controller "
	                << (*it)->toString() << " at time " << parent->currTick<< endl;
#endif

	subscribedControllers.push_back(*it);
}

void OnCallDriver::scheduleItemCompleted()
{
	driverSchedule.itemCompleted();

	if(behaviour->hasDriverShiftEnded() && driverSchedule.isScheduleCompleted())
	{
		//If the shift has ended, we no longer need to send the status message
		//and the available message. We simply wait for the shift end confirmation
		return;
	}

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

void OnCallDriver::pickupPassenger()
{
	//Get the conflux
	MesoPathMover &pathMover = movement->getMesoPathMover();
	const SegmentStats *currSegStats = pathMover.getCurrSegStats();
	medium::Conflux* conflux = NULL;
	if(currSegStats)
	{
		conflux = currSegStats->getParentConflux();
	}
	else //This is the case when we have need to pickup passenger immediately .Mean driver is at same position where he need to pickup.
	{
		conflux =  Conflux::getConfluxFromNode(getCurrentNode());
#ifndef NDEBUG
		if(!conflux)
		{
			 stringstream msg;
			 msg << "Node " << getCurrentNode()->getNodeId() << " does not have a Conflux associated with it!";
			throw runtime_error(msg.str());
		}
#endif
	}
	//Indicates whether we need to pick up another person at this point
	bool pickupAnotherPerson = false;
	do
	{
		//Get the passenger name from the schedule
		auto currItem = driverSchedule.getCurrScheduleItem();
		const string &passengerId = currItem->tripRequest.userId;
		Person_MT *personPickedUp =NULL;
		if(conflux)
		{
			personPickedUp = conflux->pickupTraveller(passengerId);
		}
		else
		{
			Print()<<"Conflux not found. can not pickup Passenger"<<endl;
		}
#ifndef NDEBUG
		if (!personPickedUp)
		{
			stringstream msg;
			msg << "Pickup failed for " << passengerId << " at time " << parent->currTick
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
		//Check if the next schedule item is also a pickup and at the same node
		//Note: This can happen when a controller that assigns shared requests is being used
		//Note: We have marked the previous schedule item as completed, so the current schedule item
		//iterator has been updated
		if(!driverSchedule.isScheduleCompleted())
		{
			auto nxtItem = driverSchedule.getCurrScheduleItem();
			pickupAnotherPerson = nxtItem->scheduleItemType == PICKUP
			&& currItem->tripRequest.startNode == nxtItem->tripRequest.startNode;
			if (pickupAnotherPerson)
			{
				ControllerLog() << "These persons will be picking up from same node :(" << passengerId << " |" <<
				nxtItem->tripRequest.userId << ") by driver " << parent->getDatabaseId() << " from Node " <<
				currItem->tripRequest.startNode->getNodeId() << endl;
			}
		}
		else
		{
			pickupAnotherPerson =false;
		}
	}while(pickupAnotherPerson);
}
void OnCallDriver::dropoffPassenger()
{
	//Indicates whether we need to drop off another person at this point
	bool dropOffAnotherPerson = false;

	do
	{
		//Get the passenger to be dropped off
		auto currItem = driverSchedule.getCurrScheduleItem();
		const string &passengerId = currItem->tripRequest.userId;
		auto itPassengers = passengers.find(passengerId);

#ifndef NDEBUG
		if (itPassengers == passengers.end())
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
		conflux->dropOffTraveller(person);

		//Remove passenger from vehicle
		passengers.erase(itPassengers);

		ControllerLog() << "Drop-off of user " << person->getDatabaseId() << " at time "
		                << parent->currTick << ", destinationNodeId " << conflux->getConfluxNode()->getNodeId()
		                << ", and driverId " << getParent()->getDatabaseId() << std::endl;

		//Mark schedule item as completed
		scheduleItemCompleted();

		//Check if the next schedule item is also a drop-off and at the same node
		//Note: This can happen when a controller that assigns shared requests is being used
		if(!driverSchedule.isScheduleCompleted())
		{
			//Note: We have marked the previous schedule item as completed, so the current schedule item
			//iterator has been updated
			auto nxtItem = driverSchedule.getCurrScheduleItem();

			dropOffAnotherPerson = nxtItem->scheduleItemType == DROPOFF
			                       && currItem->tripRequest.destinationNode == nxtItem->tripRequest.destinationNode;
		}
		else
		{
			dropOffAnotherPerson = false;
		}

	}while(dropOffAnotherPerson);
}

void OnCallDriver::endShift()
{
	auto currItem = driverSchedule.getCurrScheduleItem();

	//Check if we are in the middle of a schedule
	if(currItem->scheduleItemType != CRUISE && currItem->scheduleItemType != PARK)
	{
		return;
	}

	//Notify the controller(s)
	for(auto ctrlr : subscribedControllers)
	{
		MessageBus::PostMessage(ctrlr, MSG_DRIVER_SHIFT_END,
		                        MessageBus::MessagePtr(new DriverShiftCompleted(parent)));
	}

	isWaitingForUnsubscribeAck = true;

	ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
	                << parent->getDatabaseId() << ": Shift ended"  << endl;
}

