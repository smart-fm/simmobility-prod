//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <sstream>
#include <vector>

#include "conf/settings/DisableMPI.h"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/TurningPath.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "metrics/Length.hpp"
#include "util/DynamicVector.hpp"

namespace sim_mob
{
class RoadSegment;
class Link;
class Lane;
class PackageUtils;
class UnPackageUtils;

/**
 * The DriverPathMover allows to move forward in a series of RoadSegments based entirely
 * on its forward movement "amount".
 *
 * \author Neeraj D
 */
class DriverPathMover
{
private:
	/**The current lane of the driver (this will be null while driving in an intersection)*/
	const Lane *currLane;

	/**The current turning path of the driver (this will be non-null while driving in an intersection)*/
	const TurningPath *currTurning;

	/**The entire driving path consisting of road-segments and turning groups*/
	std::vector<WayPoint> drivingPath;

	/**An iterator pointing to the current way-point (road segment/turning group) in the driving path*/
	std::vector<WayPoint>::const_iterator currWayPointIt;

	/**The current poly-line*/
	const PolyLine *currPolyLine;

	/**An iterator pointing to the current poly-point in the current poly-line*/
	std::vector<PolyPoint>::const_iterator currPolyPoint;

	/**An iterator pointing to the next poly-point in the current poly-line*/
	std::vector<PolyPoint>::const_iterator nextPolyPoint;

	/**Indicates whether the driver is in an intersection*/
	bool inIntersection;

	/**Stores the distance moved along the partial poly-line (from the current point to the next point)*/
	double distCoveredFromCurrPtToNextPt;

	/**Stores the distance covered by the driver on the current way-point*/
	double distCoveredOnCurrWayPt;

	/**
	 * Calculates the distance between the current poly-point and the next poly-point
	 *
     * @return the calculated distance
     */
	double calcDistFromCurrToNextPt();

	/**
	 * Advances the driver's position along the poly-line to the next poly-point
	 *
     * @return overflow distance, if we move into an intersection, 0 otherwise
     */
	double advanceToNextPoint();

	/**
	 * Advances the driver's position to the next poly-line
	 *
     * @return overflow distance, if we move into an intersection, 0 otherwise
     */
	double advanceToNextPolyLine();
	
	/**
	 * NOTE: We are keeping these as constant & static because the simulation runtime keeps re-creating
	 * them on each call to throwIf().
	 */

	/**Error message to be thrown if the path is not set*/
	const static std::string ErrorDrivingPathNotSet;

	/**Error message to be thrown if the driver is not in an intersection*/
	const static std::string ErrorNotInIntersection;

	/**Error message to be thrown if the entire route is done*/
	const static std::string ErrorEntireRouteDone;

public:
	DriverPathMover();
	DriverPathMover(const DriverPathMover &pathMover);

	const Lane* getCurrLane() const;
    const TurningPath* getCurrTurning() const;
    
    const std::vector<WayPoint>& getDrivingPath() const;
    std::vector<WayPoint>::const_iterator getCurrWayPointIt() const;
    
    bool isInIntersection() const;

	const PolyPoint& getCurrPolyPoint() const;
	const PolyPoint& getNextPolyPoint() const;

	const WayPoint& getCurrWayPoint() const;
	const WayPoint* getNextWayPoint() const;

	const RoadSegment* getCurrSegment() const;
	const Link* getCurrLink() const;

	/**
	 * Determines the next segment of the current link in the driving path and returns it
	 *
     * @return the next segment (in the current link) in the driving path if it exists, else NULL
     */
	const RoadSegment* getNextSegment() const;

	/**
	 * Determines the next segment of the next link in the driving path and returns it
	 *
     * @return the next segment (in the next link) in the driving path if it exists, else NULL
     */
	const RoadSegment* getNextSegInNextLink() const;

	/**
	 * Determines the next link in the driver path and returns it
	 *
     * @return the next link in the driving path if it exists, else NULL
     */
	const Link* getNextLink() const;

	/**
	 * Uses the physical lane connections (i.e. connected poly-lines) to determine the next lane.
	 *
     * @return the next lane
     */
	const Lane* getNextLane() const;

	/**
	 * Sets the driving path and initialises the internal members to point to the elements in the path
	 *
     * @param path the path retrieved from the street directory or the path set manager
     * @param startLaneIndex the index of the starting lane (optional)
	 * @param startSegmentId the id of starting segment (optional)
     */
	void setPath(const std::vector<WayPoint> &path, int startLaneIndex = -1, int startSegmentId = 0);

	/**
	 * Checks if the path has been set
	 *
     * @return true if the path is set, else false
     */
	bool isDrivingPathSet() const;

	/**
	 * Checks if the driver has completed driving along the assigned path
	 *
     * @return true if the route has been completed, else false
     */
	bool isDoneWithEntireRoute() const;

	/**
	 * Advances the driver's position along the poly-line by the given amount.
	 *
     * @param distance the distance (in metre) by which the driver is to be moved forward
	 *
     * @return overflow distance, if we move into an intersection, 0 otherwise
     */
	double advance(double distance);

	/**
	 * Updates the current lane and the current poly-line information after a lane changing move
	 * has completed
	 *
     * @param lane the new lane
     */
	void updateLateralMovement(const Lane *lane);

	/**
	 * Calculates the distance covered on the current road way-point
	 *
     * @return the distance covered on the current way point (in metre)
     */
	double getDistCoveredOnCurrWayPt() const;

	/**
	 * Calculates the distance remaining to the end of the current way point
     * 
	 * @return the distance remaining to the end of the current way point (in metre)
     */
	double getDistToEndOfCurrWayPt() const;

	/**
	 * Calculates the distance remaining to the end of the current link
	 *
     * @return  the distance remaining to the end of the current link (in metre)
     */
	double getDistToEndOfCurrLink() const;

	/**
	 * Computes the current position based on the poly-line and the distance moved
	 *
     * @return the point representing the current position
     */
	const Point getPosition();

#ifndef SIMMOB_DISABLE_MPI
	friend class PackageUtils;
	friend class UnPackageUtils;

	static void pack(PackageUtils& package, DriverPathMover* one_mover);
	static void unpack(UnPackageUtils& unpackage, DriverPathMover* one_motor);
#endif
} ;
}
