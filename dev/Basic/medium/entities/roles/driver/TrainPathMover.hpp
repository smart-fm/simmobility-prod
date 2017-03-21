/*
 * TrainPathMover.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */
#pragma once
#include "geospatial/network/Point.hpp"
#include "geospatial/network/Block.hpp"
#include "geospatial/network/Platform.hpp"
#include "boost/thread/mutex.hpp"
#include "entities/roles/RoleFacets.hpp"
namespace sim_mob
{
class TrainPlatformMover
{
public:
	TrainPlatformMover();
	virtual ~TrainPlatformMover();
public:
	/**
	 * set platform list
	 * @param platforms are a list of platforms
	 */
	void setPlatforms(const std::vector<Platform*>& platform);
	/**
	 * get first platform
	 * @return first platform if have, otherwise return null.
	 */
	Platform* getFirstPlatform() const;
	/**
	 * get next platform
	 * @return next platform if have, otherwise return null.
	 */
	Platform* getNextPlatform(bool updated=false);
	/**
	 * is arriving at last platform
	 */
	bool isLastPlatform();
	/**
	 * get previous platform
	 * @return previous platform
	 */
	const std::vector<Platform*>& getPrevPlatforms() const;
	/**
	 * get next platform shifted by offset
	 * @param offset is the shifted offset from current platform
	 * @return empty if offset is beyond last item
	 */
	Platform* getPlatformByOffset(int offset) const;

	/**
	 * Interface to return the last platform on route
	 * @return pointer to the last platform
	 */
	Platform * getLastPlatformOnRoute() const;

	/**
	 * This interface clears the list of previous platforms of train path
	 */
	void clearPrevPlatforms();

	/**
	 * This interface resets the currPlatformItr to the beginning
	 */
	void resetPlatformItr();

	/**
	 * This interface sets the currPlatformItr iterator to the end
	 */
	void setPlatformIteratorToEnd();
	const std::vector<Platform*>& getPlatforms() const;
private:
	/**driving platforms*/
	std::vector<Platform*> platforms;
	/**the iterator to current platform*/
	std::vector<Platform*>::iterator currPlatformIt;
	/**record previous platform*/
	std::vector<Platform*> prevPlatforms;
};

class TrainPathMover
{
public:
	TrainPathMover();

	virtual ~TrainPathMover();
public:
	double advance(double distance);
	/**
	 * Sets the driving path
     * @param path is a list of blocks
     */
	void setPath(const std::vector<Block*> &path);
	/**
	 * get distance to next platform
	 * @param platform is the pointer to next platform
	 */
	double getDistanceToNextPlatform(Platform* platform) const;
	/**
	 * get distance to next train
	 * @param other is the path mover of next train
	 */
	double getDistanceToNextTrain(const TrainPathMover& other) const;
	/**
	 * get different distance about total moved path
	 * @param other is the path mover of next train
	 */
	double getDifferentDistance(const TrainPathMover& other) const;
	/**
	 * get distance in total movement
	 * @return total distance.
	 */
	double getTotalCoveredDistance() const;
	/**
	 * check whether the path is completed
	 * @return true if the path is completed
	 */
	bool isCompletePath() const;
	/**
	 * check whether the path is set
	 * @return true if the path is set
	 */
	bool isDrivingPath() const;
	/**
	 * Calculates the distance covered on the current block
     * @return the distance covered on the current block
     */
	double getDistCoveredOnCurrBlock() const;
	/**
	 * get current speed limit defined in current block
	 * @return current speed limit
	 */
	double getCurrentSpeedLimit();
	/**
	 * get current deceleration rate defined in current block
	 * @return current deceleration rate
	 */
	double getCurrentDecelerationRate();
	/**
	 * get current acceleration rate defined in current block
	 * @return current acceleration rate
	 */
	double getCurrentAccelerationRate();
	/**
	 * get current position
	 * @return current position
	 */
	Point getCurrentPosition() const;

	/**
	 * This interface returns the current block id
	 * @return current block id
	 */
	int getCurrentBlockId() const;

	/**
	 * This interface returns the distance from start to a specific platform on a train line
	 * @param lineId is the id of the line
	 * @param platform is the pointer to the platform whose distance is to be known
	 * @return distance
	 */
	double getDistanceFromStartToPlatform(std::string lineId,Platform *platform) const;

	/**
	 * This interface teleports the train to opposite platform that is it
	 * sets the position on block on the opposite platform and the distance covered
	 * and current point on polyline
	 * @param station is the name of the staion on opposite line
	 * @param lineId is the id of the line
	 * @param platform is the pointer to the platform
	 */
	void teleportToOppositeLine(std::string station,std::string lineId,Platform *platform);

	/**
	 * This interface sets the movementFacet for the train driver
	 * @param movementFacte is the movement facet to be set
	 */
	void setParentMovementFacet(MovementFacet * movementFacet);

	/**
	 * This interface finds the nearest stop point from current point
	 * @param pointVector is the vector of stop points
	 * @return is the iterator to nearest stop point
	 */
	std::vector<PolyPoint>::const_iterator findNearestStopPoint(const std::vector<PolyPoint>& pointVector) const;
	/**
	 * Interface to calculate the distance between the two points on train line
	 * @param a is the start point
	 * @param b is the end point
	 * @polyPoints is the vector of polypoints from which the two points are taken
	 * @return is the distance between the points
	 */
	double calcDistanceBetweenTwoPoints(std::vector<PolyPoint>::const_iterator& a,std::vector<PolyPoint>::const_iterator& b,const std::vector<PolyPoint> &polyPoints) const;

	/**
	 * This interface calculates the shortest distance distance between the two points
	 * @param a is the first point
	 * @param b is the second point
	 * @return the distance between two points
	 */
	double calcDistanceBetweenCurrentAndSubsequentPoint(Point a,Point b) const;

	/**
	 * This interface returns the distance moved to next point,that is if the train is between two points
	 * so the remaining distance to next point
	 * @return the distance moved to next point
	 */
	double getDistanceMoveToNextPoint();

	/**
	 * This interface returns the point after certain distance specified on the train path from beginning of route
	 * @param distance specified on the train path from beginning of route
	 * @return PolyPoint is the point after the specified distance
	 */
	PolyPoint GetStopPoint(double distance) const;

	/**
	 * This interface returns the current point of the train on its path
	 * @return iterator to current point on path
	 */
	std::vector<PolyPoint>::const_iterator GetCurrentStopPoint() const;

	/**
	 * This interface teleports the train to platform specified
	 * @param platformName is the name of the platform
	 */
	void teleportToPlatform(std::string platformName);


	MovementFacet* GetParentMovementFacet() const;
private:
	/**
	 * Calculates the distance between the current poly-point and the next poly-point
	 * @return the calculated distance
	 */
	double calcDistanceBetweenTwoPoints() const;
	/**
	 * Advances the driver's position along the poly-line to the next poly-point
	 * @return true when successfully move to next polyline point
	 */
	bool advanceToNextPoint();
	/**
	 * Advances the driver's position to the next block
	 * @return true if successfully move to block
	 */
	bool advanceToNextBlock();



private:
	/**The driving path is a vector of blocks*/
	std::vector<Block*> drivingPath;
	/**An iterator pointing to current block*/
	std::vector<Block*>::const_iterator currBlockIt;
	/**The current poly-line*/
	const PolyLine *currPolyLine;
	/**An iterator pointing to the current poly-point in the current poly-line*/
	std::vector<PolyPoint>::const_iterator currPolyPointIt;
	/**An iterator pointing to the next poly-point in the current poly-line*/
	std::vector<PolyPoint>::const_iterator nextPolyPointIt;
	/**Stores the distance moved along the partial poly-line*/
	double distanceMoveToNextPoint;
	/**Stores the distance covered by the driver on the current block*/
	double distMovedOnCurrBlock;
	/**Stores the distance covered by driver on entire path*/
	double distMovedOnEntirePath;
	MovementFacet *parentMovementFacet;
	/**the locker for this mover*/
	mutable boost::mutex moverMutex;
};

} /* namespace sim_mob */


