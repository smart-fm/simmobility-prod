/*
 * TrainDriverFacets.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: zhang huai peng
 */
#include <atomic>
#include "conf/settings/DisableMPI.h"
#include "entities/roles/RoleFacets.hpp"
#include "TrainPathMover.hpp"

namespace sim_mob {
namespace medium{
class TrainDriver;

class TrainBehavior : public BehaviorFacet
{
public:
	explicit TrainBehavior();
	virtual ~TrainBehavior();

	/**
	 * Virtual overrides
	 */
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TrainDriver* getParentDriver() const;
	void setParentDriver(TrainDriver* parentDriver);

protected:
	/**Pointer to the parent Driver role. */
	TrainDriver* parentDriver;
};

class TrainMovement : public MovementFacet
{
public:
	explicit TrainMovement();
	virtual ~TrainMovement();

	//virtual overrides
	virtual void frame_init();
	virtual void frame_tick();
	virtual std::string frame_tick_output();

	TrainDriver* getParentDriver() const;
	void setParentDriver(TrainDriver* parentDriver);
	/**
	 * get the object of path mover
	 * @return the reference to the object of path mover
	 */
	const TrainPathMover& getPathMover() const;
	/**
	 * get the object of platform mover
	 * @return the reference to the object of platform mover
	 */
	Platform* getNextPlatform();
	/**
	 * move train forward
	 * @return true if train successfully move forward.
	 */
	bool moveForward();
	/**
	 * Whether the train already arrive at one platform
	 * @return true if train already move in one platform
	 */
	bool isStopAtPlatform();
	/**
	 * Whether the train already arrive at last platform
	 * @return true if train already arrive at terminal platform
	 */
	bool isAtLastPlaform();
	/**
	 * make train leave from current platform
	 */
	void leaveFromPlaform();
	/**
	 * assign the train to the first platform
	 */
	void arrivalAtStartPlaform() const;
	/**
	 * inform the train arrival at last platform
	 */
	void arrivalAtEndPlatform() const;
	/**
	 * get distance to next train
	 * @param next is a pointer to next TrainDriver
	 * @return the distance to next train
	 */
	double getDistanceToNextTrain(const TrainDriver* next) const;
protected:
	virtual TravelMetric& startTravelTimeMetric();
	virtual TravelMetric& finalizeTravelTimeMetric();

private:
	/**Pointer to the parent Driver role.*/
	TrainDriver* parentDriver;
	/**Train path mover*/
	TrainPathMover trainPathMover;
	/**Train platform mover*/
	TrainPlatformMover trainPlatformMover;
	/**safe distance*/
	double safeDistance;
	/**safe headway*/
	double safeHeadway;
	/**next platform*/
	std::atomic<Platform*> nextPlatform;
private:
	/**
	 * get current speed limit
	 * @return current speed limit
	 */
	double getRealSpeedLimit();
	/**
	 * get effective speed
	 * @return effective speed
	 */
	double getEffectiveAccelerate();
	/**
	 * get effective moving distance
	 * @return effective distance
	 */
	double getEffectiveMovingDistance();
	/**
	 * is station case happen?
	 * @return true when station case happen
	 */
	bool isStationCase(double disToTrain, double disToPlatform, double& effectDis);
};

}

} /* namespace sim_mob */


