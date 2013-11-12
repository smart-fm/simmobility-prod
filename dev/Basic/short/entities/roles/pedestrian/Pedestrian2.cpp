//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * Pedestrian2.cpp
 *
 * \author Max
 */

#include "Pedestrian2.hpp"
#include "entities/Person.hpp"
#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "geospatial/Node.hpp"
#include "logging/Log.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/BusStop.hpp"

#include "util/GeomHelpers.hpp"
#include "geospatial/Point2D.hpp"
#include "entities/signal/Signal.hpp"

using std::vector;
using namespace sim_mob;

namespace {

//For random number generating
boost::uniform_int<> zero_to_five(0, 5);
boost::uniform_int<> zero_to_max(0, RAND_MAX);

vector<const RoadSegment*> BuildUpPath(vector<RoadSegment*>::iterator curr, vector<RoadSegment*>::iterator end)
{
	vector<const RoadSegment*> res;
	for (; curr != end; curr++) {
		res.push_back(*curr);
	}
	return res;
}

vector<const RoadSegment*> ForceForwardSubpath(const RoadSegment* revSegment, vector<RoadSegment*> candList1, vector<
		RoadSegment*> candList2) {
	//First, iterate through each list until we find an item that is the REVERSE of our revSegment
	for (int i = 0; i < 2; i++) {
		vector<RoadSegment*>& cand = (i == 0) ? candList1 : candList2;
		for (vector<RoadSegment*>::iterator it = cand.begin(); it != cand.end(); it++) {
			//Negative: break early if we find the same segment.
			if ((*it)->getStart() == revSegment->getStart() && (*it)->getEnd() == revSegment->getEnd()) {
				break;
			}

			//Positive: return if we find the reverse segment
			if ((*it)->getStart() == revSegment->getEnd() && (*it)->getEnd() == revSegment->getStart()) {
				return BuildUpPath(it, cand.end());
			}
		}
	}

	//Error:
	throw std::runtime_error("Can't retrieve forward subpath for the given candidates.");
}

}

sim_mob::Pedestrian2::Pedestrian2(Agent* parent, sim_mob::Pedestrian2Behavior* behavior, sim_mob::Pedestrian2Movement* movement, Role::type roleType_, std::string roleName):
		Role(behavior, movement, parent, roleName, roleType_), params(parent->getGenerator()) {
//	//Check non-null parent. Perhaps references may be of use here?
//
//	//Init
//	sigColor = sim_mob::Green; //Green by default
//
//#if 0
//	sigColor = Signal::Green; //Green by default
//#endif
//
//	//Set default speed in the range of 1.2m/s to 1.6m/s
//	speed = 1.2;
//
//	xVel = 0;
//	yVel = 0;
//
//	xCollisionVector = 0;
//	yCollisionVector = 0;
}

//Note that a destructor is not technically needed, but I want to enforce the idea
//  of overriding virtual destructors if they exist.
sim_mob::Pedestrian2::~Pedestrian2() {
}

vector<BufferedBase*> sim_mob::Pedestrian2::getSubscriptionParams() {
	vector<BufferedBase*> res;
	return res;
}

Role* sim_mob::Pedestrian2::clone(Person* parent) const
{
	Pedestrian2Behavior* behavior = new Pedestrian2Behavior(parent);
	Pedestrian2Movement* movement = new Pedestrian2Movement(parent);
	Pedestrian2* pedestrian2 = new Pedestrian2(parent, behavior, movement);
	behavior->setParentPedestrian2(pedestrian2);
	movement->setParentPedestrian2(pedestrian2);
	return pedestrian2;
}

void sim_mob::Pedestrian2::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}
