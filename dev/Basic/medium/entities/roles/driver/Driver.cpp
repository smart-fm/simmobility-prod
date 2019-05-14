//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Driver.hpp"

#include <cmath>
#include <ostream>
#include <algorithm>
#include "DriverFacets.hpp"
#include "entities/Person.hpp"
#include "entities/UpdateParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/conflux/Conflux.hpp"
#include "entities/AuraManager.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"

#include "buffering/BufferedDataManager.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/Point.hpp"

#include "logging/Log.hpp"
#include "util/DebugFlags.hpp"

#include "partitions/PartitionManager.hpp"
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#include "entities/Person_MT.hpp"
#include "entities/PT_Statistics.hpp"
#include "entities/roles/activityRole/ActivityFacets.hpp"

#include "config/MT_Config.hpp"
using namespace sim_mob;
using namespace sim_mob::medium;

using std::max;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::endl;

//Helper functions
namespace {
//TODO:I think lane index should be a data member in the lane class
size_t getLaneIndex(const Lane* l) {
    if (l) {
        const RoadSegment* r = l->getParentSegment();
        for (size_t i = 0; i < r->getLanes().size(); i++) {
            if (r->getLanes().at(i) == l) {
                return i;
            }
        }
    }
    return -1; //NOTE: This might not do what you expect! ~Seth
}
} //end of anonymous namespace

//Initialize
sim_mob::medium::Driver::Driver(Person_MT* parent,
        sim_mob::medium::DriverBehavior* behavior,
        sim_mob::medium::DriverMovement* movement,
        std::string roleName, Role<Person_MT>::Type roleType) :
    sim_mob::Role<Person_MT>::Role(parent, behavior, movement, roleName, roleType),
    currLane(nullptr)
{}

sim_mob::medium::Driver::~Driver() {}

vector<BufferedBase*> sim_mob::medium::Driver::getSubscriptionParams() {
    return vector<BufferedBase*>();
}

void sim_mob::medium::Driver::make_frame_tick_params(timeslice now)
{
    getParams().reset(now);
}

Role<Person_MT>* sim_mob::medium::Driver::clone(Person_MT* parent) const
{
    DriverBehavior* behavior = new DriverBehavior();
    DriverMovement* movement = new DriverMovement();
    Driver* driver = new Driver(parent, behavior, movement, "Driver_");
    if (MT_Config::getInstance().isEnergyModelEnabled())
    {
        PersonParams personInfo;
//		if (MT_Config::getInstance().getEnergyModel()->getModelType() == "tripenergy")
//		{
//			personInfo.getVehicleParams().setVehicleStruct(MT_Config::getInstance().getEnergyModel()->initVehicleStruct("Bus"));
//		}
//		else
        if (MT_Config::getInstance().getEnergyModel()->getModelType() == "simple") //jo
        {
            personInfo.getVehicleParams().setDrivetrain("ICE"); //jo
            personInfo.getVehicleParams().setVehicleStruct(MT_Config::getInstance().getEnergyModel()->initVehicleStruct("ICE"));
        }
        driver->parent->setPersonInfo(personInfo);
    }

    behavior->setParentDriver(driver);
    movement->setParentDriver(driver);
    return driver;
}

void sim_mob::medium::DriverUpdateParams::reset(timeslice now)
{
    UpdateParams::reset(now);

    secondsInTick = ConfigManager::GetInstance().FullConfig().baseGranSecond();
    elapsedSeconds = 0.0;
}

void sim_mob::medium::Driver::HandleParentMessage(messaging::Message::MessageType type, const messaging::Message& message)
{
    switch (type)
    {
    case MSG_INSERT_INCIDENT:
    {
        Movement()->handleMessage(type, message);
        break;
    }
    }
}

bool sim_mob::medium::Driver::canSheMove() const{ return true;}

/**
 * collect travel time for current role
 */
void sim_mob::medium::Driver::collectTravelTime()
{
    PersonTravelTime personTravelTime;
    personTravelTime.personId = parent->getDatabaseId();
    if(parent->getPrevRole() && parent->getPrevRole()->roleType==Role<Person_MT>::RL_ACTIVITY)
    {
        ActivityPerformer<Person_MT>* activity = dynamic_cast<ActivityPerformer<Person_MT>* >(parent->getPrevRole());
        std::string activityLocNodeIdStr = boost::lexical_cast<std::string>(activity->getLocation()->getNodeId());
        personTravelTime.tripStartPoint = activityLocNodeIdStr;
        personTravelTime.tripEndPoint = activityLocNodeIdStr;
        personTravelTime.subStartPoint = activityLocNodeIdStr;
        personTravelTime.subEndPoint = activityLocNodeIdStr;
        personTravelTime.subStartType = "N";
        personTravelTime.subEndType = "N";
        personTravelTime.mode = "ACTIVITY";
        personTravelTime.service = parent->currSubTrip->ptLineId;
        personTravelTime.travelTime = ((double) activity->getTravelTime())/1000.0;
        personTravelTime.arrivalTime = DailyTime(activity->getArrivalTime()).getStrRepr();
        messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
                                           STORE_PERSON_TRAVEL_TIME, messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)), true);
    }

    personTravelTime.tripStartPoint = (*(parent->currTripChainItem))->startLocationId;
    personTravelTime.tripEndPoint = (*(parent->currTripChainItem))->endLocationId;
    personTravelTime.subStartPoint = personTravelTime.tripStartPoint;
    personTravelTime.subEndPoint = personTravelTime.tripEndPoint;
    personTravelTime.subStartType = "NODE";
    personTravelTime.subEndType = "NODE";
    personTravelTime.mode = (*(parent->currTripChainItem))->getMode();
    personTravelTime.service = "";
    DailyTime TravelTime = DailyTime(
            this->getParams().now.ms()) + ConfigManager::GetInstance().FullConfig().simStartTime()
                           - DailyTime(parent->getRole()->getArrivalTime());
    personTravelTime.travelTime = (double) TravelTime.getValue() / 1000;
    personTravelTime.arrivalTime = DailyTime(parent->getRole()->getArrivalTime()).getStrRepr();

    if (roleType == Role<Person_MT>::RL_DRIVER)
    {
        if (personTravelTime.mode == "Taxi")
        {
            personTravelTime.mode = "ON_TAXI";
        }
        else
        {
            personTravelTime.mode = "ON_CAR";
        }
    }

    messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
                                       STORE_PERSON_TRAVEL_TIME,
                                       messaging::MessageBus::MessagePtr(new PersonTravelTimeMessage(personTravelTime)),
                                       true);
}
