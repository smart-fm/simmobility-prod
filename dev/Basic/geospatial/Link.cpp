/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Link.hpp"

#include <stdexcept>

#include "RoadSegment.hpp"

using namespace sim_mob;
using std::vector;
using std::string;


int sim_mob::Link::GetLength(bool isForward)
{
	vector<RoadSegment*> segments = GetPath(isForward);
	double totalLen = 0.0;
	for (vector<RoadSegment*>::iterator it=segments.begin(); it!=segments.end(); it++) {
		totalLen += (*it)->length;
	}
	return totalLen;
}


vector<RoadSegment*> sim_mob::Link::GetPath(bool isForward)
{
	//TODO: Currently un-implemented; we need a policy for bi-directional road segments.
	throw std::runtime_error("Not yet implemented.");
}

string sim_mob::Link::getSegmentName(const RoadSegment* segment)
{
	//TODO: Currently un-implemented; we need a naming policy.
	throw std::runtime_error("Not yet implemented.");
}


