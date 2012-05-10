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
#include "util/DebugFlags.hpp"

#include "partitions/PartitionManager.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "partitions/ParitionDebugOutput.hpp"
#endif

using namespace sim_mob;
using std::vector;
using std::string;
using std::endl;

sim_mob::GeneralPathMover::GeneralPathMover() :
	distAlongPolyline(0), /*currPolylineLength(0),*/
	distMovedInCurrSegment(0), distOfThisSegment(0), distOfRestSegments(0), inIntersection(false), isMovingForwardsInLink(false), currLaneID(0)
{
}

sim_mob::GeneralPathMover::GeneralPathMover(const GeneralPathMover& copyFrom) :
	fullPath(copyFrom.fullPath), polypointsList(copyFrom.polypointsList), distAlongPolyline(copyFrom.distAlongPolyline), distMovedInCurrSegment(copyFrom.distMovedInCurrSegment),
			distOfThisSegment(copyFrom.distOfThisSegment), distOfRestSegments(copyFrom.distOfRestSegments), inIntersection(copyFrom.inIntersection),
			isMovingForwardsInLink(copyFrom.isMovingForwardsInLink), currLaneID(copyFrom.currLaneID)

{
	//We don't really care about the debug stream, but we should probably inform the user
	if (Debug::Paths)
	{
		DebugStream << "<COPY_CONSTRUCTOR>";
	}

	//Need to align our iterators
	currSegmentIt = fullPath.begin() + (copyFrom.currSegmentIt - copyFrom.fullPath.begin());
	currPolypoint = polypointsList.begin() + (copyFrom.currPolypoint - copyFrom.polypointsList.begin());
	nextPolypoint = polypointsList.begin() + (copyFrom.nextPolypoint - copyFrom.polypointsList.begin());

	//Lane-zero iterators are a little trickier
	const vector<Point2D>& myLaneZero = const_cast<RoadSegment*> (*currSegmentIt)->getLaneEdgePolyline(0);
	const vector<Point2D>& otherLaneZero = const_cast<RoadSegment*> (*copyFrom.currSegmentIt)->getLaneEdgePolyline(0);
	currLaneZeroPolypoint = myLaneZero.begin() + (copyFrom.currLaneZeroPolypoint - otherLaneZero.begin());
	nextLaneZeroPolypoint = myLaneZero.begin() + (copyFrom.nextLaneZeroPolypoint - otherLaneZero.begin());
}

void sim_mob::GeneralPathMover::setPath(const vector<const RoadSegment*>& path, int startLaneID)
{
	if (Debug::Paths)
	{
		DebugStream << "New Path of length " << path.size() << endl;
		DebugStream << "Starting in Lane: " << startLaneID << endl;
	}

	//Determine whether or not the first one is fwd.
	bool isFwd;
	if (path.empty())
	{
		throw std::runtime_error("Attempting to set a path with 0 road segments");
	}

	//We know we are moving forward if the last Node in the last Segment in the first Link is
	//   the "end" node of that Link.
	//The inverse applies to the first segment.
	const RoadSegment* firstSeg = path.front();
	const RoadSegment* lastSeg = path.front();
	for (vector<const RoadSegment*>::const_iterator it = path.begin() + 1; it != path.end(); it++)
	{
		if ((*it)->getLink() != lastSeg->getLink())
		{
			break;
		}
		lastSeg = *it;
	}

	//First check: end
	if (lastSeg->getEnd() == lastSeg->getLink()->getEnd())
	{
		isFwd = true;
	}
	else if (lastSeg->getEnd() == lastSeg->getLink()->getStart())
	{
		isFwd = false;

		//Send check: start
	}
	else if (firstSeg->getStart() == firstSeg->getLink()->getStart())
	{
		isFwd = true;
	}
	else if (firstSeg->getStart() == firstSeg->getLink()->getEnd())
	{
		isFwd = false;

		//Failure:
	}
	else
	{
		std::stringstream msg;
		msg << "Attempting to set a path with segments that aren't in order:\n";
		for (vector<const RoadSegment*>::const_iterator it = path.begin(); it != path.end(); it++)
		{
			msg << "  " << (*it)->getStart()->originalDB_ID.getLogItem() << " => " << (*it)->getEnd()->originalDB_ID.getLogItem() << "\n";
		}

		throw std::runtime_error(msg.str().c_str());
	}

	//Add RoadSegments to the path.
	Link* currLink = nullptr;
	bool fwd = isFwd;
	fullPath.clear();
	for (vector<const RoadSegment*>::const_iterator it = path.begin(); it != path.end(); it++)
	{
		fullPath.push_back(*it);

		if (Debug::Paths)
		{
			DebugStream << "  " << (*it)->getStart()->originalDB_ID.getLogItem() << "=>" << (*it)->getEnd()->originalDB_ID.getLogItem();
			if ((*it)->getLink() != currLink)
			{
				currLink = (*it)->getLink();
				if (it != path.begin())
				{
					fwd = (*it)->getLink()->getStart() == (*it)->getStart();
				}
				DebugStream << "  Link: " << currLink << " fwd: " << (fwd ? "true" : "false") << "  length: " << Fmt_M(currLink->getLength(fwd)) << "  poly-length: " << Fmt_M(
						CalcSegmentLaneZeroDist(it, path.end()));
			}
			DebugStream << endl;
			DebugStream << "    Euclidean length: " << Fmt_M(dist((*it)->getStart()->location, (*it)->getEnd()->location)) << "   reported length: " << Fmt_M((*it)->length) << endl;
		}
	}

	//Re-generate the polylines array, etc.
	currSegmentIt = fullPath.begin();
	isMovingForwardsInLink = isFwd;
	currLaneID = startLaneID;
	generateNewPolylineArray();
	distAlongPolyline = 0;
	inIntersection = false;
	//calcNewLaneDistances();

	distMovedInCurrSegment = 0;
	distOfThisSegment = CalcSegmentLaneZeroDist(currSegmentIt, fullPath.end());
	distOfRestSegments = CalcRestSegmentsLaneZeroDist(currSegmentIt, fullPath.end());

}

void sim_mob::GeneralPathMover::resetPath(const vector<const RoadSegment*>& path)
{

	//Determine whether or not the first one is fwd.
	if (path.empty())
	{
		throw std::runtime_error("Attempting to reset a path with 0 road segments");
	}
	//Add RoadSegments to the path.
	Link* currLink = nullptr;
	fullPath.clear();
	for (vector<const RoadSegment*>::const_iterator it = path.begin(); it != path.end(); it++)
	{
		fullPath.push_back(*it);
	}
	//Re-generate the polylines array, etc.
	currSegmentIt = fullPath.begin();
}

void sim_mob::GeneralPathMover::calcNewLaneDistances()
{
	DynamicVector zeroPoly(currLaneZeroPolypoint->getX(), currLaneZeroPolypoint->getY(), nextLaneZeroPolypoint->getX(), nextLaneZeroPolypoint->getY());
	distMovedInCurrSegment += zeroPoly.getMagnitude();
}

string sim_mob::GeneralPathMover::Fmt_M(centimeter_t dist)
{
	std::stringstream res;
	res << static_cast<int> (dist / 100) << " m";
	return res.str();
}

double sim_mob::GeneralPathMover::CalcSegmentLaneZeroDist(vector<const RoadSegment*>::const_iterator it, vector<const RoadSegment*>::const_iterator end)
{
	double res = 0.0;
	if (it != end)
	{
		const vector<Point2D>& polyLine = const_cast<RoadSegment*> (*it)->getLaneEdgePolyline(0);
		for (vector<Point2D>::const_iterator it2 = polyLine.begin(); (it2 + 1) != polyLine.end(); it2++)
		{
			res += dist(it2->getX(), it2->getY(), (it2 + 1)->getX(), (it2 + 1)->getY());
		}
	}
	return res;
}

double sim_mob::GeneralPathMover::CalcRestSegmentsLaneZeroDist(vector<const RoadSegment*>::const_iterator start, vector<const RoadSegment*>::const_iterator end)
{
	double res = 0.0;
	for (vector<const RoadSegment*>::const_iterator it = start; it != end; it++)
	{
		//Add all polylines in this Segment
		const vector<Point2D>& polyLine = const_cast<RoadSegment*> (*it)->getLaneEdgePolyline(0);
		for (vector<Point2D>::const_iterator it2 = polyLine.begin(); (it2 + 1) != polyLine.end(); it2++)
		{
			res += dist(it2->getX(), it2->getY(), (it2 + 1)->getX(), (it2 + 1)->getY());
		}

		//Break if the next Segment isn't in this link.
		if ((it + 1 == end) || ((*it)->getLink() != (*(it + 1))->getLink()))
		{
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
	throwIf(polypointsList.size() < 2, "Can't manage polylines of length 0/1");

	/*if (!isMovingForwards) { //NOTE: I don't think this makes sense.
	 std::reverse(polypointsList.begin(), polypointsList.end());
	 }*/
	currPolypoint = polypointsList.begin();
	nextPolypoint = polypointsList.begin() + 1;
	//currPolylineLength = sim_mob::dist(&(*currPolypoint), &(*nextPolypoint));

	//Set our lane zero polypoint-ers.
	const vector<Point2D>& tempLaneZero = const_cast<RoadSegment*> (*currSegmentIt)->getLaneEdgePolyline(0);
	currLaneZeroPolypoint = tempLaneZero.begin();
	nextLaneZeroPolypoint = tempLaneZero.begin() + 1;

	//Debug output
	if (Debug::Paths)
	{
		DebugStream << "On new polyline (1 of " << polypointsList.size() - 1 << ") of length: " << Fmt_M(currPolylineLength()) << "  length of lane zero: " << Fmt_M(
				dist(*nextLaneZeroPolypoint, *currLaneZeroPolypoint)) << ", starting at: " << Fmt_M(distAlongPolyline) << ", starting at: " << Fmt_M(distAlongPolyline) << endl;
		DebugStream << "Distance of polyline end from end of Road Segment: " << Fmt_M(dist(polypointsList.back(), (*currSegmentIt)->getEnd()->location)) << endl;
	}
}

bool sim_mob::GeneralPathMover::isPathSet() const
{
	return !fullPath.empty();
}

bool sim_mob::GeneralPathMover::isDoneWithEntireRoute() const
{
	bool res = currSegmentIt == fullPath.end();

	if (Debug::Paths && res)
	{
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		if (!DebugStream.str().empty())
		{
			//TEMP: Re-enable later.
			DebugStream << "Path is DONE." << endl;
			std::cout << DebugStream.str();
			DebugStream.str("");
		}
#endif
	}

	return res;
}

const Lane* sim_mob::GeneralPathMover::leaveIntersection()
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(!inIntersection, "Not actually in an Intersection!");

	if (Debug::Paths)
	{
		DebugStream << "User left intersection." << endl;
	}

	//Unset flag; move to the next segment.
	inIntersection = false;
	return actualMoveToNextSegmentAndUpdateDir();
}

void sim_mob::GeneralPathMover::throwIf(bool conditional, const std::string& msg) const
{
	if (conditional)
	{
		//Debug
		if (Debug::Paths)
		{
#ifndef SIMMOB_DISABLE_OUTPUT
			boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
			if (!DebugStream.str().empty())
			{
				DebugStream << "EXCEPTION: " << msg << endl;
				std::cout << DebugStream.str();
				DebugStream.str("");
			}
#endif
		}

		throw std::runtime_error(msg.c_str());
	}
}

//This is where it gets a little complex.
double sim_mob::GeneralPathMover::advance(double fwdDistance)
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");

	//Taking precedence above everything else is the intersection model. If we are in an intersection,
	//  simply update the total distance and return (let the user deal with it). Also udpate the
	//  current polyline length to always be the same as the forward distance.
	if (inIntersection)
	{
		throw std::runtime_error("Calling \"advance\" within an Intersection currently doesn't work right; use the Intersection model.");

		distAlongPolyline += fwdDistance;
		//currPolylineLength = distAlongPolyline;
		return 0;
	}

	//Debug output
	if (Debug::Paths)
	{
		//DebugStream <<"  +" <<fwdDistance <<"cm" <<", (" <<Fmt_M(distAlongPolyline+fwdDistance) <<")";
		//Print the distance from the next Node
		Point2D myPos(getPosition().x, getPosition().y);
		DebugStream << "  " << Fmt_M(dist(myPos, (*currSegmentIt)->getEnd()->location)) << ",";
	}

	//Next, if we are truly at the end of the path, we should probably throw an error for trying to advance.
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");

	//Move down the current polyline. If this brings us to the end point, go to the next polyline
	double res = 0.0;
	distAlongPolyline += fwdDistance;
	//distMovedInSegment += fwdDistance;

	while (distAlongPolyline >= currPolylineLength() && !inIntersection)
	{
		if (Debug::Paths)
		{
			Point2D myPos(getPosition().x, getPosition().y);
			DebugStream << endl << "Now at polyline end. Distance from actual end: " << Fmt_M(dist(*nextPolypoint, myPos)) << endl;
		}

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
	throwIf(nextPolypoint == polypointsList.end(), "Polyline can't advance");

	//We can safely update our total distance here.
	calcNewLaneDistances();

	//Advance pointers

	currPolypoint++;
	nextPolypoint++;

	//Advance lane zero pointers
	currLaneZeroPolypoint++;
	nextLaneZeroPolypoint++;

	//Update length, OR move to a new Segment
	if (nextPolypoint == polypointsList.end())
	{
		if (Debug::Paths)
		{
			DebugStream << "On new Road Segment" << endl;
		}

		return advanceToNextRoadSegment();
	}
	else
	{
		if (Debug::Paths)
		{
			DebugStream << "On new polyline (" << (currPolypoint - polypointsList.begin() + 1) << " of " << polypointsList.size() - 1 << ") of length: " << Fmt_M(currPolylineLength())
					<< "  length of lane zero: " << Fmt_M(dist(*nextLaneZeroPolypoint, *currLaneZeroPolypoint)) << ", starting at: " << Fmt_M(distAlongPolyline) << endl;
		}
	}

	return 0;
}

double sim_mob::GeneralPathMover::advanceToNextRoadSegment()
{
	//std::cout << "AAA" << std::endl;
	//An error if we're already at the end of this road segment
	throwIf(currSegmentIt == fullPath.end(), "Road segment at end");
	//distMovedInSegment = distAlongPolyline;

	//If we are approaching a new Segment, the Intersection driving model takes precedence.
	// In addition, no further processing occurs. This means advancing a very large amount will
	// leave the user inside an intersection even if he/she would normally be beyond it.
	//Note that distAlongPolyline should still be valid.
	if (currSegmentIt + 1 != fullPath.end())
	{
		if ((*currSegmentIt)->getLink() != (*(currSegmentIt + 1))->getLink())
		{
			Point2D myPos(getPosition().x, getPosition().y);
			if (Debug::Paths)
			{
				DebugStream << "Now in Intersection. Distance from Node center: " << Fmt_M(dist((*currSegmentIt)->getEnd()->location, myPos)) << endl;
			}

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
	bool nextInNewLink = ((currSegmentIt + 1) != fullPath.end()) && ((*(currSegmentIt + 1))->getLink() != (*currSegmentIt)->getLink());

	//Move
	currSegmentIt++;

	//Reset our distance; calculate the total lane-zero distance of this segment.
	distMovedInCurrSegment = 0;
	distOfThisSegment = CalcSegmentLaneZeroDist(currSegmentIt, fullPath.end());
	distOfRestSegments = CalcRestSegmentsLaneZeroDist(currSegmentIt, fullPath.end());

	//In case we moved
	//distAlongPolyline = distMovedInSegment; //NOTE: Should probably factor this out into a sep. variable.

	if (currSegmentIt == fullPath.end())
	{
		return nullptr;
	}

	//Bound lanes
	currLaneID = std::min<int>(currLaneID, (*currSegmentIt)->getLanes().size() - 1);

	//Is this new segment part of a Link we're traversing in reverse?
	//const Node* prevNode = isMovingForwards ? (*(currSegmentIt-1))->getEnd() : (*(currSegmentIt-1))->getStart();
	if (nextInNewLink)
	{
		//calcNewLaneDistances();

		const Node* prevNode = (*currSegmentIt)->getStart(); //TEMP: Not sure about this.
		if ((*currSegmentIt)->getLink()->getStart() == prevNode)
		{
			isMovingForwardsInLink = true;
		}
		else if ((*currSegmentIt)->getLink()->getEnd() == prevNode)
		{
			isMovingForwardsInLink = false;
		}
		else
		{
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

double sim_mob::GeneralPathMover::getCurrLinkReportedLength() const
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

	vector<const RoadSegment*>::iterator nextSegmentIt = currSegmentIt + 1;
	if (nextSegmentIt == fullPath.end())
	{
		return nullptr;
	}
	if (((*nextSegmentIt)->getLink() != (*currSegmentIt)->getLink()) && sameLink)
	{
		return nullptr;
	}

	return *nextSegmentIt;
}
const RoadSegment* sim_mob::GeneralPathMover::getPrevSegment(bool sameLink) const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done."); //Not technically an error, but unlikely to be useful.

	if (currSegmentIt == fullPath.begin())
	{
		return nullptr;
	}

	vector<const RoadSegment*>::iterator nextSegmentIt = currSegmentIt - 1;
	if (((*nextSegmentIt)->getLink() != (*currSegmentIt)->getLink()) && sameLink)
	{
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

const Lane* sim_mob::GeneralPathMover::getCurrLane() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	if (isDoneWithEntireRoute())
		return nullptr;
	return getCurrSegment()->getLanes().at(currLaneID);
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
	if (isDoneWithEntireRoute())
	{
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
	return distMovedInCurrSegment + distRatio * totalPolyDist;
}

double sim_mob::GeneralPathMover::getTotalRoadSegmentLength() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isInIntersection(), "Can't get distance of Segment while in an intersection.");

	return distOfThisSegment;
}

double sim_mob::GeneralPathMover::getAllRestRoadSegmentsLength() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isInIntersection(), "Can't get distance of Segment while in an intersection.");

	return distOfRestSegments;
}

double sim_mob::GeneralPathMover::getCurrPolylineTotalDist() const
{
	throwIf(!isPathSet(), "GeneralPathMover path not set.");
	throwIf(isDoneWithEntireRoute(), "Entire path is already done.");
	return currPolylineLength();
}

void sim_mob::GeneralPathMover::shiftToNewPolyline(bool moveLeft)
{

	moveToNewPolyline(currLaneID + (moveLeft ? 1 : -1));
}

void sim_mob::GeneralPathMover::moveToNewPolyline(int newLaneID)
{

	//Nothing to do?
	if (newLaneID == currLaneID)
	{
#ifndef SIMMOB_DISABLE_OUTPUT
		boost::mutex::scoped_lock local_lock(sim_mob::Logger::global_mutex);
		std::cout << "Nothing to do for next" << std::endl;
#endif
		return;
	}

	if (Debug::Paths)
	{
		DebugStream << "Switching to new lane: " << newLaneID << " from lane: " << currLaneID << endl;
	}

	//Invalid ID?
	if (newLaneID < 0 || newLaneID >= static_cast<int> ((*currSegmentIt)->getLanes().size()))
	{
		throw std::runtime_error("Switching to an invalid lane ID.");
	}

	//Save our progress
	int distTraveled = currPolypoint - polypointsList.begin();
	currLaneID = newLaneID;

	//Update our polyline array
	generateNewPolylineArray();
	if (distTraveled > 0)
	{
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
	if (isDoneWithEntireRoute())
	{
		return DPoint(currPolypoint->getX(), currPolypoint->getY());
	}

	//Else, scale a vector like normal
	DynamicVector movementVect(currPolypoint->getX(), currPolypoint->getY(), nextPolypoint->getX(), nextPolypoint->getY());
	movementVect.scaleVectTo(getCurrDistAlongPolyline()).translateVect();
	return DPoint(movementVect.getX(), movementVect.getY());
}

#ifndef SIMMOB_DISABLE_MPI
void sim_mob::GeneralPathMover::pack(PackageUtils& package, GeneralPathMover* fwdMovement)
{

	if (fwdMovement == NULL)
	{
		bool is_NULL = true;
		package.packBasicData(is_NULL);
		return;
	}
	else
	{
		bool is_NULL = false;
		package.packBasicData(is_NULL);
	}

//	ParitionDebugOutput::outputToConsole("sssssssss");

	int path_size = fwdMovement->fullPath.size();
	package.packBasicData<int> (path_size);

	std::vector<const sim_mob::RoadSegment*>::const_iterator itr = fwdMovement->fullPath.begin();
	for (; itr != fwdMovement->fullPath.end(); itr++)
	{

		RoadSegment::pack(package, *itr);
	}

//	ParitionDebugOutput::outputToConsole("ddddddddd");

	int current_segment = fwdMovement->currSegmentIt - fwdMovement->fullPath.begin();
	package.packBasicData<int> (current_segment);
	ParitionDebugOutput::outputToConsole(current_segment);

	//part 2
	//polypointsList
	if (fwdMovement->polypointsList.size() > 0)
	{

//		ParitionDebugOutput::outputToConsole("d11111111");

		bool hasPointList = true;
		package.packBasicData<bool> (hasPointList);

//		ParitionDebugOutput::outputToConsole("d22222222222");
		package.packBasicDataVector<Point2D> (fwdMovement->polypointsList);

		int current_poly = fwdMovement->currPolypoint - fwdMovement->polypointsList.begin();
		package.packBasicData<int> (current_poly);

//		ParitionDebugOutput::outputToConsole("d3333333333");

		int next_poly = fwdMovement->nextPolypoint - fwdMovement->polypointsList.begin();
		package.packBasicData<int> (next_poly);

//		ParitionDebugOutput::outputToConsole("d4444444444444");

		bool hasArrivedDestination = false;
		if (fwdMovement->currSegmentIt == fwdMovement->fullPath.end())
		{
			hasArrivedDestination = true;

			//			ParitionDebugOutput::outputToConsole("d43333333");
		}

		package.packBasicData<bool> (hasArrivedDestination);

		if (hasArrivedDestination == false)
		{
			//copy from GeneralPathMover.cpp, the design of current_zero_poly is not good.
			//I thought current_zero_poly is the iterator of currPolypoint
			const std::vector<Point2D>& tempLaneZero = const_cast<RoadSegment*> (*(fwdMovement->currSegmentIt))->getLaneEdgePolyline(0);

//			ParitionDebugOutput::outputToConsole("d44455555555");

			int current_zero_poly = fwdMovement->currLaneZeroPolypoint - tempLaneZero.begin();
			package.packBasicData<int> (current_zero_poly);

//			ParitionDebugOutput::outputToConsole("d55555555");

			int next_zero_poly = fwdMovement->nextLaneZeroPolypoint - tempLaneZero.begin();
			package.packBasicData<int> (next_zero_poly);

//			ParitionDebugOutput::outputToConsole("d66666666666");
		}
	}
	else
	{
		bool hasPointList = false;
		package.packBasicData<bool> (hasPointList);

//		ParitionDebugOutput::outputToConsole("d0000000000");
	}

//	ParitionDebugOutput::outputToConsole("fffffffffffff");

	//part 3
	//Others
	package.packBasicData<double> (fwdMovement->distAlongPolyline);
	package.packBasicData<double> (fwdMovement->distMovedInCurrSegment);
	package.packBasicData<double> (fwdMovement->distOfThisSegment);
	package.packBasicData<double> (fwdMovement->distOfRestSegments);

	package.packBasicData<bool> (fwdMovement->inIntersection);
	package.packBasicData<bool> (fwdMovement->isMovingForwardsInLink);

	package.packBasicData<int> (fwdMovement->currLaneID);

	std::string value_ = fwdMovement->DebugStream.str();
	package.packBasicData<std::string> (value_);

//	ParitionDebugOutput::outputToConsole("ggggggggggg");
}

void sim_mob::GeneralPathMover::unpack(UnPackageUtils& unpackage, GeneralPathMover* one_motor)
{
	bool is_NULL = unpackage.unpackBasicData<bool> ();
	if (is_NULL)
	{
		return;
	}

	int path_size = unpackage.unpackBasicData<int> ();

	for (int i = 0; i < path_size; i++)
	{
		const sim_mob::RoadSegment* one_segment = sim_mob::RoadSegment::unpack(unpackage);
		one_motor->fullPath.push_back(one_segment);
	}

	int move_size = unpackage.unpackBasicData<int> ();

	if (move_size >= 0)
	{
		one_motor->currSegmentIt = one_motor->fullPath.begin() + move_size;
	}

	bool hasPointList = unpackage.unpackBasicData<bool> ();

	if (hasPointList)
	{
		one_motor->polypointsList = unpackage.unpackBasicDataVector<Point2D> ();

		int current_poly = unpackage.unpackBasicData<int> ();

		if (current_poly >= 0)
		{
			one_motor->currPolypoint = one_motor->polypointsList.begin() + current_poly;
		}

		int next_poly = unpackage.unpackBasicData<int> ();

		if (next_poly >= 0)
		{
			one_motor->nextPolypoint = one_motor->polypointsList.begin() + next_poly;
		}

		bool hasArrivedDestination = unpackage.unpackBasicData<bool> ();

		if (hasArrivedDestination == false)
		{
			const std::vector<Point2D>& tempLaneZero = const_cast<RoadSegment*> (*(one_motor->currSegmentIt))->getLaneEdgePolyline(0);

			int current_zero_poly = unpackage.unpackBasicData<int> ();
			if (current_zero_poly >= 0)
			{
				one_motor->currLaneZeroPolypoint = tempLaneZero.begin() + current_zero_poly;
			}

			int next_zero_poly = unpackage.unpackBasicData<int> ();
			if (next_zero_poly >= 0)
			{
				one_motor->nextLaneZeroPolypoint = tempLaneZero.begin() + next_zero_poly;
			}
		}
	}

	one_motor->distAlongPolyline = unpackage.unpackBasicData<double> ();
	one_motor->distMovedInCurrSegment = unpackage.unpackBasicData<double> ();
	one_motor->distOfThisSegment = unpackage.unpackBasicData<double> ();
	one_motor->distOfRestSegments = unpackage.unpackBasicData<double> ();
	one_motor->inIntersection = unpackage.unpackBasicData<bool> ();
	one_motor->isMovingForwardsInLink = unpackage.unpackBasicData<bool> ();
	one_motor->currLaneID = unpackage.unpackBasicData<int> ();

	std::string buffer = unpackage.unpackBasicData<std::string> ();
	(one_motor->DebugStream) << buffer;

}
#endif
