/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "GeneralPathMover.hpp"

#include <limits>
#include <stdexcept>

#include "geospatial/RoadSegment.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/Lane.hpp"
#include "util/GeomHelpers.hpp"

using namespace sim_mob;
using std::vector;



void sim_mob::GeneralPathMover::setPath(vector<WayPoint> wp_path)
{
	fullPath.clear();
	for(vector<WayPoint>::iterator it=wp_path.begin(); it!=wp_path.end(); it++) {
		if(it->type_ == WayPoint::ROAD_SEGMENT) {
			fullPath.push_back(it->roadSegment_);
		}
	}

	currSegmentIt = fullPath.begin();
	polypointsList = (*currSegmentIt)->getLanes().front()->getPolyline();
	currPolypoint = polypointsList.begin();
	nextPolypoint = polypointsList.begin()+1;
	distAlongPolyline = 0;
	currPolylineLength = sim_mob::dist(&(*currPolypoint), &(*nextPolypoint));
	inIntersection = false;
}


void sim_mob::GeneralPathMover::setPath(const vector<RoadSegment*>& path)
{
	fullPath.clear();
	for(vector<RoadSegment*>::const_iterator it=path.begin(); it!=path.end(); it++) {
		fullPath.push_back(*it);
	}

	currSegmentIt = fullPath.begin();
	polypointsList = (*currSegmentIt)->getLanes().front()->getPolyline();
	currPolypoint = polypointsList.begin();
	nextPolypoint = polypointsList.begin()+1;
	distAlongPolyline = 0;
	currPolylineLength = sim_mob::dist(&(*currPolypoint), &(*nextPolypoint));
	inIntersection = false;
}


bool sim_mob::GeneralPathMover::isPathSet() const
{
	return !fullPath.empty();
}

bool sim_mob::GeneralPathMover::isDoneWithEntireRoute() const
{
	return currSegmentIt==fullPath.end();
}

void sim_mob::GeneralPathMover::leaveIntersection()
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");

	//Unset flag; move to the next segment.
	inIntersection = false;
	currSegmentIt++;
}


//This is where it gets a little complex.
void sim_mob::GeneralPathMover::advance(centimeter_t fwdDistance)
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");

	//Taking precedence above everything else is the intersection model. If we are in an intersection,
	//  simply update the total distance and return (let the user deal with it). Also udpate the
	//  current polyline length to always be the same as the forward distance.
	if (inIntersection) {
		distAlongPolyline += fwdDistance;
		currPolylineLength = distAlongPolyline;
		return;
	}

	//Next, if we are truly at the end of the path, we should probably throw an error for trying to advance.
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");

	//Move down the current polyline. If this brings us to the end point, go to the next polyline
	distAlongPolyline += fwdDistance;
	while (distAlongPolyline>=currPolylineLength) {
		advanceToNextPolyline();
	}
}


void sim_mob::GeneralPathMover::advanceToNextPolyline()
{
	//An error if we're still at the end of this polyline
	throwIf(nextPolypoint==polypointsList.end(), "Polyline can't advance");

	//Subtract distance moved thus far
	distAlongPolyline -= currPolylineLength;

	//Advance pointers
	currPolypoint++;
	nextPolypoint++;

	//Update length, OR move to a new Segment
	if (nextPolypoint != polypointsList.end()) {
		currPolylineLength = sim_mob::dist(&(*currPolypoint), &(*nextPolypoint));
	} else {
		advanceToNextRoadSegment();
	}
}


void sim_mob::GeneralPathMover::advanceToNextRoadSegment()
{
	//An error if we're already at the end of this road segment
	throwIf(currSegmentIt==fullPath.end(), "Road segment can't advance");

	//If we are approaching a new Segment, the Intersection driving model takes precedence.
	// In addition, no further processing occurs. This means advancing a very large amount will
	// leave the user inside an intersection even if he/she would normally be beyond it.
	//Note that distAlongPolyline should still be valid.
	if ((*currSegmentIt)->getLink() != (*(currSegmentIt+1))->getLink()) {
		inIntersection = true;
		return;
	}

	//Move to the next Segment
	currSegmentIt++;
}


const RoadSegment* sim_mob::GeneralPathMover::getCurrSegment() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");
	return *currSegmentIt;
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

centimeter_t sim_mob::GeneralPathMover::getCurrDistAlongPolyline() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");

	//If we're done, returning zero makes sense
	if (isDoneWithEntireRoute()) {
		return 0;
	}

	//Limiting by the total distance makes sense.
	return std::min(distAlongPolyline, currPolylineLength);
}

centimeter_t sim_mob::GeneralPathMover::getCurrPolylineTotalDist() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");
	return currPolylineLength;
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






