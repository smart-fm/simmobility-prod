//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <vector>

#include "buffering/Shared.hpp"
#include "entities/Agent.hpp"
#include "entities/UpdateParams.hpp"
#include "metrics/Frame.hpp"

namespace sim_mob {
class Lane;

/**
 * Base class for any sensor.
 * A sensor can be a loop detector, camera or any other device which collects
 * information about traffic.
 *
 * \author Harish Loganathan
 */
class Sensor : public Agent {
public:
    Sensor(MutexStrategy const & mutexStrategy) : Agent(mutexStrategy) {}
    virtual ~Sensor();

    // Forward declaration.  Definition below.
    struct CountAndTimePair;

	virtual bool isNonspatial() { return true; } //Sensors are non-spatial in nature.

    /**
     * Return the CountAndTimePair for the specified \c lane.
     * @param lane constant reference to lane to get count from
     * @returns constant reference to CountAndTimePair
     */
    CountAndTimePair const& getCountAndTimePair(Lane const& lane) const;
    
    /**
     * Returns the entire map of Lane vs CountAndTimePair
     */
    const std::map<Lane const *, Shared<CountAndTimePair> *>& getCountAndTimePairMap() const
    {
      return data;
    }

    /**
     * reset the sensor
     */
    virtual void reset() = 0;

protected:
	virtual bool frame_init(timeslice now) { return true; }
	virtual Entity::UpdateStatus frame_tick(timeslice now) { return UpdateStatus(UpdateStatus::RS_CONTINUE); }
	virtual void frame_output(timeslice now) {}
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	/** map to store lane-wise counts and time*/
	std::map<Lane const *, Shared<CountAndTimePair> *> data;
};

/**
 * simple struct for vehicle count and time pair
 */
struct Sensor::CountAndTimePair
{
public:
    /**
     * The number of vehicles that has crossed the sensor
     * the last call to reset().
     */
    size_t vehicleCount;

    /**
     * The total amount of time in milli-seconds during which no vehicle is
     * hovering over the sensor during the cycle since the last call to reset().
     *
     * Note that this is the "space-time" value, and not the "on-time" which is
     * the total amount of time during which at least one vehicle is hovering
     * over the sensor
     */
    unsigned int spaceTimeInMilliSeconds;

    CountAndTimePair() : vehicleCount(0), spaceTimeInMilliSeconds(0) {}
};

}
