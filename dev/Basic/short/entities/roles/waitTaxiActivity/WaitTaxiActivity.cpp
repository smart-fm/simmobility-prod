#include <vector>
#include "WaitTaxiActivity.hpp"
#include "entities/Person_ST.hpp"
#include "geospatial/network/TaxiStand.hpp"
#include "entities/PT_Statistics.hpp"
#include "message/ST_Message.hpp"
#include "WaitTaxiActivityFacets.hpp"

namespace sim_mob
{
WaitTaxiActivity::WaitTaxiActivity(Person_ST* parent,
        WaitTaxiActivityBehavior* behavior, WaitTaxiActivityMovement* movement,
        const TaxiStand* stand,
        std::string roleName, Role<Person_ST>::Type roleType) :
        sim_mob::Role<Person_ST>::Role(parent, behavior, movement, roleName,
                roleType), stand(stand), waitingTime(0)
{
}

    WaitTaxiActivity::~WaitTaxiActivity() {

}

sim_mob::Role<Person_ST>* WaitTaxiActivity::clone(Person_ST *parent) const
{
    SubTrip& subTrip = *(parent->currSubTrip);
    WaitTaxiActivityBehavior* behavior = new WaitTaxiActivityBehavior();
    WaitTaxiActivityMovement* movement = new WaitTaxiActivityMovement();
    WaitTaxiActivity* waitTaxiActivity = new WaitTaxiActivity(parent, behavior, movement, subTrip.origin.taxiStand);
    behavior->setWaitTaxiActivity(waitTaxiActivity);
    movement->setWaitTaxiActivity(waitTaxiActivity);
    return waitTaxiActivity;
}

void WaitTaxiActivity::make_frame_tick_params(timeslice now)
{
    getParams().reset(now);
}

void WaitTaxiActivity::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
}

std::vector<BufferedBase*> WaitTaxiActivity::getSubscriptionParams()
{
    return std::vector<BufferedBase*>();
}

void WaitTaxiActivity::collectTravelTime()
{
    PersonTravelTime personTravelTime;
    std::string personId, tripStartPoint, tripEndPoint, subStartPoint, subEndPoint, subStartType, subEndType, mode, service, arrivaltime, travelTime;
    personTravelTime.personId = parent->getDatabaseId();
    personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
    personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
    personTravelTime.subStartPoint = parent->currSubTrip->startLocationId;
    personTravelTime.subEndPoint = parent->currSubTrip->endLocationId;
    personTravelTime.subStartType = parent->currSubTrip->startLocationType;
    personTravelTime.subEndType = parent->currSubTrip->endLocationType;

    if((*(parent->currTripChainItem))->travelMode == "SMS")
    {
        personTravelTime.mode = "WAIT_SMS";
    }
    else
    {
        personTravelTime.mode = "WAIT_TAXI";
    }

    personTravelTime.travelTime = ((double) parent->getRole()->getTravelTime())/1000.0; //convert to seconds
    personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();
    messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
                    STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
}

void WaitTaxiActivity::increaseWaitingTime(unsigned int timeMs)
{
    waitingTime += timeMs;
}
unsigned int WaitTaxiActivity::getWaitingTime() const
{
    return waitingTime;
}
const TaxiStand* WaitTaxiActivity::getTaxiStand() const
{
    return stand;
}
}

