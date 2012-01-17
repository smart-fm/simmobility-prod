/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/utility.hpp>
#include "metrics/Length.hpp"

namespace sim_mob
{

class Agent;
class Point2D;
class Lane;

/**
 * A singleton that can locate agents/entities within any rectangle.
 *
 * \author LIM Fung Chai
 * \author Seth N. Hetu
 *
 * To locate nearby agents, calculate 2 points to form the search rectangle before calling
 * agentsInRect(). 
 *   \code
 *   Point2D p1 = ...;
 *   Point2D p2 = ...;
 *   const std::vector<const Agent*> nearby_agents = AuraManager::instance().agentsInRect(p1, p2);
 *   \endcode
 *
 * Alternatively if an agent is located on a lane, it can call nearbyAgents(), passing in its
 * current position and lane, and the distance forward and behind its current position.
 *
 * Both methods return an array of Agent's.  It is the responsibility of the caller to determine
 * the type (i.e. the current role) of each agent.
 */
class AuraManager : private boost::noncopyable
{
public:
    static AuraManager &
    instance()
    {
        return instance_;
    }

    /**
     * Called every frame, this method builds a spatial index of the positions of all agents.
     *
     * This method should be called after all the agents have calculated their new positions
     * and (if double-buffering data types are used) after the new positions are published.
     */
    void update(frame_t /* frameNumber */);

    /**
     * Return a collection of agents that are located in the axially-aligned rectangle.
     *   \param lowerLeft The lower left corner of the axially-aligned search rectangle.
     *   \param upperRight The upper right corner of the axially-aligned search rectangle.
     *
     * The caller is responsible to determine the "type" of each agent in the returned array.
     */
    std::vector<Agent const *>
    agentsInRect(Point2D const & lowerLeft, Point2D const & upperRight) const;

    /**
     * Return a collection of agents that are on the left, right, front, and back of the specified
     * position.
     *   \param position The center of the search rectangle.
     *   \param lane The lane 
     *   \param distanceInFront The forward distance of the search rectangle.
     *   \param distanceBehind The back
     *
     * This query is designed for Driver/Vehicle agents.  It calculates the search rectangle
     * based on \c position, \c lane, \c distanceInFront, and \c distanceBehind.  \c position
     * should be the current location of the Driver agent and is within the boundary of \c lane.
     * The search rectangle is (not necessarily symmetrically) centered around \c position.
     * It includes the adjacent lanes on the left and right of \c position (effectively of \c
     * lane as well).  If \c lane is the leftmost lane and/or is the rightmost lane, the search
     * rectangle extends 300 centimeters to include the sidewalk or the road segment of the reverse
     * direction.
     */
    std::vector<Agent const *>
    nearbyAgents(Point2D const & position, Lane const & lane,
                 centimeter_t distanceInFront, centimeter_t distanceBehind) const;

    /**
     * Initialize the AuraManager object (to be invoked by the simulator kernel).
     *
     * An internal class is used to collect statistics to measure the efficiency of the
     * algorithm that was implemented.  A Release version should disabled this collection.
     *   \param keepStats Keep statistics on internal operations if true.
     */
    void
    init(bool keepStats = false);

    /**
     * Print statistics collected on internal operationss.
     *
     * Useful only if \c keepStats is \c true when \c init() was called.
     */
    void
    printStatistics() const;

private:
    AuraManager()
      : pimpl_(0)
      , stats_(0)
    {
    }

    // No need to define the dtor.

    static AuraManager instance_;

    // Using the pimple design pattern.  Impl is defined in the source file.
    class Impl;
    Impl* pimpl_;
    friend class Impl;  // allow access to stats_.

    class Stats;
    Stats* stats_;
};

}
