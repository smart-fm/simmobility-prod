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
        isExitingParking(false),passengerInteractedDropOff(0)
{
}

OnCallDriver::OnCallDriver(Person_MT *parent) : Driver(parent)
{
}

OnCallDriver::~OnCallDriver()
{
    //Check if this driver have some passenger
    if(!passengers.empty())
    {

        ControllerLog()<< "OnCallDriver " << parent->getDatabaseId() << " is being destroyed, but it has "
        << passengers.size() << " passengers("<<getPassengersId()<<"). So we just removing these passengers before destroying the Driver."<<endl;
        //We are just making these passengers as Null same as we have done in onHail evictPassenger
        for(auto itPax : passengers)
        {
            itPax.second = nullptr;
    }
        passengers.clear();
    }



    if(this->behaviour->hasDriverShiftEnded())
    {
        ControllerLog()<<"Driver "<<parent->getDatabaseId()<<" is being deleted because of shift end"<<endl;

    }

    else
    {
        ControllerLog() << "Driver " << parent->getDatabaseId() <<" destructor is called at time "<< parent->currTick <<
        " is being deleted BUT NO SHIFT END. it is because of some other reason. May be Path Not Found. Please check \"Warn.log\"..." << endl;
    }

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
        const Schedule & prevSheduleBeforeSet= driverSchedule.getSchedule();
        driverSchedule.setPrevSchedule(prevSheduleBeforeSet);
        const SchedulePropositionMessage &msg = MSG_CAST(SchedulePropositionMessage, message);
        Schedule schedule = msg.getSchedule();
        if (msg.getSchedule().front().tripRequest.requestType == RequestType::TRIP_REQUEST_SHARED)
        {
            setSharedSchedule(schedule);
        }
        else
        {
            driverSchedule.setSchedule(schedule);
        }
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
        const Schedule & prevSheduleBeforeSet= driverSchedule.getSchedule();
        driverSchedule.setPrevSchedule(prevSheduleBeforeSet);
        const SchedulePropositionMessage &msg = MSG_CAST(SchedulePropositionMessage, message);
        Schedule updatedSchedule = msg.getSchedule();

        auto controllerCopyError = checkForRepeatedPickups(updatedSchedule);
        setSharedSchedule(updatedSchedule, msg.GetSender(), true);

        if (controllerCopyError)
        {
            sendSyncMessage();
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

void OnCallDriver::setSharedSchedule(Schedule &schedule, MessageHandler *controller, const bool hasExistingSchedule)
{
    bool preemptCurrentItem = false;
    auto currItem = driverSchedule.getCurrScheduleItem();
    const Node *currNode = nullptr;

    if (hasExistingSchedule)
    {
        if (currentItemRescheduled(schedule))
        {
            preemptCurrentItem = true;
            currItem = schedule.begin();
        }
        currNode = currItem->getNode();
    }

    // Delete all schedule items in the front of the schedule that have the same node as current one
    for (auto itemIterator = schedule.begin(); itemIterator != schedule.end();)
    {
        auto node  = itemIterator->getNode();
        if (hasExistingSchedule && currItem->tripRequest != itemIterator->tripRequest && currNode == node)
        {
            sameNodeItems.insert(make_pair(*currItem, *itemIterator));
#ifndef NDEBUG
            ControllerLog() << getParent()->getDatabaseId() << " Inserting into sameNodeItems " << *itemIterator
                    << " Same as current item " << *currItem << endl;
#endif
            itemIterator = schedule.erase(itemIterator);
        }
        else
        {
            break;
        }
    }

    // Scan the schedule and delete all consecutive schedule items that share the same node.
    // Such items can be performed simultaneously, in a single frame tick. So, we need to
    // keep track of only one of them.
    auto node1 = schedule.begin()->getNode();
    for (auto itemIterator = schedule.begin() + 1; itemIterator != schedule.end();)
    {
        auto node2 = itemIterator->getNode();
        if (node1 == node2)
        {
            sameNodeItems.insert(make_pair(*(itemIterator - 1), *itemIterator));
#ifndef NDEBUG
            ControllerLog() << getParent()->getDatabaseId() << " Inserting into sameNodeItems " << *itemIterator
                    << "Same as previous item " << *(itemIterator - 1) << endl;
#endif
            itemIterator = schedule.erase(itemIterator);
        }
        else
        {
               itemIterator++;
               node1 = node2;
        }
    }

    if (!hasExistingSchedule)
    {
        driverSchedule.setSchedule(schedule);
    }
    else if (preemptCurrentItem)
    {
        driverSchedule.setSchedule(schedule);
        if (driverSchedule.isSameNodeItem(currNode))
        {
            // This happens when a schedule item is performed by the driver in some frame tick 't1'.
            // The controller updates the schedule during the same tick, 't1' and places a new item
            // at the head of the schedule. Additionally, the new item can be served at the same node
            // as the schedule item performed by the driver during 't1'. However, the communication
            // happens 1 frame tick later, causing an inconsistency in the controller and driver copies.

            SyncScheduleMsg *syncMsg = new SyncScheduleMsg(getParent(), schedule);
            MessageBus::PostMessage(controller, MSG_SYNC_SCHEDULE, MessageBus::MessagePtr(syncMsg));
        }
            movement->performScheduleItem();
    }
    else
    {
        driverSchedule.updateSchedule(schedule);
    }
}

const bool OnCallDriver::checkForRepeatedPickups(Schedule &schedule) const
{
    bool erased = false;
    for (auto itemIt = schedule.begin(); itemIt != schedule.end();)
    {
        if (itemIt->scheduleItemType == ScheduleItemType::PICKUP &&
                getPassengersId().find(itemIt->tripRequest.userId) != std::string::npos)
        {
            itemIt = schedule.erase(itemIt);
            erased = true;
        }
        else
        {
            itemIt++;
        }
    }

    return erased;
}

void OnCallDriver::immediatelyPerformItem()
{
    informController = false;

    switch(driverSchedule.getCurrScheduleItem()->scheduleItemType)
    {
    case ScheduleItemType::PICKUP:
    {
        if (getPassengersId().find(driverSchedule.getCurrScheduleItem()->tripRequest.userId) != std::string::npos)
        {
            // There is an error in the schedule. The driver is asked to pickup a passenger who is already
            // present in the vehicle. So we simply mark the current pickup item as completed and
            // start the next schedule item.
            driverSchedule.itemCompleted();
        }
        else
        {
#ifndef NDEBUG
        ControllerLog() << "Driver " << getParent()->getDatabaseId() << " can do next item pickup at same location." << std::endl;
#endif
            pickupPassenger();
        }

        break;
    }
    case ScheduleItemType::DROPOFF:
    {
        if (getPassengersId().find(driverSchedule.getCurrScheduleItem()->tripRequest.userId) == std::string::npos)
        {
            // There is an error in the schedule. The driver is asked to dropoff a passenger who isn't
            // present in the vehicle. So we simply mark the current dropoff item as completed and
            // start the next schedule item.
            driverSchedule.itemCompleted();
        }
        else
        {
#ifndef NDEBUG
        ControllerLog() << "Driver " << getParent()->getDatabaseId() << " can do next item dropoff at same location." << std::endl;
#endif
            dropoffPassenger();
        }
        break;
    }
    default:
    {
        stringstream msg;
        msg << "Schedule item in sameNodeItems for driver " << getParent()->getDatabaseId()
                << " is not pickup or dropoff" << std::endl;
        throw runtime_error(msg.str());
    }
    }
}

const Node* OnCallDriver::getCurrentNode() const
{
    return movement->getCurrentNode();
}

void  OnCallDriver::setCurrentNode(const Node* thisNode)
{
    movement->setCurrentNode(thisNode);
}

const unsigned OnCallDriver::getNumDropoffs(const ScheduleItem item) const
{
    unsigned dropoffs = 0;
    auto itemList = sameNodeItems.equal_range(item);
    for (auto itemIt = itemList.first; itemIt != itemList.second; itemIt++)
    {
        dropoffs += getNumDropoffs(itemIt->second);
    }

    if (item.scheduleItemType == ScheduleItemType::DROPOFF)
    {
        dropoffs++;
    }

    return dropoffs;
}

const unsigned OnCallDriver::getNumAssigned() const
{
    unsigned numDropoffs = 0;
    for (auto item : driverSchedule.getSchedule())
    {
        numDropoffs += getNumDropoffs(item);
    }

    return numDropoffs;
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

    if (informController)
    {
        if(behaviour->hasDriverShiftEnded() && driverSchedule.isScheduleCompleted())
        {
            //If the shift has ended, we no longer need to send the status message
            //and the available message. We simply wait for the shift end confirmation
            ControllerLog()<< "Driver "<<getParent()->getDatabaseId()<<"served total "<<passengerInteractedDropOff<<" persons from it's last available status."<<endl;
            passengerInteractedDropOff=0;
            return;
        }

        sendStatusMessage();

        if(driverSchedule.isScheduleCompleted())
        {
            ControllerLog()<< "Driver "<<getParent()->getDatabaseId()<<"served total "<<passengerInteractedDropOff<<" persons from it's last available status."<<endl;
            passengerInteractedDropOff=0;
            sendAvailableMessage();
        }
    }
    else
    {
        informController = true;
    }
}


const bool OnCallDriver::currentItemRescheduled(Schedule &updatedSchedule)
{
    auto currItem = driverSchedule.getCurrScheduleItem();
    auto schItr = updatedSchedule.begin();

    // If the current item is the first item in the new schedule as well, continue updating
    if (*currItem == *schItr)
    {
        updatedSchedule.erase(schItr);
        return false;
    }
    for (++schItr; schItr != updatedSchedule.end(); schItr++)
    {
        if(*currItem == *schItr)
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

void OnCallDriver::sendSyncMessage()
{
    ControllerLog() << "There is a mismatch between the driver and controller copies of schedule for driver "
            << getParent()->getDatabaseId() << ". Forcing controller to use driver schedule. "
            << driverSchedule.getSchedule() << " at time:" << getParent()->currTick << endl;

    Warn() << "Mismatch in schedule for driver " << getParent()->getDatabaseId() << " at time:"
    << getParent()->currTick << " Please look at the ControllerLog for more information." << endl;

    for(auto ctrlr : subscribedControllers)
    {
        SyncScheduleMsg *syncMsg = new SyncScheduleMsg(getParent(), driverSchedule.getSchedule());
        MessageBus::PostMessage(ctrlr, MSG_SYNC_SCHEDULE, MessageBus::MessagePtr(syncMsg));
    }
}

void OnCallDriver::sendWakeUpShiftEndMsg()
{
    unsigned int timeToShiftEnd = (parent->getServiceVehicle().endTime * 1000) - parent->currTick.ms();
    unsigned int tick = ConfigManager::GetInstance().FullConfig().baseGranMS();

    medium::Conflux *cflx = movement->getMesoPathMover().getCurrSegStats()->getParentConflux();
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
    setCurrentNode(conflux->getConfluxNode());
    passenger->setStartPoint(personPickedUp->currSubTrip->origin);
    passenger->setStartPointDriverDistance(movement->getTravelMetric().distance);
    passenger->setEndPoint(personPickedUp->currSubTrip->destination);
    passenger->Movement()->startTravelTimeMetric();
    passenger->resetSharingCount();

    for (auto &p : passengers)
    {
        p.second->setSharingCount(getPassengerCount());
    }

    ControllerLog() << "Pickup succeeded for " << passengerId << " at time " << parent->currTick
    << " with startNodeId " << conflux->getConfluxNode()->getNodeId()<<conflux->getConfluxNode()->printIfNodeIsInStudyArea()<<" and  destinationNodeId "
    << personPickedUp->currSubTrip->destination.node->getNodeId() <<personPickedUp->currSubTrip->destination.node->printIfNodeIsInStudyArea()<<", and driverId "
    << parent->getDatabaseId() << std::endl;

    auto itemList = sameNodeItems.equal_range(*driverSchedule.getCurrScheduleItem());

    //Mark schedule item as completed
    scheduleItemCompleted();

    for (auto itemIt = itemList.first; itemIt != itemList.second;)
    {
        auto schedule = driverSchedule.getSchedule();
        schedule.insert(schedule.begin(), itemIt->second);
        driverSchedule.setSchedule(schedule);
        itemIt = sameNodeItems.erase(itemIt);

        immediatelyPerformItem();
    }
}

void OnCallDriver::dropoffPassenger()
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
    medium::Conflux *conflux = segStats->getParentConflux();
    passengerToBeDroppedOff->setFinalPointDriverDistance(movement->getTravelMetric().distance);
    conflux->dropOffTraveller(person);
    const auto maxSharingCount = passengerToBeDroppedOff->getSharingCount();

    //Remove passenger from vehicle
    passengers.erase(itPassengers);
    ++passengerInteractedDropOff;
    setCurrentNode(conflux->getConfluxNode());
	ControllerLog() << "Drop-off of user " << person->getDatabaseId() << " at time "
	                << parent->currTick << ", destinationNodeId " << conflux->getConfluxNode()->getNodeId()
                    << conflux->getConfluxNode()->printIfNodeIsInStudyArea()
                    << "and driverId " << getParent()->getDatabaseId()
                    << ". This person shared the vehicle with a maximum of "
                    << maxSharingCount << " people." << std::endl;

    auto itemList = sameNodeItems.equal_range(*driverSchedule.getCurrScheduleItem());

    //Mark schedule item as completed
    scheduleItemCompleted();

    for (auto itemIt = itemList.first; itemIt != itemList.second;)
    {
        auto schedule = driverSchedule.getSchedule();
        schedule.insert(schedule.begin(), itemIt->second);
        driverSchedule.setSchedule(schedule);
        itemIt = sameNodeItems.erase(itemIt);

        immediatelyPerformItem();
    }
}


void OnCallDriver::endShift()
{
    if(getDriverStatus()!=PARKED)     //As for Parked Driver there would be no current schedule item
    {
        auto currItem = driverSchedule.getCurrScheduleItem();
        //Check if we are in the middle of a schedule
        if(currItem->scheduleItemType != CRUISE && currItem->scheduleItemType != PARK)
        {
            return;
        }
    }

    //Notify the controller(s)
    for(auto ctrlr : subscribedControllers)
    {
        MessageBus::PostMessage(ctrlr, MSG_DRIVER_SHIFT_END,
                                MessageBus::MessagePtr(new DriverShiftCompleted(parent)));
    }

    passengerInteractedDropOff=0;
    isWaitingForUnsubscribeAck = true;

    ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
                    << parent->getDatabaseId() << ": Shift ended"  << endl;
}

std::string OnCallDriver::getPassengersId() const
{
    std::string passengerID = "";
    if (passengers.empty())
    {
        passengerID = "No Passenger";
    }
    else
    {
        for (std::unordered_map<std::string, Passenger *>::const_iterator it = passengers.begin(); it != passengers.end(); ++it)
        {
            if (it != passengers.begin())
            {
                passengerID.append("|");
            }

            passengerID.append(it->first);
        }

    }
    return passengerID;
}

void OnCallDriver::collectTravelTime()
{
    //Do nothing, as we do not collect travel times for onCall drivers
}


void OnCallDriver::removeDriverEntryFromAllContainer()
{
    Person *thisDriver = this->getParent();
    const vector<MobilityServiceController *> & driverSubscribedToController  = getSubscribedControllers();
    for(auto it = driverSubscribedToController.begin(); it !=driverSubscribedToController.end(); ++it)
    {
        if(getDriverStatus()== MobilityServiceDriverStatus::DRIVE_START )
        {
            /* If Person is not yet started  & going to be removed Mean it's call from findStartingConflux is fail & This person will not Loaded to Simulation at all
             * Please note  for first tick for any person is generated by findStartingConflux ----> frame_init   NOT from updateAgent --->callMovementFrameInit-->frame_init.
             * So for this Driver (for DRIVE_START case) even subscription is filtered.
            */

            ControllerLog()<<" Driver "<<thisDriver->getDatabaseId()<<"have not started yet to move (Driver_STATUS = DRIVE_START)." \
                    " And going to be removed for such case Actually driver is not loaded at all and not going to subscribe" \
                    "So No point to Unsubscribe or Kill (delete) driver." << endl;


        }
        else if((*it)->getControllerCopyDriverSchedulesMap().find(thisDriver)== (*it)->getControllerCopyDriverSchedulesMap().end())
        {
            ControllerLog()<<" Driver "<<thisDriver->getDatabaseId()<<"have  started  to move (Driver_STATUS = CRUISE)." \
                    " Subscription is sent, But Subscription is not recieved yet. In such case we are removing driver's entry from availableDrivers list before deletion."<<endl;

            (*it)->getAvailableDriverSet().erase(thisDriver);
        }
        else
        {
            Schedule &thisDriverScheduleControllerCopy = (*it)->getControllerCopyDriverSchedulesMap().at(thisDriver);
            while (!thisDriverScheduleControllerCopy.empty())
            {
                thisDriverScheduleControllerCopy.pop_back();

            }
            (*it)->unsubscribeDriver(thisDriver);
        }
    }

}
