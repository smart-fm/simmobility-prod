//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "entities/Sensor.hpp"
#include "entities/signal/Signal.hpp"
#include "geospatial/Lane.hpp"

namespace sim_mob
{

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
 * reset() methods is different.  From a modelling point of view, this difference will not be
 * significant.  However it does introduce some randomness into each simulation run.
 */
class LoopDetectorEntity : public sim_mob::Sensor
{
public:
    LoopDetectorEntity(MutexStrategy const & mutexStrategy) : Agent(mutexStrategy), pimpl_(0) {}
    virtual ~LoopDetectorEntity();

protected:
	virtual bool frame_init(timeslice now) { return true; }
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now) {}

public:
	//Loop detectors are non-spatial in nature.
	virtual bool isNonspatial() { return true; }

    void init(Signal const & signal);

    //May want to implement later.
    virtual void load(const std::map<std::string, std::string>& configProps) {}

    /**
     * Called by the Signal object at the end of its cycle to reset all CountAndTimePair.
     */
    void reset();

    /**
     * Called by the Signal object at the end of its cycle to reset the CountAndTimePair
     * for the specified \c lane.
     */
    void reset(Lane const & lane);

protected:
    class Impl;
    Impl* pimpl_;
    friend class Impl;
};
}
