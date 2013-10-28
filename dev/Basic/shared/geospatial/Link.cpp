//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Link.hpp"

#include "conf/settings/DisableMPI.h"

#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "RoadSegment.hpp"
#include "Lane.hpp"
#include "util/GeomHelpers.hpp"
#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "util/GeomHelpers.hpp"
#include "workers/Worker.hpp"
#endif

using namespace sim_mob;
using std::vector;
using std::set;
using std::string;


namespace {

RoadSegment* findSegment(const set<RoadSegment*>& segments, const Node* const startsAt, const Node* const prevNode) {
	RoadSegment* res = nullptr;
	for (set<RoadSegment*>::const_iterator it=segments.begin(); it!=segments.end(); it++) {
		//Simple case.
		if ((*it)->getStart()==startsAt) {
			res = *it;
		}

		//Special case for bidirectional roads
		if ((*it)->isBiDirectional() && (*it)->getEnd()==startsAt) {
			res = *it;
		}

		//Quality control; are we going back the way we came?
		if (res) {
			if (res->getStart()!=prevNode && res->getEnd()!=prevNode) {
				return res;
			}
			res = nullptr; //Keep searching.
		}
	}

	//Failure
	return res;
}


bool buildLinkList(const set<RoadSegment*>& segments, vector<RoadSegment*>& res, set<RoadSegment*>& usedSegments,
		const Node* start, const Node* end)
{
	const Node* prev = nullptr;
	for (const Node* fwd=start; fwd!=end;) {
		//Retrieve the next segment
		RoadSegment* nextSeg = findSegment(segments, fwd, prev);
		if (!nextSeg) {
			return false;
		}

		//Add it, track it, increment
		res.push_back(nextSeg);
		usedSegments.insert(nextSeg);
		prev = fwd;
		if (fwd!=nextSeg->getEnd()) {
			fwd = nextSeg->getEnd();
		} else {
			fwd = nextSeg->getStart();
		}
	}
	return true;
}


} //End anon namespace



void sim_mob::Link::initializeLinkSegments(const std::set<sim_mob::RoadSegment*>& newSegs)
{
	//We build in two directions; forward and backwards. We also maintain a list of which
	// road segments are used, to catch cases where RoadSegments are skipped.
	set<RoadSegment*> usedSegments;
	bool res1 = buildLinkList(newSegs, this->segs, usedSegments, start, end);
	//bool res2 = buildLinkList(segments, revSegments, usedSegments, end, start);

	//Ensure we have at least ONE path (for one-way Links)
	if (!res1) {
		throw std::runtime_error("Incomplete link; missing RoadSegment.");
	}

	//Double-check that everything's been read at least once.
	if (usedSegments.size() < newSegs.size()) {
		std::stringstream msg;
		msg <<"Link constructed without the use of all its segments: " <<usedSegments.size() <<" of " <<newSegs.size()
			<<"  segments are: ";
		for (std::set<sim_mob::RoadSegment*>::const_iterator it=newSegs.begin(); it!=newSegs.end(); it++) {
			msg <<(*it)->originalDB_ID.getLogItem() <<"  ";
		}
		throw std::runtime_error(msg.str().c_str());
	}

	//Save all segments
	uniqueSegments.insert(newSegs.begin(), newSegs.end());
}




int sim_mob::Link::getLength() const
{
	vector<RoadSegment*> segments = getSegments();
	int totalLen = 0;
	for (vector<RoadSegment*>::iterator it=segments.begin(); it!=segments.end(); it++) {
		totalLen += (*it)->length;
	}
	return totalLen;
}

const unsigned int & sim_mob::Link::getLinkId() const
{
	return linkID;
}
const std::string & sim_mob::Link::getRoadName() const
{
	return roadName;
}


const vector<RoadSegment*>& sim_mob::Link::getSegments() const
{
	return segs;
}

vector<RoadSegment*>& sim_mob::Link::getSegments()
{
	return segs;
}

string sim_mob::Link::getSegmentName(const RoadSegment* segment)
{
	//Return something like RoadName-10F, which means it's the 10th item in a forward-directional segment.
	// Could also be RoadName-10F-9R for bi-directional.
	std::stringstream res;
	res <<roadName;

	vector<RoadSegment*>::iterator it = std::find(segs.begin(), segs.end(), segment);
	if (it!=segs.end()) {
		res <<"-" <<(it-segs.begin()+1);
	}

	return res.str();
}

const std::set<sim_mob::RoadSegment*> & sim_mob::Link::getUniqueSegments()
{
	return uniqueSegments;
}

void sim_mob::Link::extendPolylinesBetweenRoadSegments()
{
	extendPolylinesBetweenRoadSegments(segs);
	//extendPolylinesBetweenRoadSegments(revSegments);
}

void sim_mob::Link::extendPolylinesBetweenRoadSegments(std::vector<RoadSegment*>& segments)
{

	if(segments.size()<=1)
	{
		return;
	}
	for(int i=0;i<segments.size()-1;i++)
	{
		RoadSegment* seg1 = segments.at(i);
		RoadSegment* seg2 = segments.at(i+1);
		size_t j=0;
		for(;j<seg1->getLanes().size();j++)
		{
			seg1->getLanes()[j]->getPolyline();
		}
		for(j=0;j<seg2->getLanes().size();j++)
			seg2->getLanes()[j]->getPolyline();
		for(j=0;j<seg1->getLanes().size();j++)
		{
			if(j>=seg2->getLanes().size())
				break;
			Lane* preLane = seg1->getLanes().at(j);
			Lane* nextLane = seg2->getLanes().at(j);
			const std::vector<sim_mob::Point2D>& prePolyline = preLane->getPolyline();
			const std::vector<sim_mob::Point2D>& nextPolyline = nextLane->getPolyline();
			size_t size1 = prePolyline.size();
			size_t size2 = nextPolyline.size();
//			Point2D newPoint = LineLineIntersect(prePolyline.at(size1-2),prePolyline.at(size1-1),
//					nextPolyline.at(0),nextPolyline.at(1));
			//use middle point between the end point of the previous lane and start point of the next lane
			int newX = prePolyline.at(size1-1).getX()/2 + nextPolyline.at(0).getX()/2;
			int newY = prePolyline.at(size1-1).getY()/2 + nextPolyline.at(0).getY()/2;
			Point2D newPoint(newX,newY);
			int dx = prePolyline.at(size1-1).getX() - newX;
			int dy = prePolyline.at(size1-1).getY() - newY;
			//two points are very close, don't insert new point
			int dis = sqrt(dx*dx + dy*dy);
			if(dis>=20)
				preLane->insertNewPolylinePoint(newPoint, true);
			dx = nextPolyline.at(0).getX() - newX;
			dy = nextPolyline.at(0).getY() - newY;
			dis = sqrt(dx*dx + dy*dy);
			if(dis>=20)
				nextLane->insertNewPolylinePoint(newPoint, false);
		}
	}
}

sim_mob::Worker* sim_mob::Link::getCurrWorker() const
{
	return currWorker;
}

void sim_mob::Link::setCurrWorker(sim_mob::Worker* w)
{
	currWorker = w;
}

