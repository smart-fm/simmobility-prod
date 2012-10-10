/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <sstream>

#include "util/DynamicVector.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/StreetDirectory.hpp"


namespace sim_mob
{

//Forward declarations
class RoadSegment;
class Link;

class PackageUtils;
class UnPackageUtils;



/**
 * The GeneralPathMover allows a vehicle to move forward in a series of RoadSegments based entirely
 * on its forward movement "amount".
 *
 * \author Seth N. Hetu
 * \author Li Zhemin
 * \author Xu Yan
 * \author LIM Fung Chai
 */
class GeneralPathMover {
public:
	GeneralPathMover();
	GeneralPathMover(const GeneralPathMover& copyFrom); ///<Copy constructor, used to make sure iterators work cleanly.

	///Set the path of RoadSegments contained in our path. These segments need not
	/// necessarily be in the same Link.
	///TODO: I'm not entirely sure that all cases of fwd/rev RoadSegments are handled properly.
	void setPath(const std::vector<const sim_mob::RoadSegment*>& path, std::vector<bool>& areFwds, int startLaneID);
	void setPath(const std::vector<const sim_mob::RoadSegment*>& path, int startLaneID);

	//reset path
	//in route choice model, it will be used when the vehicle is approaching an intersection
	//and it needs to find a new route
	void resetPath(const std::vector<const sim_mob::RoadSegment*>& path);

	///Is it possible to move? Attempting to operate on a GeneralPathmover which has no RoadSegments in
	/// its path is an error.
	bool isPathSet() const;

	///General forward movement function: move X cm forward. Automatically switches to a new polypoint or
	///  road segment as applicable.
	//Returns any "overflow" distance if we are in an intersection, 0 otherwise.
	double advance(double fwdDistance);
	double advance(const RoadSegment* currSegment, std::vector<const RoadSegment*> path, std::vector<bool> areFwds, double fwdDistance);
	double advanceToNextRoadSegment(); //~moved to public by melani - for use externally for mid-term
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
	const RoadSegment* getNextToNextSegment() const;
	const sim_mob::RoadSegment* getPrevSegment(bool sameLink) const;
	const sim_mob::Link* getCurrLink() const;
	const sim_mob::Lane* getCurrLane() const;
	const sim_mob::Point2D& getCurrPolypoint() const;
	const sim_mob::Point2D& getNextPolypoint() const;
	//bool isMovingForwardsOnCurrSegment() const;
	double getCurrLinkReportedLength() const;

	//Retrieve useful properties of the current polypoint
	double getCurrDistAlongPolyline() const;
	double getCurrPolylineTotalDist() const;

	// segment length is based on lane's polypoints , be careful, it is not relate to segment's start ,end nodes
	// unit cm
	double getCurrentSegmentLength();

	//Retrieve the current distance moved in this road segment. Due to lane changing, etc., it
	//  is entirely possible that this may appear longer than the actual RoadSegment.
	//Note that this function does not attempt to subtract any distance moved beyond the
	//  limit of the current polyline. (But it should be close; we try to normalize it).
	//You will almost always want to use getCurrDistAlongPolyline() instead of this function.
	double getCurrDistAlongRoadSegment() const;

	//Get what should be the total distance of the RoadSegment.
	double getTotalRoadSegmentLength() const;

	//Get the length of rest road segments in current link, include current road segment
	double getAllRestRoadSegmentsLength() const;

	//Retrieve our X/Y position based ONLY on forward movement (e.g., nothing with Lanes)
	sim_mob::DPoint getPosition() const;

	//We might be able to fold Lane movement in here later. For now, it has to be called externally.
	void shiftToNewPolyline(bool moveLeft);
	void moveToNewPolyline(int newLaneID);

#ifndef SIMMOB_DISABLE_MPI
public:
	///Serialization
	static void pack(PackageUtils& package, GeneralPathMover* one_mover);

	static void unpack(UnPackageUtils& unpackage, GeneralPathMover* one_motor);
#endif

	/* needed by mid-term */
	double getPositionInSegment();
	void setPositionInSegment(double newDist2end);
	double getNextSegmentLength();
//temp
//private:
public:
	//List of RoadSegments we're moving to in order.
	std::vector<const sim_mob::RoadSegment*> fullPath;
	std::vector<const sim_mob::RoadSegment*>::iterator currSegmentIt;

	//This can change dynamically (lane changes, etc.)
	std::vector<sim_mob::Point2D> polypointsList;
	std::vector<sim_mob::Point2D> laneZeroPolypointsList;
	std::vector<sim_mob::Point2D>::iterator currPolypoint;
	std::vector<sim_mob::Point2D>::iterator nextPolypoint;

	//Unfortuante duplication, but necessary to avoid aliasing
	std::vector<sim_mob::Point2D>::const_iterator currLaneZeroPolypoint;
	std::vector<sim_mob::Point2D>::const_iterator nextLaneZeroPolypoint;

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
	//NOTE: This is always the same regardless of what lane you're in. In other words,
	//      you take (distAlongPolyline/currPolylineLength) and multiply that by some "normalized"
	//      distance for that Segment (e.g., the median lane line) and then add that to the normalized distances
	//      for all previous Segments. This is important as it prevents obstacles from appearing in the wrong
	//      places to different drivers.
	//NOTE:  This does NOT include the distance moved in the current polyline.
	double distMovedInCurrSegment;

	//And track the expected total distance.
	double distOfThisSegment;

	//length of rest road segments in current link, include current segment
	double distOfRestSegments;

	//distance to the end of the current segment. needed by mid-term
	double distToEndSegment;

	//Intersection driving is different.
	bool inIntersection;

	//We might be moving backwards on a Link.
	bool isMovingForwardsInLink;

	//For tracking lane IDs
	int currLaneID;

	//Debug, the debug message also will be transformed and reset PC
	mutable std::stringstream DebugStream;

	struct PathWithDirection{
		std::vector<const RoadSegment*> path;
		std::vector<bool> areFwds;
	}pathWithDirection;

private:
	//Helper functions
	double advanceToNextPolyline(bool isFwd);
	//double advanceToNextRoadSegment();
	const Lane* actualMoveToNextSegmentAndUpdateDir();
	void generateNewPolylineArray(const RoadSegment* currSegment, std::vector<const RoadSegment*> path, std::vector<bool> areFwds);
	void generateNewPolylineArray();
	void calcNewLaneDistances();
	static double CalcSegmentLaneZeroDist(std::vector<const sim_mob::RoadSegment*>::const_iterator start, std::vector<const sim_mob::RoadSegment*>::const_iterator end);
	static double CalcRestSegmentsLaneZeroDist(std::vector<const sim_mob::RoadSegment*>::const_iterator start, std::vector<const sim_mob::RoadSegment*>::const_iterator end);
	static std::string Fmt_M(centimeter_t dist); //Helper to format cm as m for debug output.

	//General throw function. There is probably a better way to do this.
	void throwIf(bool conditional, const std::string& msg) const;


	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;


};


}


