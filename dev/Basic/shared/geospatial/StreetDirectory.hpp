/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"
#include "util/LangHelpers.hpp"

#include <map>
#include <vector>

#include <boost/utility.hpp>
#include "metrics/Length.hpp"

//Pull in our typedefs
#include "entities/signal_transitional.hpp"

namespace sim_mob
{

class Lane;
class Point2D;
class RoadNetwork;
class RoadSegment;
class Node;
class BusStop;
class Crossing;

/**
 * A point in the shortest path returned by StreetDirectory::shortestDrivingPath() and StreetDirectory::shortestWalkingPath().
 *
 * \author LIM Fung Chai
 * \author Seth N. Hetu
 *
 * It is a "variant" type because it can be one of various types.  You should check the \c type_
 * member to determine which pointer is appropriate.  For example,
 *   \code
 *   std::vector<WayPoint> path_;
 *   size_t path_index_;
 *   ...
 *
 *   Point2D from = ...;
 *   Point2D to = ...;
 *   path_ = StreetDirectory::instance().shortestWalkingPath(from, to);
 *   if (path_.empty())  // <to> is not reachable from <from> via side-walks and crossings.
 *   {
 *       error_message(...);
 *       return;
 *   }
 *   path_index_ = 0;
 *   ...
 *
 *   const WayPoint wp = path_[path_index_];
 *   if (WayPoint::SIDE_WALK == wp.type_)
 *   {
 *      walk_on(wp.lane_);  // walk on the side walk
 *   }
 *   else if (WayPoint::CROSSING == wp.type_)
 *   {
 *      walk_on(wp.crossing_);  // walk on the crossing to cross the road.
 *   }
 *   else ...
 *
 *   ++path_index_;
 *   ...
 *   \endcode
 *
 * A WayPoint is a point and the BUS_STOP and NODE types reflect that notion.  For the other types,
 * the WayPoint is an "abbreviation" of 2 points -- from the start point of the road-segment,
 * side-walk, or crossing to their end point.
 *
 * The shortest path may include a BUS_STOP WayPoint.  This would be useful for buses; such
 * WayPoints mark out the points they need to stop for passengers to board and alight.  Car
 * drivers can skip these way-points.  Similarly pedestrians may skip BUS_STOP way-points unless
 * the way-point is their destination.  The WayPoint before and after the BUS_STOP WayPoint in the
 * shortest path may refer to the same road-segment (for vehicles) or side-walk (for pedestrians).
 *
 * Refer to StreetDirectory::shortestWalkingPath() for a description of the NODE type.
 *
 * Future extensions would include underpasses and overhead bridges types.
 *
 * \sa StreetDirectory::shortestDrivingPath()
 * \sa StreetDirectory::shortestWalkingPath()
 */
struct WayPoint
{
    enum
    {
        SIDE_WALK, //!< waypoint is a side walk; lane_ points to a (side-walk) Lane object.
        ROAD_SEGMENT, //!< waypoint is a road-segment; roadSegment_ points to a RoadSegment object.
        BUS_STOP, //!< waypoint is a bus-stop; busStop_ points to a BusStop object.
        CROSSING, //!< waypoint is a crossing; crossing_ points to a Crossing object.
        NODE, //!< waypoint is node; node_ points to a Node object.
        INVALID  //!< waypoint is invalid, none of the pointers are valid.
    } type_;
    union
    {
        Lane const * lane_;
        RoadSegment const * roadSegment_;
        BusStop const * busStop_;
        Crossing const * crossing_;
        Node const * node_;
    };

    bool directionReverse;

    /** \cond ignoreStreetDirectoryInnards -- Start of block to be ignored by doxygen.  */
    // Used only by the StreetDirectory.  No need to expose them in the doxygen pages.
    WayPoint() : type_(INVALID) {}
    explicit WayPoint(Lane const * lane) : type_(SIDE_WALK), lane_(lane),directionReverse(false) {}
    explicit WayPoint(RoadSegment const * road) : type_(ROAD_SEGMENT), roadSegment_(road),directionReverse(false) {}
    explicit WayPoint(BusStop const * stop) : type_(BUS_STOP), busStop_(stop),directionReverse(false) {}
    explicit WayPoint(Crossing const * crossing) : type_(CROSSING), crossing_(crossing),directionReverse(false) {}
    explicit WayPoint(Node const * node) : type_(NODE), node_(node),directionReverse(false) {}
    /** \endcond ignoreStreetDirectoryInnards -- End of block to be ignored by doxygen.  */
};

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
     * Return the distance-based shortest path to drive from one node to another.
     *
     * The fucntion may return an empty array if \c toNode is not reachable from \c fromNode via
     * road-segments allocated for vehicle traffic.
     *
     * The array contains only ROAD_SEGMENT and BUS_STOP WayPoint types.
     */
    std::vector<WayPoint>
    shortestDrivingPath(Node const & fromNode, Node const & toNode) const;

    std::vector<WayPoint>
    GetShortestDrivingPath(Node const & fromNode, Node const & toNode) const;

    /**
     * Return the distance-based shortest path to walk from one point to another.
     *
     * The function may return an empty array if \c toPoint is not reachable from \c fromPoint
     * via side-walks and crossings.
     *
     * The array contains only SIDE_WALK, BUS_STOP, CROSSING and NODE WayPoint types.
     *
     * It is possible that \c fromPoint or \c toPoint are off the road network (for example,
     * inside a building).  In that case, the first (last) wayPoint in the array would be a NODE
     * type if \c fromPoint (\c toPoint) is not within the road network; the NODE way-point would
     * be located in the road network.  The pedestrian is required to move from \c fromPoint to the
     * way-point (or from the way-point to \c toPoint) by some undefined mean.
     */
    std::vector<WayPoint>
    shortestWalkingPath(Point2D const & fromPoint, Point2D const & toPoint) const;

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

    void updateDrivingMap();

    //Print using the old output format.
    void printDrivingGraph();
    void printWalkingGraph();

private:
    StreetDirectory()
      : pimpl_(nullptr)
      , spImpl_(nullptr)
      , stats_(nullptr)
    {
    }

    // No need to define the dtor.

    static StreetDirectory instance_;

    // Using the pimple design pattern.  Impl is defined in the source file.
    class Impl;
    Impl* pimpl_;
    friend class Impl;  // allow access to stats_.

    // A private class to calculate distance-based shortest paths for drivers and pedestrians.
    class ShortestPathImpl;
    ShortestPathImpl* spImpl_;

    class Stats;
    Stats* stats_;

    std::map<const Node *, Signal const *> signals_;
};


}
