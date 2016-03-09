/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   EventsInjector.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on November 8, 2013, 1:32 PM
 */

#include "EventsInjector.hpp"
#include "model/lua/LuaProvider.hpp"
#include "message/MessageBus.hpp"
#include "event/LT_EventArgs.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "AgentsLookup.hpp"
#include "message/LT_Message.hpp"
#include <model/AwakeningSubModel.hpp>


using namespace sim_mob;
using namespace sim_mob::long_term;
using namespace sim_mob::event;
using namespace sim_mob::messaging;
using std::vector;

namespace
{
    /**
     * Converts given ExternalEvent::Type into EventId
     * @param externalType to match.
     * @return correspondent EventId or -1.
     */
    inline EventId toEventId(int externalType)
    {
        switch (externalType)
        {
            case ExternalEvent::LOST_JOB:
                return LTEID_EXT_LOST_JOB;
            case ExternalEvent::NEW_CHILD:
                return LTEID_EXT_NEW_CHILD;
            case ExternalEvent::NEW_JOB:
                return LTEID_EXT_NEW_JOB;
            case ExternalEvent::NEW_JOB_LOCATION:
                return LTEID_EXT_NEW_JOB_LOCATION;
            case ExternalEvent::NEW_SCHOOL_LOCATION:
                return LTEID_EXT_NEW_SCHOOL_LOCATION;
            case ExternalEvent::ZONING_RULE_CHANGE:
            	return LTEID_EXT_ZONING_RULE_CHANGE;
            default:
            	return -1;
        }
    }
}

EventsInjector::EventsInjector() : Entity(-1), model(nullptr){}

EventsInjector::~EventsInjector() {}

bool EventsInjector::isNonspatial()
{
    return false;
}

void EventsInjector::setModel(HM_Model *value)
{
	model = value;
}

HM_Model* EventsInjector::getModel()
{
	return model;
}

void EventsInjector::buildSubscriptionList(vector<BufferedBase*>& subsList) {}

void EventsInjector::onWorkerEnter() {}

void EventsInjector::onWorkerExit() {}

Entity::UpdateStatus EventsInjector::update(timeslice now)
{
    const ExternalEventsModel& model = LuaProvider::getExternalEventsModel();
    
    vector<ExternalEvent> events;
    AwakeningSubModel awakenings;
    events = awakenings.DailyAwakenings( now.ms(), getModel() );

    AgentsLookup& lookup = AgentsLookupSingleton::getInstance();
    const HouseholdAgent* householdAgent = nullptr;
    const DeveloperAgent* developerAgent = nullptr;
    const RealEstateAgent* realEstateAgent = nullptr;
    for ( vector<ExternalEvent>::iterator it = events.begin(); it != events.end(); ++it)
    {
    	developerAgent = lookup.getDeveloperAgentById(it->getDeveloperId());
    	if(developerAgent)
    	{
    		MessageBus::PublishEvent(toEventId(it->getType()), const_cast<DeveloperAgent*>(developerAgent), MessageBus::EventArgsPtr(new ExternalEventArgs(*it)));
    	}
    	else
    	{
			householdAgent = lookup.getHouseholdAgentById(it->getHouseholdId());
			if (householdAgent)
			{
				MessageBus::PublishEvent(toEventId(it->getType()), const_cast<HouseholdAgent*>(householdAgent), MessageBus::EventArgsPtr(new ExternalEventArgs(*it)));
			}
			else
			{
				realEstateAgent = lookup.getRealEstateAgentById(it->getHouseholdId());
				if (realEstateAgent)
				{
					MessageBus::PublishEvent(toEventId(it->getType()), const_cast<RealEstateAgent*>(realEstateAgent), MessageBus::EventArgsPtr(new ExternalEventArgs(*it)));
				}

			}
    	}
    }
    return Entity::UpdateStatus(Entity::UpdateStatus::RS_CONTINUE);
}
