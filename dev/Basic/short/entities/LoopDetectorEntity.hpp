//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <boost/unordered_set.hpp>

#include "buffering/Vector2D.hpp"
#include "entities/Sensor.hpp"
#include "entities/signal/Signal.hpp"
#include "entities/vehicle/Vehicle.hpp"
#include "geospatial/network/Lane.hpp"

namespace sim_mob
{

/**
 * This class models an Axially Aligned Bounding Box.  That is, it is a rectangle that is 
 * parallel to both the X- and Y- axes.  As a bounding box, it is the smallest rectangle that 
 * would contain something.
 */
struct AABB
{
	Point lowerLeft_;
	Point upperRight_;

	// Expand this AABB to include another AABB.  That is, this = this union <another>.
	// union is a keyword in C++, so the method name is united.
	void united(AABB const &another);
};

/**
 * The LoopDetectorEntity is an entity that models all the loop-detectors located just before the
 * stop-line on all vehicle lanes approaching a Signal.
 *
 * \author LIM Fung Chai
 *
 * Each LoopDetectorEntity object must run at the same rate as the Driver objects.  This
 * implementation assumes this is so.
 *
 * For each loop detector, the entity counts the number of vehicles crossing the detector and
 * the total amount of time that no vehicle is hovering over the detector.  The time attribute
 * is known as the total "space-time".
 *
 * The LoopDetectorEntity expects its Signal object to reset the vehicle count and space-time
 * attributes periodically via the reset() method.
 *
 * There will definitely be a race condition with respect to calls to the update() and reset()
 * methods.  Although the LoopDetectorEntity objects run at a more frequent rate than the Signal
 * objects, there will be frames during which both objects run.  Because the objects are managed
 * by different Worker objects, the order in which they run is not deterministic.  The vehicle
 * count and space-time values may be different if the order of the calls to the update() and
 * reset() methods is different.  From a modeling point of view, this difference will not be
 * significant.  However it does introduce some randomness into each simulation run.
 */
class LoopDetectorEntity : public sim_mob::Sensor
{
public:
	LoopDetectorEntity(MutexStrategy const & mutexStrategy) : Sensor(mutexStrategy), pimpl_(0)
	{
	}
	virtual ~LoopDetectorEntity();

	/**
	 * @return true, as Loop detectors are non-spatial in nature.
	 */
	virtual bool isNonspatial()
	{
		return true;
	}

	void init(Signal const & signal);

	virtual void load(const std::map<std::string, std::string>& configProps)
	{
	}

	/**
	 * Called by the Signal object at the end of its cycle to reset all CountAndTimePair.
	 */
	virtual void reset();

	/**
	 * Called by the Signal object at the end of its cycle to reset the CountAndTimePair
	 * for the specified \c lane.
	 */
	void reset(Lane const & lane);

protected:
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	class Impl;
	Impl* pimpl_;
	friend class Impl;
};

////////////////////////////////////////////////////////////////////////////////////////////
// LoopDetector
////////////////////////////////////////////////////////////////////////////////////////////

// The LoopDetector class actually models a monitoring area surrounding and centered at a loop
// detector and oriented in the same direction as the loop detector.  The width of the area is that
// of the lane, and the loop-detector's width is assumed to be a few centimeters narrower than the
// lane's width.  The length of the monitoring area include an area before and after the loop
// detector.  This is the outer area, while the inner area matches the area occupied by the loop
// detector.  See the comment in LoopDetectorEntity::Impl::Impl() below.
//
// Since most loop detectors are not aligned to the X- and Y- axes, the monitoring area is a 2D
// Object-oriented Bounding Box (OBB).  The OBB implementation has a center, 2 normalized
// orientations and 2 scalars representing the length and width of the OBB.  <orientationL_>
// represents the orientation of the OBB along the length of the monitoring area.  <innerLength_>
// is exactly half the length of the loop detector while 2 * <outerLength_> represents the full
// length of the OBB.  That is, the length of the outer area before (or after) the loop detector
// is outerLength_ - innerLength_.

class LoopDetector : private boost::noncopyable
{
public:
	LoopDetector(Lane const *lane, meter_t innerLength, meter_t outerLength, Shared<Sensor::CountAndTimePair> &pair);

	// Check if any vehicle in the <vehicles> list is hovering over the loop detector.  If there
	// is no vehicle, then set <vehicle_> to 0 and increment the space-time attribute.  If a
	// vehicle (or part of) is hovering over the loop detector, set <vehicle_> to it; increment
	// the vehicle count whenever <vehicle_> changes.
	bool check(boost::unordered_set<Vehicle const *> &vehicles);

	/**
	 * @return the AABB that would contain the monitoring area of this object.
	 */
	AABB getAABB() const;

	const Vehicle* vehicle() const
	{
		return vehicle_;
	}

	/**
	 * Called by the Signal object at the end of its cycle to reset the CountAndTimePair
	 */
	void reset()
	{
		request_to_reset_ = true;
	}

private:
	Point center_;
	Vector2D<double> orientationL_; // orientation of the OBB along its length.
	Vector2D<double> orientationW_; // orientation of the OBB along its width.
	meter_t width_;
	meter_t innerLength_;
	meter_t outerLength_;

	unsigned int timeStepInMilliSeconds_; // The loop detector entity runs at this rate.
	bool request_to_reset_; // See the comment in check().
	Shared<Sensor::CountAndTimePair> & countAndTimePair_;
	const Vehicle *vehicle_; // Current vehicle, if any, that is hovering over the loop detector.

private:
	// Return true if any part of <vehicle> is hovering over the loop detector.
	bool check(Vehicle const &vehicle);

	void incrementSpaceTime()
	{
		Sensor::CountAndTimePair pair(countAndTimePair_);
		pair.spaceTimeInMilliSeconds += timeStepInMilliSeconds_;
		countAndTimePair_.set(pair);
	}

	void incrementVehicleCount()
	{
		Sensor::CountAndTimePair pair(countAndTimePair_);
		++pair.vehicleCount;
		countAndTimePair_.set(pair);
	}
};
}
