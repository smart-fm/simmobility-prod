/*
 * TrainPathMover.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */

#include "geospatial/network/Point.hpp"
#include "geospatial/network/Block.hpp"
#include "geospatial/network/Platform.hpp"
#include "boost/thread/mutex.hpp"

namespace sim_mob {
class TrainPlatformMover {
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
	Platform* getPlatformByOffset(unsigned int offset);
private:
	/**driving platforms*/
	std::vector<Platform*> platforms;
	/**the iterator to current platform*/
	std::vector<Platform*>::iterator currPlatformIt;
	/**record previous platform*/
	std::vector<Platform*> prevPlatforms;
};
class TrainPathMover {
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

	int GetCurrentBlockId();

	double GetDistanceFromStartToPlatform(std::string lineId,Platform *platform);

	void TeleportToOppositeLine(std::string station,std::string lineId,Platform *platform);
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
	/**the locker for this mover*/
	mutable boost::mutex moverMutex;
};

} /* namespace sim_mob */


