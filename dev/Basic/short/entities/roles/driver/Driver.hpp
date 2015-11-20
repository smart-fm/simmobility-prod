//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <math.h>
#include <set>
#include <vector>

#include "buffering/Shared.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/Person_ST.hpp"
#include "entities/roles/Role.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "entities/roles/driver/models/CarFollowModel.hpp"
#include "entities/roles/driver/models/LaneChangeModel.hpp"
#include "entities/roles/driver/models/IntersectionDrivingModel.hpp"
#include "message/Message.hpp"
#include "perception/FixedDelayed.hpp"
#include "util/DynamicVector.hpp"
#include "util/Math.hpp"

#include "DriverUpdateParams.hpp"
#include "DriverFacets.hpp"

namespace sim_mob
{

//Forward declarations
class Signal;
class Link;
class RoadSegment;
class Lane;
class Node;
class Point;
class UpdateParams;
class DriverBehavior;
class DriverMovement;

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * \author Wang Xinyuan
 * \author Li Zhemin
 * \author Runmin Xu
 * \author Seth N. Hetu
 * \author Luo Linbo
 * \author LIM Fung Chai
 * \author Zhang Shuai
 * \author Xu Yan
 */

class Driver : public Role<Person_ST>, public UpdateWrapper<DriverUpdateParams>
{
private:
	/**
	 * Indicates whether the driver is in a loading queue. There isn't actually any data structure to represent this
	 * queue. We use the fact that at every time tick, agents are going to be processed sequentially anyway.
	 * If true, it means that there is no space for it on the road.
	 */
	bool isVehicleInLoadingQueue;

	/**Indicates whether the position of the vehicle has been found.*/
	bool isVehiclePositionDefined;

	/**Indicates whether we are yielding to another driver at a conflict in the intersection*/
	int yieldingToInIntersection;

	/**Represents the vehicle this driver is controlling.*/
	Vehicle *vehicle;

	/**Indicates whether the driver is a bus driver*/
	bool isBusDriver;

	/**Pointer to the Driver object that is performing 'nosing'. (Current driver is 'yielding')*/
	const Driver *yieldingToDriver;

	/**Rection time of the driver*/
	unsigned int reactionTime;

	/**The origin of the driver's trip*/
	const Node *origin;

	/**The destination of the driver's trip*/
	const Node *destination;

	/**Perceived value of forward velocity*/
	FixedDelayed<double> *perceivedFwdVel;

	/**Perceived value of the acceleration*/
	FixedDelayed<double> *perceivedFwdAcc;

	/**Perceived value of the velocity of the vehicle in front*/
	FixedDelayed<double> *perceivedVelOfFwdCar;

	/**Perceived value acceleration of the vehicle in front*/
	FixedDelayed<double> *perceivedAccOfFwdCar;

	/**Perceived distance to the vehicle in front*/
	FixedDelayed<double> *perceivedDistToFwdCar;

	/**The perceived colour of the traffic signal*/
	FixedDelayed<TrafficColor> *perceivedTrafficColor;

	/**The perceived distance to the traffic signal*/
	FixedDelayed<double> *perceivedDistToTrafficSignal;

	/**
	 * Buffered data.
	 * These values are stored the double buffer because they are needed by other drivers.
	 */

	/**The driver's current lane*/
	Shared<const Lane*> currLane_;

	/**Indicates whether the driver is in an intersection*/
	Shared<bool> isInIntersection_;

	/**The current turning. If not NULL, the driver is on the turning*/
	Shared<const TurningPath *> currTurning_;

	/**The expected turning. If not NULL, the driver is approaching the intersection and will probably take this turning*/
	Shared<const TurningPath *> expectedTurning_;

	/**Represents the distance covered on the current way point, which is either a segment or a turning group (in metre)*/
	Shared<double> distCoveredOnCurrWayPt_;

	/**Represents the distance to be covered in order to reach the approaching intersection (in metre)*/
	Shared<double> distToIntersection_;

	/**Represents the lateral movement distance of the vehicle (in metre)*/
	Shared<double> latMovement_;

	/**Represents the forward speed of the vehicle (m/s)*/
	Shared<double> fwdVelocity_;

	/**Represents the lateral velocity of the vehicle (m/s)*/
	Shared<double> latVelocity_;

	/**Represents the acceleration of the vehicle (m/s^2)*/
	Shared<double> fwdAccel_;

	/**Indicates the lane changing move that the driver is going to make*/
	Shared<LaneChangeTo> turningDirection_;

	friend class DriverBehavior;
	friend class DriverMovement;

protected:
	/**Current position of the Driver*/
	Point currPos;

public:
	Driver(Person_ST *parent, MutexStrategy mtxStrat, DriverBehavior* behavior = nullptr, DriverMovement* movement = nullptr,
		Role<Person_ST>::Type roleType_ = Role<Person_ST>::RL_DRIVER, std::string roleName_ = "driver");
	virtual ~Driver();

	const Driver* getYieldingToDriver() const;
	void setYieldingToDriver(const Driver *driver);

	const Lane* getCurrLane() const;

	const Point& getCurrPosition() const;
	void setCurrPosition(Point currPosition);

	int getYieldingToInIntersection() const;
	void setYieldingToInIntersection(int);

	double getDistCoveredOnCurrWayPt() const;
	double getDistToIntersection() const;

	const double getFwdVelocity() const;
	const double getFwdAcceleration() const;

	/**Initialises the reaction time of the driver and the perception delays based on the reaction time*/
	void initReactionTime();

	/**
	 * Updates the information held by the current driver about a nearby driver
	 *
     * @param mFacet the movement facet
     */
	void handleUpdateRequest(MovementFacet *mFacet);

	/**
	 * Checks whether the driver is a bus driver
	 * 
     * @return true if the driver is driving a bus, else returns false
     */
	bool IsBusDriver();

	/**
	 * Calculates and returns the gap between the current driver and the given driver. The gap calculated
	 * is in terms of seconds (i.e. headway)
	 *
	 * NOTE: The driver in "front" and this driver may not be in the same lane (it could be in the left
	 * or right neighbour lane), but the two have to be in either the same segment or in adjoining segment downstream
	 *
     * @param front the driver in front
     *
	 * @return the calculated gap (headway)
     */
	double gapDistance(const Driver *front);

	/**
	 * Sets the reaction time of the driver to the one provided (in milli-seconds).
	 * Also resets the perception delays accordingly.
	 * 
     * @param time time in milli-seconds
     */
	void resetReactionTime(double time);

	/**
	 * Creates and initialises the movement and behaviour objects required for the Driver role,
	 * assigns them to a new driver and returns a pointer to the driver.
     *
	 * @param parent the person who will be taking up the requested role
     *
	 * @return the created role
     */
	virtual Role* clone(Person_ST* parent) const;

	/**
	 * Resets the driver parameters object
     *
	 * @param now the time frame for which the parameters are to be reset
     */
	virtual void make_frame_tick_params(timeslice now);

	/**
	 * Creates a vector of the subscription parameters and returns it
	 *
     * @return vector of the subscription parameters
     */
	virtual std::vector<BufferedBase *> getSubscriptionParams();

	/**
	 * Handler for the parent event from other agents
	 * 
     * @param eventId event identifier
     * @param ctxId context identifier
     * @param sender the sender of the event
     * @param args event arguments
     */
	virtual void onParentEvent(event::EventId eventId, event::Context ctxId, event::EventPublisher *sender, const event::EventArgs &args);

	/**
	 * Reroute around a blacklisted set of links.
	 *
     * @param blacklisted the blacklisted links
     */
	virtual void rerouteWithBlacklist(const std::vector<const Link *> &blacklisted);

	/**
	 * Sets a new path from the current segment to the destination.
	 * NOTE: Used only by road-runner. The vehicle will restart from the start of the current segment
     *
	 * @param path the new path
     */
	void rerouteWithPath(const std::vector<WayPoint>& path);

	const Vehicle* getVehicle() const
	{
		return vehicle;
	}

	Vehicle* getVehicle()
	{
		return vehicle;
	}

	void setVehicle(Vehicle *vehicle)
	{
		safe_delete_item(this->vehicle);
		this->vehicle = vehicle;
	}

	const double getVehicleLength() const
	{
		return vehicle->getLengthInM();
	}

	bool IsVehicleInLoadingQueue() const
	{
		return isVehicleInLoadingQueue;
	}

#ifndef SIMMOB_DISABLE_MPI
	//Serialization
	virtual void pack(PackageUtils& packageUtil);
	virtual void unpack(UnPackageUtils& unpackageUtil);

	virtual void packProxy(PackageUtils& packageUtil);
	virtual void unpackProxy(UnPackageUtils& unpackageUtil);
#endif

};
}
