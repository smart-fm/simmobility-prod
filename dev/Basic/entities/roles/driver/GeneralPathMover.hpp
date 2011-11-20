/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "util/DynamicVector.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/StreetDirectory.hpp"


namespace sim_mob
{

//Forward declarations
class RoadSegment;
class Link;



/**
 * The GeneralPathMover allows a vehicle to move forward in a series of RoadSegments based entirely
 * on its forward movement "amount".
 */
class GeneralPathMover {
public:
	GeneralPathMover();

	///Set the path of RoadSegments contained in our path. These segments need not
	/// necessarily be in the same Link.
	///TODO: I'm not entirely sure that all cases of fwd/rev RoadSegments are handled properly.
	void setPath(const std::vector<const sim_mob::RoadSegment*>& path, bool firstSegMoveFwd, int startLaneID);

	///Is it possible to move? Attempting to operate on a GeneralPathmover which has no RoadSegments in
	/// its path is an error.
	bool isPathSet() const;

	///General forward movement function: move X cm forward. Automatically switches to a new polypoint or
	///  road segment as applicable.
	//Returns any "overflow" distance if we are in an intersection, 0 otherwise.
	double advance(double fwdDistance);

	///Are we completely done?
	bool isDoneWithEntireRoute() const;

	//Are we within an intersection?
	//Note that this will NOT return true if you end on an intersection --all movement stops once
	// the destination node has been reached.
	bool isInIntersection() const { return inIntersection; }

	//Indicate that we are done processing the current intersection. Moves the user to the next
	//  road segment.
	const Lane* leaveIntersection();

	//Retrieve properties of your current position in the path.
	const sim_mob::RoadSegment* getCurrSegment() const;
	const sim_mob::RoadSegment* getNextSegment(bool sameLink) const;
	const sim_mob::RoadSegment* getPrevSegment(bool sameLink) const;
	const sim_mob::Link* getCurrLink() const;
	const sim_mob::Point2D& getCurrPolypoint() const;
	const sim_mob::Point2D& getNextPolypoint() const;
	//bool isMovingForwardsOnCurrSegment() const;
	double getCurrLinkLength() const;

	//Retrieve useful properties of the current polypoint
	double getCurrDistAlongPolyline() const;
	double getCurrPolylineTotalDist() const;

	//Retrieve the current distance moved in this road segment. Due to lane changing, etc., it
	//  is entirely possible that this may appear longer than the actual RoadSegment.
	//Note that this function does not attempt to subtract any distance moved beyond the
	//  limit of the current polyline.
	//You will almost always want to use getCurrDistAlongPolyline() instead of this function.
	double getCurrDistAlongRoadSegment() const;

	//Retrieve our X/Y position based ONLY on forward movement (e.g., nothing with Lanes)
	sim_mob::DPoint getPosition() const;

	//We might be able to fold Lane movement in here later. For now, it has to be called externally.
	void shiftToNewPolyline(bool moveLeft);
	void moveToNewPolyline(int newLaneID);


private:
	//List of RoadSegments we're moving to in order.
	std::vector<const sim_mob::RoadSegment*> fullPath;
	std::vector<const sim_mob::RoadSegment*>::iterator currSegmentIt;

	//This can change dynamically (lane changes, etc.)
	std::vector<sim_mob::Point2D> polypointsList;
	std::vector<sim_mob::Point2D>::iterator currPolypoint;
	std::vector<sim_mob::Point2D>::iterator nextPolypoint;

	//Movement along a single line
	double distAlongPolyline;
	double currPolylineLength() const {
		//TEMP: Just making sure.
		if (isInIntersection()) {
			return distAlongPolyline;
		}
		DynamicVector temp(currPolypoint->getX(), currPolypoint->getY(),nextPolypoint->getX(), nextPolypoint->getY());
		return temp.getMagnitude();
	}

	//Counter
	double distMovedInSegment;

	//Intersection driving is different.
	bool inIntersection;

	//We might be moving backwards on a Link.
	bool isMovingForwardsInLink;

	//For tracking lane IDs
	int currLaneID;

private:
	//Helper functions
	double advanceToNextPolyline();
	double advanceToNextRoadSegment();
	const Lane* actualMoveToNextSegmentAndUpdateDir();
	void generateNewPolylineArray();

	//General throw function. There is probably a better way to do this.
	void throwIf(bool conditional, const std::string& msg) const {
		if (conditional) {
			throw std::runtime_error(msg.c_str());
		}
	}






};


}



