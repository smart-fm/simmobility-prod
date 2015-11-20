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
#include "geospatial/network/Node.hpp"
#include "logging/Log.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/LaneConnector.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/network/Point.hpp"
#include "entities/signal/Signal.hpp"
#include "entities/Person_ST.hpp"

using std::vector;
using namespace sim_mob;

namespace
{

//For random number generating
boost::uniform_int<> zero_to_five(0, 5);
boost::uniform_int<> zero_to_max(0, RAND_MAX);

vector<const RoadSegment*> BuildUpPath(vector<RoadSegment*>::iterator curr, vector<RoadSegment*>::iterator end)
{
	vector<const RoadSegment*> res;
	for (; curr != end; curr++)
	{
		res.push_back(*curr);
	}
	return res;
}

vector<const RoadSegment*> ForceForwardSubpath(const RoadSegment* revSegment, vector<RoadSegment*> candList1, vector<
											   RoadSegment*> candList2)
{
	/*
	//First, iterate through each list until we find an item that is the REVERSE of our revSegment
	for (int i = 0; i < 2; i++)
	{
		vector<RoadSegment*>& cand = (i == 0) ? candList1 : candList2;
		for (vector<RoadSegment*>::iterator it = cand.begin(); it != cand.end(); it++)
		{
			//Negative: break early if we find the same segment.
			if ((*it)->getStart() == revSegment->getStart() && (*it)->getEnd() == revSegment->getEnd())
			{
				break;
			}

			//Positive: return if we find the reverse segment
			if ((*it)->getStart() == revSegment->getEnd() && (*it)->getEnd() == revSegment->getStart())
			{
				return BuildUpPath(it, cand.end());
			}
		}
	}
	 */
	//Error:
	throw std::runtime_error("Can't retrieve forward subpath for the given candidates.");
}

}

Pedestrian2::Pedestrian2(Person_ST *parent, Pedestrian2Behavior *behavior, Pedestrian2Movement *movement, Role<Person_ST>::Type roleType_, std::string roleName) 
: Role<Person_ST>(parent, behavior, movement, roleName, roleType_), params(parent->getGenerator())
{
	//	//Check non-null parent. Perhaps references may be of use here?
	//
	//	//Init
	//	sigColor = Green; //Green by default
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

Pedestrian2::~Pedestrian2()
{
}

vector<BufferedBase*> Pedestrian2::getSubscriptionParams()
{
	vector<BufferedBase*> res;
	return res;
}

Role<Person_ST>* Pedestrian2::clone(Person_ST *parent) const
{
	Pedestrian2Behavior* behavior = new Pedestrian2Behavior();
	Pedestrian2Movement* movement = new Pedestrian2Movement();
	Pedestrian2* pedestrian2 = new Pedestrian2(parent, behavior, movement);
	
	behavior->setParentPedestrian2(pedestrian2);
	movement->setParentPedestrian2(pedestrian2);
	
	return pedestrian2;
}

void Pedestrian2::make_frame_tick_params(timeslice now)
{
	getParams().reset(now);
}
