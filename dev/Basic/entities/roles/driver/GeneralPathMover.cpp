/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "GeneralPathMover.hpp"

#include <limits>
#include <algorithm>
#include <stdexcept>

#include "geospatial/RoadSegment.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/Point2D.hpp"
#include "util/GeomHelpers.hpp"

using namespace sim_mob;
using std::vector;
using std::string;
using std::endl;


//Used for path debugging.
const bool sim_mob::GeneralPathMover::DebugOn = true;


sim_mob::GeneralPathMover::GeneralPathMover() : distAlongPolyline(0), /*currPolylineLength(0),*/
		distMovedInPrevSegments(0), distOfThisSegment(0), inIntersection(false), isMovingForwardsInLink(false), currLaneID(0)
{
}


void sim_mob::GeneralPathMover::setPath(const vector<const RoadSegment*>& path, bool firstSegMoveFwd, int startLaneID)
{
	if (DebugOn) {
		DebugStream <<"New Path of length " <<path.size() <<endl;
		DebugStream <<"Starting in Lane: " <<startLaneID <<endl;
	}

	//Add RoadSegments to the path.
	Link* currLink = nullptr;
	bool fwd = firstSegMoveFwd;
	fullPath.clear();
	for(vector<const RoadSegment*>::const_iterator it=path.begin(); it!=path.end(); it++) {
		fullPath.push_back(*it);

		if (DebugOn) {
			DebugStream <<"  " <<(*it)->getStart()->originalDB_ID.getLogItem() <<"=>" <<(*it)->getEnd()->originalDB_ID.getLogItem();
			if ((*it)->getLink()!=currLink) {
				currLink = (*it)->getLink();
				if (it!=path.begin()) {
					fwd = (*it)->getLink()->getStart() == (*it)->getStart();
				}
				DebugStream <<"  Link: " <<currLink <<" fwd: " <<(fwd?"true":"false") <<"  length: " <<Fmt_M(currLink->getLength(fwd)) <<"  poly-length: " <<Fmt_M(CalcSegmentLaneZeroDist(it, path.end()));
			}
			DebugStream <<endl;
			DebugStream <<"    Euclidean length: " <<Fmt_M(dist((*it)->getStart()->location, (*it)->getEnd()->location)) <<"   reported length: " <<Fmt_M((*it)->length) <<endl;
		}
	}

	//Re-generate the polylines array, etc.
	currSegmentIt = fullPath.begin();
	isMovingForwardsInLink = firstSegMoveFwd;
	currLaneID = startLaneID;
	generateNewPolylineArray();
	distAlongPolyline = 0;
	inIntersection = false;
	calcNewLaneDistances();
}

void sim_mob::GeneralPathMover::calcNewLaneDistances()
{
	distMovedInPrevSegments = 0;
	distOfThisSegment = CalcSegmentLaneZeroDist(currSegmentIt, fullPath.end());
}

string sim_mob::GeneralPathMover::Fmt_M(centimeter_t dist)
{
	std::stringstream res;
	res <<static_cast<int>(dist/100) <<" m";
	return res.str();
}

double sim_mob::GeneralPathMover::CalcSegmentLaneZeroDist(vector<const RoadSegment*>::const_iterator start, vector<const RoadSegment*>::const_iterator end)
{
	double res = 0.0;
	for (vector<const RoadSegment*>::const_iterator it=start;it!=end;it++) {
		//Add all polylines in this Segment
		const vector<Point2D>& polyLine = const_cast<RoadSegment*>(*it)->getLaneEdgePolyline(0);
		for (vector<Point2D>::const_iterator it2=polyLine.begin(); (it2+1)!=polyLine.end(); it2++) {
			res += dist(it2->getX(), it2->getY(), (it2+1)->getX(), (it2+1)->getY());
		}

		//Break if the next Segment isn't in this link.
		if ((it+1==end) || ((*it)->getLink() != (*(it+1))->getLink()))  {
			break;
		}
	}
	return res;
}

void sim_mob::GeneralPathMover::generateNewPolylineArray()
{
	//Simple; just make sure to take the forward direction into account.
	//TODO: Take the current lane into account.
	polypointsList = (*currSegmentIt)->getLanes().at(currLaneID)->getPolyline();

	//Check
	throwIf(polypointsList.size()<2, "Can't manage polylines of length 0/1");

	/*if (!isMovingForwards) { //NOTE: I don't think this makes sense.
		std::reverse(polypointsList.begin(), polypointsList.end());
	}*/
	currPolypoint = polypointsList.begin();
	nextPolypoint = polypointsList.begin()+1;
	//currPolylineLength = sim_mob::dist(&(*currPolypoint), &(*nextPolypoint));

	//Set our lane zero polypoint-ers.
	const vector<Point2D>& tempLaneZero = const_cast<RoadSegment*>(*currSegmentIt)->getLaneEdgePolyline(0);
	currLaneZeroPolypoint = tempLaneZero.begin();
	nextLaneZeroPolypoint = tempLaneZero.end()+1;

	//Debug output
	if (DebugOn) {
		DebugStream <<"On new polyline (1 of " <<polypointsList.size()-1 <<") of length: " <<Fmt_M(currPolylineLength()) <<"  length of lane zero: " <<Fmt_M(dist(&(*nextLaneZeroPolypoint), &(*currLaneZeroPolypoint))) <<endl;
	}
}

bool sim_mob::GeneralPathMover::isPathSet() const
{
	return !fullPath.empty();
}

bool sim_mob::GeneralPathMover::isDoneWithEntireRoute() const
{
	bool res = currSegmentIt==fullPath.end();

	if (DebugOn && res) {
		if (!DebugStream.str().empty()) {
			DebugStream <<"Path is DONE." <<endl;
			std::cout <<DebugStream.str();
			DebugStream.str("");
		}
	}

	return res;
}

const Lane* sim_mob::GeneralPathMover::leaveIntersection()
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(!inIntersection, "Not actually in an Intersection!");

	//Unset flag; move to the next segment.
	inIntersection = false;
	return actualMoveToNextSegmentAndUpdateDir();
}

/*bool sim_mob::GeneralPathMover::isMovingForwardsOnCurrSegment() const
{
	return isMovingForwards;
}*/


//This is where it gets a little complex.
double sim_mob::GeneralPathMover::advance(double fwdDistance)
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");

	//Taking precedence above everything else is the intersection model. If we are in an intersection,
	//  simply update the total distance and return (let the user deal with it). Also udpate the
	//  current polyline length to always be the same as the forward distance.
	if (inIntersection) {
		throw std::runtime_error("Calling \"advance\" within an Intersection currently doesn't work right; use the Intersection model.");

		distAlongPolyline += fwdDistance;
		//currPolylineLength = distAlongPolyline;
		return 0;
	}

	//Debug output
	if (DebugOn) {
		DebugStream <<"  +" <<fwdDistance <<"cm" <<", (" <<Fmt_M(distAlongPolyline) <<")";
	}

	//Next, if we are truly at the end of the path, we should probably throw an error for trying to advance.
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");

	//Move down the current polyline. If this brings us to the end point, go to the next polyline
	double res = 0.0;
	distAlongPolyline += fwdDistance;
	//distMovedInSegment += fwdDistance;
	while (distAlongPolyline>=currPolylineLength() && !inIntersection) {
		if (DebugOn) { DebugStream <<endl; }

		//Subtract distance moved thus far
		distAlongPolyline -= currPolylineLength();

		//Advance pointers, etc.
		res = advanceToNextPolyline();
	}

	return res;
}


double sim_mob::GeneralPathMover::advanceToNextPolyline()
{
	//An error if we're still at the end of this polyline
	throwIf(nextPolypoint==polypointsList.end(), "Polyline can't advance");

	//Advance pointers
	currPolypoint++;
	nextPolypoint++;

	//Advance lane zero pointers
	currLaneZeroPolypoint++;
	nextLaneZeroPolypoint++;

	//Update length, OR move to a new Segment
	if (nextPolypoint == polypointsList.end()) {
		return advanceToNextRoadSegment();
	} else {
		if (DebugOn) {
			DebugStream <<"On new polyline (" <<(currPolypoint-polypointsList.begin()+1) <<" of " <<polypointsList.size()-1 <<") of length: " <<Fmt_M(currPolylineLength()) <<"  length of lane zero: " <<Fmt_M(dist(&(*nextLaneZeroPolypoint), &(*currLaneZeroPolypoint))) <<endl;
		}
	}

	return 0;
}


double sim_mob::GeneralPathMover::advanceToNextRoadSegment()
{
	//An error if we're already at the end of this road segment
	throwIf(currSegmentIt==fullPath.end(), "Road segment at end");
	//distMovedInSegment = distAlongPolyline;

	//We can safely update our total distance here.
	DynamicVector zeroPoly(currLaneZeroPolypoint->getX(), currLaneZeroPolypoint->getY(), nextLaneZeroPolypoint->getX(), nextLaneZeroPolypoint->getY());
	distMovedInPrevSegments += zeroPoly.getMagnitude();

	//If we are approaching a new Segment, the Intersection driving model takes precedence.
	// In addition, no further processing occurs. This means advancing a very large amount will
	// leave the user inside an intersection even if he/she would normally be beyond it.
	//Note that distAlongPolyline should still be valid.
	if (currSegmentIt+1!=fullPath.end()) {
		if ((*currSegmentIt)->getLink() != (*(currSegmentIt+1))->getLink()) {
			//Return early; we can't actually move the car now.
			inIntersection = true;
			return distAlongPolyline;
		}
	}

	//Move to the next Segment
	actualMoveToNextSegmentAndUpdateDir();
	return 0;
}

const Lane* sim_mob::GeneralPathMover::actualMoveToNextSegmentAndUpdateDir()
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");

	//Record
	bool nextInNewLink = ((currSegmentIt+1)!=fullPath.end()) && ((*(currSegmentIt+1))->getLink() != (*currSegmentIt)->getLink());

	//Move
	currSegmentIt++;

	//In case we moved
	//distAlongPolyline = distMovedInSegment; //NOTE: Should probably factor this out into a sep. variable.

	//Done?
	if (currSegmentIt==fullPath.end()) {
		return nullptr;
	}

	//Bound lanes
	currLaneID = std::min<int>(currLaneID, (*currSegmentIt)->getLanes().size()-1);

	//Is this new segment part of a Link we're traversing in reverse?
	//const Node* prevNode = isMovingForwards ? (*(currSegmentIt-1))->getEnd() : (*(currSegmentIt-1))->getStart();
	if (nextInNewLink) {
		calcNewLaneDistances();

		const Node* prevNode = (*currSegmentIt)->getStart(); //TEMP: Not sure about this.
		if ((*currSegmentIt)->getLink()->getStart() == prevNode) {
			isMovingForwardsInLink = true;
		} else if ((*currSegmentIt)->getLink()->getEnd() == prevNode) {
			isMovingForwardsInLink = false;
		} else {
			//Presumably, we could enable something like this later, but it would require advanced
			//  knowledge of which Segments face forwards.
			throw std::runtime_error("Can't jump around to arbitrary nodes with GeneralPathMover.");
		}
	}

	//Now generate a new polyline array.
	generateNewPolylineArray();

	//Done
	return (*currSegmentIt)->getLanes().at(currLaneID);
}

double sim_mob::GeneralPathMover::getCurrLinkLength() const
{
	return getCurrLink()->getLength(isMovingForwardsInLink);
}

const RoadSegment* sim_mob::GeneralPathMover::getCurrSegment() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");
	return *currSegmentIt;
}
const RoadSegment* sim_mob::GeneralPathMover::getNextSegment(bool sameLink) const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");

	vector<const RoadSegment*>::iterator nextSegmentIt = currSegmentIt+1;
	if (nextSegmentIt==fullPath.end()) {
		return nullptr;
	}
	if (((*nextSegmentIt)->getLink()!=(*currSegmentIt)->getLink()) && sameLink) {
		return nullptr;
	}

	return *nextSegmentIt;
}
const RoadSegment* sim_mob::GeneralPathMover::getPrevSegment(bool sameLink) const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done."); //Not technically an error, but unlikely to be useful.

	if (currSegmentIt==fullPath.begin()) {
		return nullptr;
	}

	vector<const RoadSegment*>::iterator nextSegmentIt = currSegmentIt-1;
	if (((*nextSegmentIt)->getLink()!=(*currSegmentIt)->getLink()) && sameLink) {
		return nullptr;
	}

	return *nextSegmentIt;
}
const Link* sim_mob::GeneralPathMover::getCurrLink() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");
	return getCurrSegment()->getLink();
}
const Point2D& sim_mob::GeneralPathMover::getCurrPolypoint() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done."); //Not technically wrong, but probably an error.
	return *currPolypoint;
}
const Point2D& sim_mob::GeneralPathMover::getNextPolypoint() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");
	return *nextPolypoint;
}

double sim_mob::GeneralPathMover::getCurrDistAlongPolyline() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");

	//If we're done, returning zero makes sense
	if (isDoneWithEntireRoute()) {
		return 0;
	}

	//Limiting by the total distance makes sense.
	return std::min(distAlongPolyline, currPolylineLength());
}

double sim_mob::GeneralPathMover::getCurrDistAlongRoadSegment() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isInIntersection(), "Can't get distance in Segment while in an intersection.");

	//Get the current median polyline distance
	DynamicVector zeroPoly(currLaneZeroPolypoint->getX(), currLaneZeroPolypoint->getY(), nextLaneZeroPolypoint->getX(), nextLaneZeroPolypoint->getY());
	double totalPolyDist = zeroPoly.getMagnitude();

	//Get the ratio of distance moved over the current one.
	double distRatio = distAlongPolyline / currPolylineLength();

	//Add this to the distance moved so far.
	return distMovedInPrevSegments + distRatio*totalPolyDist;
}

double sim_mob::GeneralPathMover::getTotalRoadSegmentLength() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isInIntersection(), "Can't get distance of Segment while in an intersection.");

	return distOfThisSegment;
}

double sim_mob::GeneralPathMover::getCurrPolylineTotalDist() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");
	return currPolylineLength();
}

void sim_mob::GeneralPathMover::shiftToNewPolyline(bool moveLeft)
{
	moveToNewPolyline(currLaneID + (moveLeft?1:-1));
}

void sim_mob::GeneralPathMover::moveToNewPolyline(int newLaneID)
{
	//Nothing to do?
	if (newLaneID<0 || newLaneID>=static_cast<int>((*currSegmentIt)->getLanes().size())) {
		return;
	}


	//Save our progress
	int distTraveled = currPolypoint - polypointsList.begin();

	//Update our polyline array
	generateNewPolylineArray();
	if (distTraveled>0) {
		currPolypoint += distTraveled;
		nextPolypoint += distTraveled;

		//And our lane zero pointers
		currLaneZeroPolypoint += distTraveled;
		nextLaneZeroPolypoint += distTraveled;
		//currPolylineLength = sim_mob::dist(&(*currPolypoint), &(*nextPolypoint));
	}

	//TODO: This is kind of a hack, but it's possible to have been on a shorter polyline than the
	//      one you just switched to. Might want to look at this later.
	advance(0);
}


DPoint sim_mob::GeneralPathMover::getPosition() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");

	//If we're done, return the position of the last poly-point
	if (isDoneWithEntireRoute()) {
		return DPoint(currPolypoint->getX(),currPolypoint->getY());
	}

	//Else, scale a vector like normal
	DynamicVector movementVect(currPolypoint->getX(),currPolypoint->getY(), nextPolypoint->getX(), nextPolypoint->getY());
	movementVect.scaleVectTo(getCurrDistAlongPolyline()).translateVect();
	return DPoint(movementVect.getX(), movementVect.getY());
}






