/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "entities/Agent.hpp"
#include "metrics/Length.hpp"
#include "buffering/Shared.hpp"

namespace sim_mob
{

class Node;
class Lane;
class Signal;

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
class LoopDetectorEntity : public Agent
{
public:
    // Forward declaration.  Definition is below.
    struct CountAndTimePair;

public:
    LoopDetectorEntity(Signal const & signal, MutexStrategy const & mutexStrategy);
    ~LoopDetectorEntity();

    virtual bool update(frame_t frameNumber);

    virtual void output(frame_t frameNumber) {}

    Node const & getNode() const { return node_; }

    void init(Signal const & signal);

    /**
     * Return the CountAndTimePair for the specified \c lane.
     */
    CountAndTimePair const & getCountAndTimePair(Lane const & lane) const;

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
    virtual void buildSubscriptionList();

private:
    Node const & node_;
    std::map<Lane const *, Shared<CountAndTimePair> *> data_;

    class Impl;
    Impl* pimpl_;
    friend class Impl;
};

struct LoopDetectorEntity::CountAndTimePair
{
    /**
     * The number of vehicles that has crossed the loop detector during the cycle since
     * the last call to reset().
     */
    size_t vehicleCount;

    /**
     * The total amount of time in milli-seconds during which no vehicle is hovering over the
     * loop detector during the cycle since the last call to reset().
     *
     * Note that this is the "space-time" value, and not the "on-time" which is the total
     * amount of time during which at least one vehicle is hovering over the loop detector.
     */
    unsigned int spaceTimeInMilliSeconds;

    /** \cond ignoreLoopDetectorEntityInnards -- Start of block to be ignored by doxygen.  */
    CountAndTimePair() : vehicleCount(0), spaceTimeInMilliSeconds(0) {}
    /** \endcond ignoreLoopDetectorEntityInnards -- End of block to be ignored by doxygen.  */
};

}
