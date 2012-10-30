/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "Link.hpp"

#include <stdexcept>
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



void sim_mob::Link::initializeLinkSegments(const std::set<sim_mob::RoadSegment*>& segments)
{
	//We build in two directions; forward and backwards. We also maintain a list of which
	// road segments are used, to catch cases where RoadSegments are skipped.
	set<RoadSegment*> usedSegments;
	bool res1 = buildLinkList(segments, fwdSegments, usedSegments, start, end);
	bool res2 = buildLinkList(segments, revSegments, usedSegments, end, start);

	//Ensure we have at least ONE path (for one-way Links)
	if (!res1 && !res2) {
		throw std::runtime_error("Incomplete link; missing RoadSegment.");
	}

	//Double-check that everything's been read at least once.
	if (usedSegments.size() < segments.size()) {
		throw std::runtime_error("Link constructed without the use of all its segments.");
	}

	//Save all segments
	uniqueSegments.insert(segments.begin(), segments.end());
}




int sim_mob::Link::getLength(bool isForward) const
{
	vector<RoadSegment*> segments = getPath(isForward);
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


const vector<RoadSegment*>& sim_mob::Link::getPath(bool isForward) const
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

const std::set<sim_mob::RoadSegment*> & sim_mob::Link::getUniqueSegments()
{
	return uniqueSegments;
}
const std::vector<sim_mob::RoadSegment*> & sim_mob::Link::getFwdSegments()
{
	return fwdSegments;
}
const std::vector<sim_mob::RoadSegment*> & sim_mob::Link::getRevSegments(){
	return revSegments;
}

void sim_mob::Link::extendPolylinesBetweenRoadSegments()
{
	extendPolylinesBetweenRoadSegments(fwdSegments);
	extendPolylinesBetweenRoadSegments(revSegments);
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
//		std::cout << "\nloop 0"; //getchar();
		size_t j=0;
		for(;j<seg1->getLanes().size();j++)
		{
			seg1->getLanes()[j]->getPolyline();
//			std::cout << "\nended getPolyline"; //getchar();
		}
//		std::cout << "\nloop 00"; //getchar();
		for(j=0;j<seg2->getLanes().size();j++)
			seg2->getLanes()[j]->getPolyline();
//		std::cout << " 000\n";
		for(j=0;j<seg1->getLanes().size();j++)
		{
			if(j>=seg2->getLanes().size())
				break;
			Lane* preLane = seg1->getLanes().at(j);
			Lane* nextLane = seg2->getLanes().at(j);
//			std::cout << " 2";
			const std::vector<sim_mob::Point2D>& prePolyline = preLane->getPolyline();
			const std::vector<sim_mob::Point2D>& nextPolyline = nextLane->getPolyline();
			size_t size1 = prePolyline.size();
			size_t size2 = nextPolyline.size();
//			std::cout << " 3";
//			Point2D newPoint = LineLineIntersect(prePolyline.at(size1-2),prePolyline.at(size1-1),
//					nextPolyline.at(0),nextPolyline.at(1));
			//use middle point between the end point of the previous lane and start point of the next lane
			int newX = prePolyline.at(size1-1).getX()/2 + nextPolyline.at(0).getX()/2;
			int newY = prePolyline.at(size1-1).getY()/2 + nextPolyline.at(0).getY()/2;
			Point2D newPoint(newX,newY);
//			std::cout << " 4";
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
//			std::cout << " 5\n";
		}
//		std::cout << " \n";
	}
	std::cout << " \n";
}

sim_mob::Worker* sim_mob::Link::getCurrWorker(){
		return currWorker;
	}
void sim_mob::Link::setCurrWorker(sim_mob::Worker* w){
		currWorker = w;
	}

#ifndef SIMMOB_DISABLE_MPI

//TODO: I think this merged incorrectly. ~Seth
//void sim_mob::Link::pack(PackageUtils& package,const Link* one_link) {}

#endif

