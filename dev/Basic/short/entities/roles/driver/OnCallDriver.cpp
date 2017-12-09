#include "OnCallDriver.hpp"
#include "conf/ConfigManager.hpp"
#include "message/ST_Message.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "message/MessageHandler.hpp"
#include "driverCommunication/DriverCommFacets.hpp"

#include "message/MessageBus.hpp"
#include "message/MobilityServiceControllerMessage.hpp"

using namespace sim_mob;
using namespace messaging;
using namespace std;


OnCallDriver::OnCallDriver(Person_ST *parent, const MutexStrategy &mtx, OnCallDriverBehaviour *behaviour,
                           OnCallDriverMovement *movement, std::string roleName, Type roleType) :
        Driver(parent,mtx,behaviour,movement,roleType,roleName),movement(movement),behaviour(behaviour), isWaitingForUnsubscribeAck(false)
{

}

OnCallDriver::OnCallDriver(Person_ST *parent, const MutexStrategy &mtx) : Driver(parent,mtx)
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

Role<Person_ST>* OnCallDriver::clone(Person_ST *person) const
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

    driverMvt->init();

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
            movement->performScheduleItem();//implement the movement part later.
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
                movement->performScheduleItem();//implement the movement part later.
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
const MobilityServiceDriver* OnCallDriver::exportServiceDriver() const
{
    return this;
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

void OnCallDriver::scheduleItemCompleted()
{
    driverSchedule.itemCompleted();

    sendStatusMessage();

    if(driverSchedule.isScheduleCompleted())
    {
        sendAvailableMessage();
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
void OnCallDriver::endShift()
{
    ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
                    << parent->getDatabaseId() << ": Shift ended"  << endl;
}

void OnCallDriver::reload()
{
//revisit
    ControllerLog() << parent->currTick.ms() << "ms: OnCallDriver "
                   << ": Reloaded"  << endl;
}

void OnCallDriver::subscribeToOrIgnoreController(const SvcControllerMap& controllers, unsigned int controllerId)
{
    if (parent->getServiceVehicle().controllerSubscription & controllerId)
    {
        auto range = controllers.find(controllerId);

#ifndef NDEBUG
        if (range == controllers.end())
        {
            std::stringstream msg;
            msg << "OnCallDriver " << parent->getDatabaseId() << " wants to subscribe to id "
                << controllerId << ", but no controller of that id is registered";
            throw std::runtime_error(msg.str());
        }
#endif

        auto itController = range;
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


void OnCallDriver::sendWakeUpShiftEndMsg()
{
    unsigned int timeToShiftEnd = (parent->getServiceVehicle().endTime * 1000) - parent->currTick.ms();
    unsigned int tick = ConfigManager::GetInstance().FullConfig().baseGranMS();
    auto tripRequest = driverSchedule.getCurrScheduleItem()->tripRequest;
    MessageBus::PostMessage(tripRequest.GetSender(), MSG_WAKEUP_SHIFT_END, MessageBus::MessagePtr(new PersonMessage(parent)),
                            false, timeToShiftEnd / tick);
}
void OnCallDriver::collectTravelTime(Person_ST* person)
{
    if (person && person->getRole())
    {
        person->getRole()->collectTravelTime();
    }
}