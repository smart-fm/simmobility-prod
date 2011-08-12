/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Link.hpp"

#include <stdexcept>
#include <algorithm>

#include "RoadSegment.hpp"

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;


namespace {

RoadSegment* findSegment(const set<RoadSegment*>& segments, const Node* startsAt) {
	for (set<RoadSegment*>::const_iterator it=segments.begin(); it!=segments.end(); it++) {
		//Simple case.
		if ((*it)->getStart()==startsAt) {
			return *it;
		}

		//Special case for bidirectional roads
		if ((*it)->isBiDirectional() && (*it)->getEnd()==startsAt) {
			return *it;
		}
	}
	return nullptr;
}


void buildLinkList(const set<RoadSegment*>& segments, vector<RoadSegment*> res, set<RoadSegment*> usedSegments, const Node* start, const Node* end) {
	for (const Node* fwd=start; fwd!=end;) {
		//Retrieve the next segment
		RoadSegment* nextSeg = findSegment(segments, fwd);
		if (!nextSeg) {
			throw std::runtime_error("Incomplete link; missing RoadSegment.");
		}

		//Add it, track it, increment
		res.push_back(nextSeg);
		usedSegments.insert(nextSeg);
		if (fwd!=nextSeg->getEnd()) {
			fwd = nextSeg->getEnd();
		} else {
			fwd = nextSeg->getStart();
		}
	}
}


} //End anon namespace



void sim_mob::Link::initializeLinkSegments(const std::set<sim_mob::RoadSegment*>& segments)
{
	//We build in two directions; forward and backwards. We also maintain a list of which
	// road segments are used, to catch cases where RoadSegments are skipped.
	set<RoadSegment*> usedSegments;
	buildLinkList(segments, fwdSegments, usedSegments, start, end);
	buildLinkList(segments, revSegments, usedSegments, end, start);

	//Double-check that everything's been read at least once.
	if (usedSegments.size() < segments.size()) {
		throw std::runtime_error("Link constructed without the use of all its segments.");
	}
}




int sim_mob::Link::GetLength(bool isForward) const
{
	vector<RoadSegment*> segments = GetPath(isForward);
	double totalLen = 0.0;
	for (vector<RoadSegment*>::iterator it=segments.begin(); it!=segments.end(); it++) {
		totalLen += (*it)->length;
	}
	return totalLen;
}


const vector<RoadSegment*>& sim_mob::Link::GetPath(bool isForward) const
{
	if (isForward) {
		return fwdSegments;
	} else {
		return revSegments;
	}
}

string sim_mob::Link::getSegmentName(const RoadSegment* segment)
{
	//Return something like RoadName-10F, which means it's the 10th item in a forward-directional segment.
	// Could also be RoadName-10F-9R for bi-directional.
	std::stringstream res;
	res <<roadName;

	vector<RoadSegment*>::iterator it = std::find(fwdSegments.begin(), fwdSegments.end(), segment);
	if (it!=fwdSegments.end()) {
		res <<"-" <<(it-fwdSegments.begin()+1) <<"F";
	}
	it = std::find(revSegments.begin(), revSegments.end(), segment);
	if (it!=revSegments.end()) {
		res <<"-" <<(it-revSegments.begin()+1) <<"R";
	}

	return res.str();
}


