//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <stdexcept>
#include "ActivityPerformer.hpp"
#include "conf/ConfigManager.hpp"
#include "entities/roles/RoleFacets.hpp"
#include "entities/UpdateParams.hpp"
#include "geospatial/MultiNode.hpp"
#include "entities/Person.hpp"

namespace sim_mob
{

template<class PERSON> class ActivityPerformer;

template<class PERSON>
class ActivityPerformerBehavior : public sim_mob::BehaviorFacet
{
public:
	explicit ActivityPerformerBehavior() : BehaviorFacet()
	{
	}
	virtual ~ActivityPerformerBehavior()
	{
	}

	//Virtual overrides
	virtual void frame_init()
	{
		throw std::runtime_error("ActivityPerformerBehavior::frame_init() is not implemented yet");
	}
	virtual void frame_tick()
	{
		throw std::runtime_error("ActivityPerformerBehavior::frame_tick() is not implemented yet");
	}
	virtual void frame_tick_output()
	{
		throw std::runtime_error("ActivityPerformerBehavior::frame_tick_output() is not implemented yet");
	}
};

template<class PERSON>
class ActivityPerformerMovement : public sim_mob::MovementFacet
{
public:
	explicit ActivityPerformerMovement() : MovementFacet(), parentActivity(NULL)
	{
	}

	virtual ~ActivityPerformerMovement()
	{
		/*if(travelMetric.started)
		{
			finalizeTravelTimeMetric();
		}*/
	}

	//Virtual overrides
	virtual void frame_init()
	{
		parentActivity->initializeRemainingTime();
		parentActivity->setTravelTime(parentActivity->getRemainingTimeToComplete());
		//startTravelTimeMetric();
	}

	virtual void frame_tick()
	{
		parentActivity->updateRemainingTime();
		if(parentActivity->getRemainingTimeToComplete() <= 0)
		{
			parentActivity->parent->setToBeRemoved();
		}
		parentActivity->parent->setRemainingTimeThisTick(0.0);
	}

	virtual void frame_tick_output()
	{
	}

	virtual sim_mob::Conflux* getStartingConflux() const
	{
		const sim_mob::MultiNode* activityLocation = dynamic_cast<sim_mob::MultiNode*>(parentActivity->getLocation());
		if(activityLocation) //activity locations must ideally be multinodes
		{
			return ConfigManager::GetInstanceRW().FullConfig().getConfluxForNode(activityLocation);
		}
		return nullptr;
	}

	/// mark startTimeand origin
	virtual TravelMetric& startTravelTimeMetric()
	{
		//travelMetric.started = true;
		return  travelMetric;
	}

	///	mark the destination and end time and travel time
	virtual TravelMetric& finalizeTravelTimeMetric()
	{
		//parent->serializeCBD_Activity(travelMetric);
		//travelMetric.finalized = true;
		return  travelMetric;
	}

private:
	sim_mob::ActivityPerformer<PERSON>* parentActivity;
	friend class ActivityPerformer<PERSON>;
};
}
