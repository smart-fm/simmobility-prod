//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/utility.hpp>

#include "geospatial/Point2D.hpp"
#include "geospatial/streetdir/WayPoint.hpp"
#include "metrics/Length.hpp"
#include "util/LangHelpers.hpp"


namespace sim_mob
{

class Lane;
class Link;
class Point2D;
class RoadNetwork;
class RoadRunnerRegion;
class RoadSegment;
class Node;
class MultiNode;
class BusStop;
class Crossing;
class Signal;



/**
 * A singleton that provides street-directory information.
 *
 * Any agent (usually a Driver or Pedestrian) can call the getLane() method to find
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
enum TimeRange{
	MorningPeak=0,
	EveningPeak=1,
	OffPeak=2,
	Default=3,
	HighwayBias_Distance=4,
	HighwayBias_MorningPeak=5,
	HighwayBias_EveningPeak=6,
	HighwayBias_OffPeak=7,
	HighwayBias_Default=8,
	Random
};
class StreetDirectory : private boost::noncopyable
{
public:
	///Retrieve the current StreetDirectory instance. There can only be one StreetDirectory at any given time.
    static StreetDirectory& instance() {
        return instance_;
    }

    /**
     * Data structure returned by getLane().
     *
     * \c startIndex_ and \c endIndex_ are indices into the RoadSegment::polyline array.
     * If \c lane_ is not \c 0, then the area of interest is that stretch of the road segment.
     */
    struct LaneAndIndexPair {
        const Lane* lane_;
        size_t startIndex_;
        size_t endIndex_;

        explicit LaneAndIndexPair(const Lane* lane=nullptr, size_t startIndex=0, size_t endIndex=0)
        	: lane_(lane), startIndex_(startIndex), endIndex_(endIndex)
        {}
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
    struct RoadSegmentAndIndexPair {
        const RoadSegment* segment_;
        size_t startIndex_;
        size_t endIndex_;

        RoadSegmentAndIndexPair(const RoadSegment* segment, size_t startIndex, size_t endIndex)
        	: segment_(segment), startIndex_(startIndex), endIndex_(endIndex)
        {}
    };


    /**
     * Internal typedef to StreetDirectory representing:
     *   key:    The "vertex_name" property.
     *   value:  The Point2D representing that vertex's location. This point is not 100% accurate,
     *           and should be considered a rough guideline as to the vertex's location.
     * This is just a handy way of retrieving data "stored" at a given vertex.
     */
    typedef boost::property<boost::vertex_name_t, Point2D> VertexProperties;


    /**
     * Internal typedef to StreetDirectory representing:
     *   keys:   The "edge_weight" and "edge_name" properties.
     *   values: The Euclidean distance of this edge, and the WayPoint which represents this edge's traversal.
     * The distance is needed for our A* search, and the WayPoint is used when returning the actual results.
     */
    typedef boost::property<boost::edge_weight_t, double,
    		boost::property<boost::edge_name_t, WayPoint> > EdgeProperties;


    /**
     * Internal typedef to StreetDirectory representing:
     *   The actual graph, bound to its VertexProperties and EdgeProperties (see those comments for details on what they store).
     * You can use StreetDirectory::Graph to mean "a graph" in all contexts.
     */
    typedef boost::adjacency_list<boost::vecS,
                                  boost::vecS,
                                  boost::directedS,
                                  VertexProperties,
                                  EdgeProperties> Graph;

    /**
     * Internal typedef to StreetDirectory representing:
     *   A Vertex within our graph. Internally, these are defined as some kind of integer, but you should
     *   simply treat this as an identifying handle.
     * You can use StreetDirectory::Vertex to mean "a vertex" in all contexts.
     */
    typedef Graph::vertex_descriptor Vertex;

    /**
     * Internal typedef to StreetDirectory representing:
     *   An Edge within our graph. Internally, these are defined as pairs of integers (fromVertex, toVertex),
     *   but you should simply treat this as an identifying handle.
     * You can use StreetDirectory::Edge to mean "an edge" in all contexts.
     */
    typedef Graph::edge_descriptor Edge;


	//A return value for "Driving/WalkingVertex"
	struct VertexDesc {
		bool valid;    //Is this a valid struct? If false, treat as "null"
		Vertex source; //The outgoing Vertex (used for "source" master nodes).
		Vertex sink;   //The incoming Vertex (used for "sink" master nodes).

		VertexDesc(bool valid=false) : valid(valid), source(Vertex()), sink(Vertex()) {}
	};


    /**
     * Provides an implementation of the main StreetDirectory functionality. We define this as a public class
     *   to allow the testing of different implementations, rather than restricting ourselves to cpp-defined functionality.
     *
     * All methods in this class are protected, so that only the StreetDirectory can use them. Any sub-classes should define
     *   a public constructor, and leave the remainder of the fields as-is. This (should) allow proper hiding of internal details.
     */
    class Impl {
    protected:
        //Impl();  //Abstract?

		virtual std::pair<sim_mob::RoadRunnerRegion, bool> getRoadRunnerRegion(const sim_mob::RoadSegment* seg) = 0;

		virtual std::vector<const sim_mob::RoadSegment*> getSegmentsFromRegion(const sim_mob::RoadRunnerRegion& region) = 0;

		virtual const BusStop* getBusStop(const Point2D& position) const = 0;

		virtual const Node* getNode(const int id) const = 0;

        virtual LaneAndIndexPair getLane(const Point2D& position) const = 0;

        virtual const MultiNode* GetCrossingNode(const Crossing* cross) const = 0;

        virtual std::vector<RoadSegmentAndIndexPair> closestRoadSegments(const Point2D& point, centimeter_t halfWidth, centimeter_t halfHeight) const = 0;

        virtual const sim_mob::RoadSegment* getRoadSegment(const unsigned int id) = 0;

        //TODO: Does this work the way I want it to?
        friend class StreetDirectory;
    };


    /**
     * Provides an implementation of the StreetDirectory's shortest-path lookup functionality. See Impl's description for
     *  the general idea with these classes.
     */
    class ShortestPathImpl {
    protected:
    	//ShortestPathImpl();   //Abstract?

    	///Retrieve a Vertex based on a Node, BusStop, etc.. Flag in the return value is false to indicate failure.
    	virtual VertexDesc DrivingVertex(const Node& n) const = 0;
    	virtual VertexDesc WalkingVertex(const Node& n) const = 0;
    	virtual VertexDesc DrivingVertex(const BusStop& b) const = 0;
    	virtual VertexDesc WalkingVertex(const BusStop& b) const = 0;

    	//Meant to be used with the "DrivingVertex/WalkingVertex" functions.
        virtual std::vector<WayPoint> GetShortestDrivingPath(VertexDesc from, VertexDesc to, std::vector<const sim_mob::RoadSegment*> blacklist) const = 0;
        virtual std::vector<WayPoint> GetShortestWalkingPath(VertexDesc from, VertexDesc to) const = 0;

        virtual void updateEdgeProperty() = 0;

        virtual void printDrivingGraph(std::ostream& outFile) const = 0;
        virtual void printWalkingGraph(std::ostream& outFile) const = 0;

        //TODO: Does this work the way I want it to?
        friend class StreetDirectory;
    };



    /**
     * Retrieve the RoadRunnerRegion that a given RoadSegment passes through.
     * boolean value indicates success.
     * NOTE: We assume that a Segment is "inside" a Region if its midpoint is inside that Region, or
     *       if its from/to line intersects one of that Region's line segments.
     * If multiple Regions overlap on a RoadSegment, an arbitrary one will be chosen.
     */
    std::pair<sim_mob::RoadRunnerRegion, bool> getRoadRunnerRegion(const sim_mob::RoadSegment* seg);

    /**
     * Retrieve the list of RoadSegments that a given RoadRunnerRegion encompasses.
     * If multiple Regions overlap on a RoadSegment, that Segment will only be considered part of
     *   an arbitrary Region.
     */
    std::vector<const sim_mob::RoadSegment*> getSegmentsFromRegion(const sim_mob::RoadRunnerRegion& region);


    const BusStop* getBusStop(const Point2D& position) const;

	const Node* getNode(const int id) const;

    /**
     * Return the lane that contains the specified \c point; 0 if the point is outside the
     * road network.
     */
    LaneAndIndexPair getLane(const Point2D& point) const;


    /**
     * Return the MultiNode closest to this Crossing (may be null).
     */
    const MultiNode* GetCrossingNode(const Crossing* cross) const;


    /**
     * Return the RoadSegments within a rectangle centered around a point;
     * possibly empty list if rectangle is too small.
     */
    std::vector<RoadSegmentAndIndexPair> closestRoadSegments(const Point2D & point, centimeter_t halfWidth, centimeter_t halfHeight) const;


    /**
     * Return the Signal object, possibly none, located at the specified node.
     *
     * It is possible that the intersection, specified by \c node, is an unsignalized junction.
     * All road users must observe the highway code.
     */
    const Signal* signalAt(const sim_mob::Node& node) const;


	VertexDesc DrivingVertex(const sim_mob::Node& n) const;
	VertexDesc DrivingTimeVertex(const sim_mob::Node& n,sim_mob::TimeRange tr = sim_mob::MorningPeak,int random_graph_idx=0) const;
	VertexDesc WalkingVertex(const sim_mob::Node& n) const;
	VertexDesc DrivingVertex(const sim_mob::BusStop& b) const;
	VertexDesc WalkingVertex(const sim_mob::BusStop& b) const;


    /**
     * Return the distance-based shortest path to drive from one node to another. Performs a search (currently using
     *  the A* algorithm) from "fromNode" to "toNode".
     *
     * The function may return an empty array if \c toNode is not reachable from \c fromNode via
     * road-segments allocated for vehicle traffic.
     *
     * The resulting array contains only ROAD_SEGMENT and NODE WayPoint types. NODES at the beginning or end
     *  of the array can be ignored; NODES in the middle represent LaneConnectors.
     *
     * \todo
     * Although A* is pretty fast, we might want to cache the X most recent from/to pairs, since we're likely to have
     *   a lot of drivers asking for the same path information. This is trickier than one might think, since the
     *   StreetDirectory is used in parallel, so a shared structure will need to be designed carefully. ~Seth
     */
    std::vector<WayPoint> SearchShortestDrivingPath(VertexDesc from, VertexDesc to, std::vector<const sim_mob::RoadSegment*> blacklist=std::vector<const sim_mob::RoadSegment*>()) const;

    std::vector<WayPoint> SearchShortestDrivingTimePath(VertexDesc from,
    		VertexDesc to,
    		std::vector<const sim_mob::RoadSegment*> blacklist=std::vector<const sim_mob::RoadSegment*>(),
    		sim_mob::TimeRange tr=sim_mob::MorningPeak,
    		int random_graph_idx=0) const;

    /**
     * Return the distance-based shortest path to walk from one point to another.
     *
     * The function may return an empty array if \c toPoint is not reachable from \c fromPoint
     * via side-walks and crossings.
     *
     * The array contains only SIDE_WALK, CROSSING and NODE WayPoint types.
     *
     * It is possible that \c fromPoint or \c toPoint are off the road network (for example,
     * inside a building).  In that case, the first (last) wayPoint in the array would be a NODE
     * type if \c fromPoint (\c toPoint) is not within the road network; the NODE way-point would
     * be located in the road network.  The pedestrian is required to move from \c fromPoint to the
     * way-point (or from the way-point to \c toPoint) by some undefined mean.
     *
     * \todo
     * Adding of ad-hoc points is untested at the moment.
     */
    std::vector<WayPoint> SearchShortestWalkingPath(VertexDesc from, VertexDesc to) const;


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
    void init(const RoadNetwork& network, bool keepStats=false, centimeter_t gridWidth=100000, centimeter_t gridHeight=80000);


    /**
     * Register the Signal object with the StreetDirectory (to be invoked by the simulator kernel).
     */
    void registerSignal(const Signal& signal);


    /**
     * Print statistics collected on internal operations.
     *
     * Useful only if \c keepStats is \c true when \c init() was called.
     */
    void printStatistics() const;


    void updateDrivingMap();


    ///Print the Driving graph to LogOut(), in the old output format (out.txt)
    void printDrivingGraph(std::ostream& outFile);

    ///Print the Walking graph to LogOut(), in the old output format (out.txt)
    void printWalkingGraph(std::ostream& outFile);

    ///Return the Link associated with a given Node.
    ///Appears to associate Nodes based on an arbitrary choice (whether they appear as "start" nodes in a Link)
    ///However, since multiple Links share the same "start" Node, this isn't very realistic. Leaving in for now for
    /// compatibility purposes.
    const sim_mob::Link* getLinkLoc(const sim_mob::Node* node) const;

    ///Helper: find the nearest MultiNode to this Segment.
    static const MultiNode* FindNearestMultiNode(const RoadSegment* seg, const Crossing* cr);

    ///Return the Link associated with a given start and end Node.
    const sim_mob::Link* searchLink(const sim_mob::Node* start, const sim_mob::Node* end);

    /**
     * return a road segment from a aimsun-id
     * @param id is a given aimsun id
     * return a pointer to associated road segment
     */
    const sim_mob::RoadSegment* getRoadSegment(const unsigned int id);

private:
    //Helper: Find the point closest to the origin.
    static double GetShortestDistance(const Point2D& origin, const Point2D& p1, const Point2D& p2, const Point2D& p3, const Point2D& p4);


private:
    StreetDirectory() : pimpl_(nullptr), spImpl_(nullptr), sttpImpl_(nullptr)/*, stats_(nullptr)*/
    {}

    static StreetDirectory instance_;


private:
    ///Our current implementation of StreetDirectory functionality.
    Impl* pimpl_;

    ///Our current implementation of the shortest path searcher.
    ShortestPathImpl* spImpl_;

    // shortest travel time path
    ShortestPathImpl* sttpImpl_;

    ///The current set of StreetDirectoryStats
    //Stats* stats_;

    ///A lookup of all Signals in the RoadNetwork
    std::map<const Node*, const Signal*> signals_;

    ///A lookup of all Nodes/Links by a very specific criteria; see getLinkLoc() above.
    /// This criteria should definitely be re-examined.
	std::map<const sim_mob::Node*, const sim_mob::Link*> node_link_loc_cache;

	///A lookup of all Links by their start/end Nodes
	///key is <start, end>
    std::map< std::pair<const sim_mob::Node*, const sim_mob::Node*>, sim_mob::Link*> links_by_node;

};


}
