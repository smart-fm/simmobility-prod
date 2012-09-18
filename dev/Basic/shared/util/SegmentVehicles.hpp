/* Copyright Singapore-MIT Alliance for Research and Technology */

//TODO: Add comments to each of the methods ~ Harish

#pragma once
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "entities/Agent.hpp"
#include <boost/unordered_set.hpp>

namespace sim_mob {
/**
 * Keeps a lane wise count of moving and queuing vehicles in a road segment.
 * Used by mid term supply
 */
class SegmentVehicles {

private:
	const sim_mob::RoadSegment* roadSegment;
	std::map<const sim_mob::Lane*, std::vector<sim_mob::Agent*> > queuingAgents;
	std::map<const sim_mob::Lane*, std::set<sim_mob::Agent*> > movingAgents;

public:
	SegmentVehicles(const sim_mob::RoadSegment* rdSeg);
	void addQueuingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void removeQueuingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	sim_mob::Agent* dequeue(const sim_mob::Lane* lane);
	bool isFront(const sim_mob::Lane* lane, sim_mob::Agent* agent);
	void addMovingAgent(const sim_mob::Lane* lane, sim_mob::Agent* ag);
	void removeMovingAgent(const sim_mob::Lane* lane, sim_mob::Agent*);
	std::set<Agent*> getAgentsOnMovingVehicles(const sim_mob::Lane* lane);
	std::vector<Agent*> getAgentsOnQueuingVehicles(const sim_mob::Lane* lane);
	const sim_mob::RoadSegment* getRoadSegment() const;
	void merge(sim_mob::SegmentVehicles*);
};

}
