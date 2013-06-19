//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PedestrianPathMover.h
 *
 *  Created on: Aug 20, 2012
 *      Author: redheli
 */

#pragma once

#include "util/DynamicVector.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/RoadSegment.hpp"
#include <vector>
#include <boost/unordered_map.hpp>

namespace sim_mob
{
inline std::size_t hash_value(const sim_mob::Point2D& p)
{
    size_t seed = 0;
    boost::hash_combine(seed, boost::hash_value(p.getX()));
    boost::hash_combine(seed, boost::hash_value(p.getY()));
    return seed;
}

class PedestrianPathMover {
public:
	PedestrianPathMover();
	~PedestrianPathMover();
public:
	void setPath(const std::vector<WayPoint> path);
	///General forward movement function: move X cm forward. Automatically switches to a new polypoint or
	///  road segment as applicable.
	//Returns any "overflow" distance if we are in an intersection, 0 otherwise.
	double advance(double fwdDistance);
	//Retrieve our X/Y position based ONLY on forward movement (e.g., nothing with Lanes)
	sim_mob::DPoint getPosition() const;
	WayPoint* getCurrentWaypoint(){ return currentWaypoint; };
public:
	std::vector<WayPoint> pedestrian_path;
	std::vector<WayPoint>::iterator pedestrian_path_iter;
	double currPolylineLength();
	bool isDoneWithEntireRoute() { return isDoneWithEntirePath; };
	bool isAtCrossing();
private:
	WayPoint *currentWaypoint;
	WayPoint *nextWaypoint;
	// advance to next waypoint, if has next waypoint ,return true
	bool advanceToNextPolylinePoint();
	std::vector<sim_mob::Point2D> getCrossingPolylinePoints(WayPoint *wp);
	void relToAbs(double xRel, double yRel, double & xAbs, double & yAbs,
			double cStartX,double cStartY,double cEndX,double cEndY);
	void absToRel(double xAbs, double yAbs, double & xRel, double & yRel,double cStartX,
			double cStartY,double cEndX,double cEndY);
private:
	double distAlongPolyline;
	bool isDoneWithEntirePath;
	//ploy line start ,end point , maybe cross two segments or segment->crossring
	std::vector<sim_mob::Point2D> polylinePoints;
	std::vector<sim_mob::Point2D>::iterator currPolylineStartpoint;
	std::vector<sim_mob::Point2D>::iterator currPolylineEndpoint;
	boost::unordered_map<sim_mob::Point2D,WayPoint* > polylinePoint_wayPoint_map;
};
}
