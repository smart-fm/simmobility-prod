//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
#include <map>
#include <vector>

#include "buffering/Shared.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {
class Lane;

/**
 * base class for a sensor.
 * Sensor can be a loop detector, camera or any other device which collects
 * information about traffic.
 *
 * \author Harish Loganathan
 */
class Sensor : public Agent {
public:
    // Forward declaration.  Definition is below.
    struct CountAndTimePair;

    Sensor(MutexStrategy const & mutexStrategy) : Agent(mutexStrategy) {}
    virtual ~Sensor() {}

public:
	//Sensors are non-spatial in nature.
	virtual bool isNonspatial() { return true; }
    virtual void load(const std::map<std::string, std::string>& configProps) = 0;

    /**
     * Return the CountAndTimePair for the specified \c lane.
     */
    CountAndTimePair const & getCountAndTimePair(Lane const & lane) const;

    /**
     * reset the sensor
     */
    virtual void reset() = 0;

protected:
	virtual bool frame_init(timeslice now) { return true; }
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now) {}
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList);

	std::map<Lane const *, Shared<CountAndTimePair> *> data_;
};

struct Sensor::CountAndTimePair
{
    /**
     * The number of vehicles that has crossed the sensor
     * the last call to reset().
     */
    size_t vehicleCount;

    /**
     * The total amount of time in milli-seconds during which no vehicle is hovering over the
     * sensor during the cycle since the last call to reset().
     *
     * Note that this is the "space-time" value, and not the "on-time" which is the total
     * amount of time during which at least one vehicle is hovering over the sensor
     */
    unsigned int spaceTimeInMilliSeconds;

    /** \cond ignoreLoopDetectorEntityInnards -- Start of block to be ignored by doxygen.  */
    CountAndTimePair() : vehicleCount(0), spaceTimeInMilliSeconds(0) {}
    /** \endcond ignoreLoopDetectorEntityInnards -- End of block to be ignored by doxygen.  */
};

}
