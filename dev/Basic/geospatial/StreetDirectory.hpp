/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include "Length.hpp"

namespace sim_mob
{

class Lane;
class Point2D;
class RoadNetwork;
class Node;
class Signal;

/**
 * A singleton that provides street-directory information.
 *
 * Any agent (usually a driver agent or vehicle entity) can call the getLane() method to find
 * which lane it is currently located.
 *   \code
 *   StreetDirectory::LaneAndIndexPair pair = StreetDirectory::instance().getLane(myPosition);
 *   if (pair.lane_ != 0)
 *   {
 *       ...
 *   }
 *   \endcode
 * The test on <tt>pair.lane</tt> is necessary because the agent may not be positioned on
 * any portion of the road network.
 *
 * If an agent (usually a pedestrian) knows it is not on any road, it can call
 * closestRoadSegments().
 * It should call this with an
 * initial small search rectangle, expanding the search area if the return list is empty.  The
 * following example starts with a initial search area of 5 meter square, increasing the sides
 * of the search area by 2 meters until the StreetDirectory returns some road segments.
 *   \code
 *   centimeter_t side = 500;
 *   std::vector<StreetDirectory::RoadSegmentAndIndexPair> segments;
 *   do
 *   {
 *       segments = StreetDirectory::instance().closestRoadSegments(myPosition, side, side);
 *       side += 200;
 *   }
 *   while (segments.empty());
 *   \endcode
 *
 *   \sa LaneAndIndexPair
 *   \sa RoadSegmentAndIndexPair
 */
class StreetDirectory : private boost::noncopyable
{
public:
    static StreetDirectory&
    instance()
    {
        return instance_;
    }

    /**
     * Data structure returned by getLane().
     *
     * \c startIndex_ and \c endIndex_ are indices into the RoadSegment::polyline array.
     * If \c lane_ is not \c 0, then the area of interest is that stretch of the road segment.
     */
    struct LaneAndIndexPair
    {
        Lane const * lane_;
        size_t startIndex_;
        size_t endIndex_;

        LaneAndIndexPair(Lane const * lane = 0, size_t startIndex = 0, size_t endIndex = 0)
          : lane_(lane), startIndex_(startIndex), endIndex_(endIndex)
        {
        }
    };

    /**
     * Data structure returned by closestRoadSegments().
     *
     * \c startIndex_ and \c endIndex_ are indices into the RoadSegment::polyline array.
     * If \c segment_ is not \c 0, then the area of interest is that stretch of the road segment.
     * If the search area is too large, the list returned by closestRoadSegments() may contain
     * the same road segment several times.  They are different stretches of the road segment
     * as reflected by different \c startIndex and \c endIndex values.
     */
    struct RoadSegmentAndIndexPair
    {
        RoadSegment const * segment_;
        size_t startIndex_;
        size_t endIndex_;

        RoadSegmentAndIndexPair(RoadSegment const * segment, size_t startIndex, size_t endIndex)
          : segment_(segment), startIndex_(startIndex), endIndex_(endIndex)
        {
        }
    };

    /**
     * Return the lane that contains the specified \c point; 0 if the point is outside the
     * road network.
     *
     */
    LaneAndIndexPair
    getLane(Point2D const & point) const;

    /**
     * Return the RoadSegments within a rectangle centered around a point;
     * possibly empty list if rectangle is too small.
     *
     */
    std::vector<RoadSegmentAndIndexPair>
    closestRoadSegments(Point2D const & point, centimeter_t halfWidth, centimeter_t halfHeight) const;

    /**
     * Return the Signal object, possibly none, located at the specified node.
     *
     * It is possible that the intersection, specified by \c node, is an unsignalized junction.
     * All road users must observe the highway code.
     */
    Signal const *
    signalAt(Node const & node) const;

    /**
     * Initialize the StreetDirectory object (to be invoked by the simulator kernel).
     *
     * The StreetDirectory partitions the road network into a rectangular grid for fast lookup.
     * This method is used to initialize the spatial index with the loaded road network.
     *   \param network The road network that was loaded into the simulator.
     *   \param keepStats Keep statistics on internal operations if true.
     *   \param gridWidth Width of the grid, default of 1000 meters.
     *   \param gridHeight Height of the grid, default of 800 meters.
     * In future version, the first parameter, \c network, may be removed; the StreetDirectory
     * singleton will load the road network on demand during calls to getLane() and
     * closestRoadSegments().
     */
    void
    init(RoadNetwork const & network, bool keepStats = false,
         centimeter_t gridWidth = 100000, centimeter_t gridHeight = 80000);

    /**
     * Register the Signal object with the StreetDirectory (to be invoked by the simulator kernel).
     */
    void
    registerSignal(Signal const & signal);

    /**
     * Print statistics collected on internal operationss.
     *
     * Useful only if \c keepStats is \c true when \c init() was called.
     */
    void
    printStatistics() const;

private:
    StreetDirectory()
      : pimpl_(nullptr)
      , stats_(nullptr)
    {
    }

    // No need to define the dtor.

    static StreetDirectory instance_;

    // Using the pimple design pattern.  Impl is defined in the source file.
    class Impl;
    Impl* pimpl_;
    friend class Impl;  // allow access to stats_.

    class Stats;
    Stats* stats_;

    boost::unordered_map<const Node *, Signal const *> signals_;
};

inline Signal const *
StreetDirectory::signalAt(Node const & node)
const
{
    boost::unordered_map<const Node *, Signal const *>::const_iterator iter = signals_.find(&node);
    if (signals_.end() == iter)
        return 0;
    return iter->second;
}

}
