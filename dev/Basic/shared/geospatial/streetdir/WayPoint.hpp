//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "util/LangHelpers.hpp"


namespace sim_mob
{

class Lane;
class RoadSegment;
class Node;
class BusStop;
class Crossing;
class Point2D;


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
 * Previously, a WayPoint represented an actual "Point". Now, however, a WayPoint actually represents a portion of a path.
 * In particular, the types SIDE_WALK and ROAD_SEGMENT represent direct, observable edges; that is, they represent
 * SideWalk lanes and RoadSegments respectively. Similarly, the CROSSING type represents a Pedestrian crossing (from/to a
 * given RoadSegment, in an arbitrary order). The BUS_STOP type, although not used at the moment, would represent
 * an invisible link from a RoadSegment or Node to the actual BusStop's location. The NODE type usually represents
 * movement within "parts" of a Node. Consider, for example, that a UniNode may actually have two vertices in the graph:
 * the roadway moving in one direciton, and the roadway moving reverse to that direction. In this case, a NODE WayPoint would
 * be used to move from the UniNode itself to one of these two vertices (if the vehicle starts on that Node exactly). In addition,
 * the NODE type is also currently used to store LaneConnectors.
 *
 * Agents should skip any type which does not matter to them. For example, Drivers in the medium term can safely skip all NODE
 * types, since medium-term Drivers perform no intersection-level driving.
 *
 * Refer to StreetDirectory::shortestWalkingPath() for a description of the NODE type.
 *
 * \todo
 * This class is beginning to stretch the limits of our union-in-a-struct abstraction. In particular, it is far too easy to
 * take a SIDE_WALK WayPoint and attempt to access its roadSegment_ data member. Perhaps we can start with getters/setters
 * and make the unionized type private? That way we can throw an exception if the wrong access pattern is used.  ~Seth
 *
 * \todo
 * As an example of the possible cleanup, "nonspatial" should actually be given x/y positional information, but we can't do that
 * now (because we are using value types)
 *
 * \sa StreetDirectory::shortestDrivingPath()
 * \sa StreetDirectory::shortestWalkingPath()
 */
struct WayPoint
{
    enum {
        SIDE_WALK,    //!< WayPoint is a side walk; lane_ points to a (side-walk) Lane object.
        ROAD_SEGMENT, //!< WayPoint is a road-segment; roadSegment_ points to a RoadSegment object.
        BUS_STOP,     //!< WayPoint is a bus-stop; busStop_ points to a BusStop object.
        CROSSING,     //!< WayPoint is a crossing; crossing_ points to a Crossing object.
        NODE,         //!< WayPoint is node; node_ points to a Node object.
        NONSPATIAL,   //!< WayPoint has no associated data; it must simply be "traversed".
        INVALID       //!< WayPoint is invalid, none of the pointers are valid.
    } type_;

    union {
        const Lane* lane_;
        const RoadSegment* roadSegment_;
        const BusStop* busStop_;
        const Crossing* crossing_;
        const Node* node_;
    };

    //NOTE: These two functions have been removed; they're not robust and make the WayPoint's union even
    //      more likely to cause an error. New solution later. ~Seth.
    //Point2D location();
    //int getID();

    ///If true, this indicates the the current WayPoint represents a reverse traversal of the given type.
    ///  This usually means that a RoadSegment should be traversed in reverse. Note that this only has meaning
    ///   in the walking graph.
    bool directionReverse;


    /** \cond ignoreStreetDirectoryInnards -- Start of block to be ignored by doxygen.  */
    // Used only by the StreetDirectory.  No need to expose them in the doxygen pages.
    WayPoint() : type_(INVALID), directionReverse(false), node_(nullptr){}
    explicit WayPoint(Lane const * lane) : type_(SIDE_WALK), lane_(lane),directionReverse(false) {}
    explicit WayPoint(RoadSegment const * road) : type_(ROAD_SEGMENT), roadSegment_(road),directionReverse(false) {}
    explicit WayPoint(BusStop const * stop) : type_(BUS_STOP), busStop_(stop),directionReverse(false) {}
    explicit WayPoint(Crossing const * crossing) : type_(CROSSING), crossing_(crossing),directionReverse(false) {}
    explicit WayPoint(Node const * node) : type_(NODE), node_(node),directionReverse(false) {}
    explicit WayPoint(const Point2D& ignored) : type_(NONSPATIAL), node_(nullptr),directionReverse(false) {}
    /** \endcond ignoreStreetDirectoryInnards -- End of block to be ignored by doxygen.  */
};


}
