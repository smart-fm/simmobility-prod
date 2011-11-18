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
	///Set the path of RoadSegments contained in our path. These segments need not
	/// necessarily be in the same Link.
	void setPath(std::vector<sim_mob::WayPoint> wp_path) ;

	///Set the path of RoadSegments contained in our path. These segments need not
	/// necessarily be in the same Link.
	void setPath(const std::vector<sim_mob::RoadSegment*>& path);

	///Is it possible to move? Attempting to operate on a GeneralPathmover which has no RoadSegments in
	/// its path is an error.
	bool isPathSet() const;

	///General forward movement function: move X cm forward. Automatically switches to a new polypoint or
	///  road segment as applicable.
	void advance(centimeter_t fwdDistance);

	///Are we completely done?
	bool isDoneWithEntireRoute() const;

	//Are we within an intersection?
	//Note that this will NOT return true if you end on an intersection --all movement stops once
	// the destination node has been reached.
	bool isInIntersection() const { return inIntersection; }

	//Indicate that we are done processing the current intersection. Moves the user to the next
	//  road segment.
	void leaveIntersection();

	//Retrieve properties of your current position in the path.
	const sim_mob::RoadSegment* getCurrSegment() const;
	const sim_mob::Link* getCurrLink() const;
	const sim_mob::Point2D& getCurrPolypoint() const;
	const sim_mob::Point2D& getNextPolypoint() const;

	//Retrieve useful properties of the current polypoint
	centimeter_t getCurrDistAlongPolyline() const;
	centimeter_t getCurrPolylineTotalDist() const;

	//Retrieve our X/Y position based ONLY on forward movement (e.g., nothing with Lanes)
	sim_mob::DPoint getPosition() const;


private:
	//List of RoadSegments we're moving to in order.
	std::vector<const sim_mob::RoadSegment*> fullPath;
	std::vector<const sim_mob::RoadSegment*>::iterator currSegmentIt;

	//This can change dynamically (lane changes, etc.)
	std::vector<sim_mob::Point2D> polypointsList;
	std::vector<sim_mob::Point2D>::iterator currPolypoint;
	std::vector<sim_mob::Point2D>::iterator nextPolypoint;

	//Movement along a single line
	centimeter_t distAlongPolyline;
	centimeter_t currPolylineLength;

	//Intersection driving is different.
	bool inIntersection;

private:
	//Helper functions
	void advanceToNextPolyline();
	void advanceToNextRoadSegment();

	//General throw function. There is probably a better way to do this.
	void throwIf(bool conditional, const std::string& msg) const {
		if (conditional) {
			throw std::runtime_error(msg.c_str());
		}
	}






};


}



