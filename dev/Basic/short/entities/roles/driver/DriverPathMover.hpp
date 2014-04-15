//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <sstream>

#include "conf/settings/DisableMPI.h"

#include "geospatial/Point2D.hpp"
#include "metrics/Length.hpp"
#include "util/DynamicVector.hpp"


namespace sim_mob
{

//Forward declarations
class RoadSegment;
class Link;
class Lane;

class PackageUtils;
class UnPackageUtils;



/**
 * The DriverPathMover allows to move forward in a series of RoadSegments based entirely
 * on its forward movement "amount".
 *
 * \author Yao Jin

 */
class DriverPathMover {
public:
	DriverPathMover();
	DriverPathMover(const DriverPathMover& copyFrom); ///<Copy constructor, used to make sure iterators work cleanly.

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
	const sim_mob::Point2D& getNextPolypointNew() const;
	double getCurrLinkReportedLengthCM() const;

	//Retrieve useful properties of the current polypoint
	double getCurrDistAlongPolylineCM() const;
	double getCurrPolylineTotalDistCM() const;

	// segment length is based on lane's polypoints , be careful, it is not relate to segment's start ,end nodes
	// unit cm
	double getCurrentSegmentLengthCM();

	//Retrieve the current distance moved in this road segment. Due to lane changing, etc., it
	//  is entirely possible that this may appear longer than the actual RoadSegment.
	//Note that this function does not attempt to subtract any distance moved beyond the
	//  limit of the current polyline. (But it should be close; we try to normalize it).
	//You will almost always want to use getCurrDistAlongPolyline() instead of this function.
	double getCurrDistAlongRoadSegmentCM() const;

	//Get what should be the total distance of the RoadSegment.
	double getTotalRoadSegmentLengthCM() const;

	//Get the length of rest road segments in current link, include current road segment
	double getAllRestRoadSegmentsLengthCM() const;

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
	double getPositionInSegmentCM();
	void setPositionInSegmentCM(double newDistToEndCM);
	void setStartPositionInSegmentCM();
	double getNextSegmentLengthCM();
	void advance_med(double fwdDistance);
	void actualMoveToNextSegmentAndUpdateDir_med();
	double currPolylineLengthCM() const;

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
	double distAlongPolylineCM;

	//Counter
	//NOTE: This is always the same regardless of what lane you're in. In other words,
	//      you take (distAlongPolyline/currPolylineLength) and multiply that by some "normalized"
	//      distance for that Segment (e.g., the median lane line) and then add that to the normalized distances
	//      for all previous Segments. This is important as it prevents obstacles from appearing in the wrong
	//      places to different drivers.
	//NOTE:  This does NOT include the distance moved in the current polyline.
	double distMovedInCurrSegmentCM;

	//And track the expected total distance(CM).
	double distOfThisSegmentCM;

	//length(CM) of rest road segments in current link, include current segment
	double distOfRestSegmentsCM;

	//distance(CM) to the end of the current segment. needed by mid-term
	double distToEndSegmentCM;

	//Intersection driving is different.
	bool inIntersection;

	//We might be moving backwards on a Link.
	//TODO: This is still relevant (even with 1-way Links) since Pedestrians can move backwards on a Link.
	//       Note that we need to merge this code with GeneralPathMover2.
	bool isMovingForwardsInLink;

	//For tracking lane IDs
	int currLaneID;

	//Debug, the debug message also will be transformed and reset PC
	mutable std::stringstream DebugStream;

	struct PathWithDirection{
		std::vector<const RoadSegment*> path;
		std::vector<bool> areFwds;
	} pathWithDirection;

private:
	//Error messages for throw_if.
	//NOTE: We are keeping these as const-static because the simulation runtime keeps re-creating them
	//      on each call to throwIf().
	const static std::string ErrorPathNotSet;
	const static std::string ErrorPolylineLength;
	const static std::string ErrorNotInIntersection;
	const static std::string ErrorAdvancePathDone1;
	const static std::string ErrorAdvancePathDone2;
	const static std::string ErrorPolylineCantAdvance;
	const static std::string ErrorRoadSegmentAtEnd;
	const static std::string ErrorPathDoneActual;
	const static std::string ErrorGeneralPathDone;

	//Helper functions
	double advanceToNextPolyline(bool isFwd);
	double advanceToNextRoadSegment();
	const Lane* actualMoveToNextSegmentAndUpdateDir();
	void generateNewPolylineArray(const RoadSegment* currSegment, std::vector<const RoadSegment*> path, std::vector<bool> areFwds);
	void generateNewPolylineArray();
	void calcNewLaneDistancesCM();
	static double CalcSegmentLaneZeroDistCM(std::vector<const sim_mob::RoadSegment*>::const_iterator start, std::vector<const sim_mob::RoadSegment*>::const_iterator end);
	static double CalcRestSegmentsLaneZeroDistCM(std::vector<const sim_mob::RoadSegment*>::const_iterator start, std::vector<const sim_mob::RoadSegment*>::const_iterator end);
	static std::string centimeterToMeter(centimeter_t dist); //Helper to format cm as m for debug output.

	//General throw function. There is probably a better way to do this.
	void throwIf(bool conditional, const std::string& msg) const;


	//Serialization-related friends
	friend class PackageUtils;
	friend class UnPackageUtils;


};


}
