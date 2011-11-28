/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <vector>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "Lane.hpp"
#include "RoadNetwork.hpp"
#include "StreetDirectory.hpp"
#include "buffering/Vector2D.hpp"
#include "entities/Signal.hpp"
#include "BusStop.hpp"
#include "Crossing.hpp"
#include "ZebraCrossing.hpp"
#include "UniNode.hpp"

namespace sim_mob
{

/* static */ StreetDirectory StreetDirectory::instance_;

/** \cond ignoreStreetDirectoryInnards -- Start of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// StreetDirectory::Stats
////////////////////////////////////////////////////////////////////////////////////////////

struct StreetDirectory::Stats : private boost::noncopyable
{
    void
    printStatistics() const;
};

void
StreetDirectory::Stats::printStatistics() const
{
    std::cout << "StreetDirectory::Stats not implemented yet" << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////
// StreetDirectory::Impl
////////////////////////////////////////////////////////////////////////////////////////////

class StreetDirectory::Impl : private boost::noncopyable
{
public:
    Impl(RoadNetwork const & network, centimeter_t gridWidth, centimeter_t gridHeight);

    // No need to define the dtor.

    LaneAndIndexPair
    getLane(Point2D const & position) const;

    std::vector<RoadSegmentAndIndexPair>
    closestRoadSegments(Point2D const & point, centimeter_t halfWidth, centimeter_t halfHeight) const;

private:
    // Partition the road network into a rectangular grid.
    void
    partition(RoadNetwork const & network);

    // Partition the list of road segments into a rectangular grid.
    // If <isForward> is true, then vehicle traffic on the road segments flows from their start
    // to end points.
    void
    partition(std::vector<RoadSegment*> const & segments, bool isForward);

    // Partition the road segment into a rectangular grid.
    // If <isForward> is true, then vehicle traffic on the road segment flows from its start
    // to end points.
    void
    partition(RoadSegment const & segment, bool isForward);

    // Return true if the stretch of the road segment is inside the specified grid cell.
    // The stretch is specified by <p1>, <p2>, and <halfWidth>; the line from <p1> to <p2>
    // traces the middle of the stretch.  <m> and <n> specify the (m, n) grid cell of a
    // rectangular grid of gridWidth_ and gridHeight_.
    bool
    checkGrid(int m, int n, Point2D const & p1, Point2D const & p2, centimeter_t halfWidth) const;

private:
    centimeter_t gridWidth_;
    centimeter_t gridHeight_;

    // The following custom hash and equality functions were taken
    // from the boost::unordered documentation.

    typedef Point2D Grid2D;

    struct Hash2D : private std::unary_function<Grid2D, std::size_t>
    {
        size_t
        operator()(Grid2D const & key) const
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, key.getX());
            boost::hash_combine(seed, key.getY());
            return seed;
        }
    };

    struct Equal2D : private std::binary_function<Grid2D, Grid2D, bool>
    {
        bool
        operator()(Grid2D const & p1, Grid2D const & p2) const
        {
            return p1.getX() == p2.getX() && p1.getY() == p2.getY();
        }
    };

    // map< key, vector<value> > is used for GridType instead of multimap<key, value>.
    typedef std::vector<RoadSegmentAndIndexPair> RoadSegmentSet;
    typedef boost::unordered_map<Grid2D, RoadSegmentSet, Hash2D, Equal2D> GridType;
    GridType grid_;

    std::map<std::string, RoadSegment const *> roadSegments_;
};

// Forward declarations.

namespace
{
    // AABB are Axially-Aligned Bounding Boxes.
    // They are aligned to the X- and Y- axes and they are bounding boxes of some object, that is,
    // the object is wholly inside the AABB.
    struct AABB
    {
        Point2D lowerLeft_;
        Point2D upperRight_;

        AABB(Point2D const & lowerLeft, Point2D const & upperRight)
          : lowerLeft_(lowerLeft)
          , upperRight_(upperRight)
        {
        }

        AABB(Point2D const & lowerLeft, centimeter_t width, centimeter_t height)
          : lowerLeft_(lowerLeft)
          , upperRight_(lowerLeft_.getX() + width, lowerLeft_.getY() + height)
        {
        }

        AABB(centimeter_t left, centimeter_t right, centimeter_t bottom, centimeter_t top)
          : lowerLeft_(left, bottom)
          , upperRight_(right, top)
        {
        }

        centimeter_t left() const { return lowerLeft_.getX(); }
        centimeter_t right() const { return upperRight_.getX(); }
        centimeter_t bottom() const { return lowerLeft_.getY(); }
        centimeter_t top() const { return upperRight_.getY(); }
    };

    // Calculate the AABB that would contain a stretch of a road segment.
    // The stretch is specified by <p1>, <p2>, and <halfWidth>; the line from <p1> to <p2>
    // traces the middle of the stretch.  The line may not be aligned to the X- and Y- axes.
    AABB
    getBoundingBox(Point2D const & p1, Point2D const & p2, centimeter_t halfWidth);

    // Return true if <point> is inside the rectangle <aabb>.
    inline bool
    isPointInsideAABB(Point2D const & point, AABB const & aabb)
    {
        return    aabb.left() <= point.getX() && point.getX() <= aabb.right() 
               && aabb.bottom() <= point.getY() && point.getY() <= aabb.top();
    }

    // Return the width of the specified road segment.
    inline centimeter_t
    getWidth(RoadSegment const & segment)
    {
        if (segment.width != 0)
        {
            return segment.width;
        }
        else
        {
            centimeter_t width = 0;
            std::vector<Lane*> const & lanes = segment.getLanes();
            for (size_t i = 0; i < lanes.size(); i++)
            {
                width += lanes[i]->getWidth();
            }
            return width;
        }
    }

    // Return the lane where <point> is located, 0 if the point is outside of the stretch of
    // the road segment.
    // The stretch is specified by <p1>, <p2>, and <segment->width>; the line from <p1> to <p2>
    // traces the middle of the stretch.
    Lane const *
    getTheLane(RoadSegment const & segment, Point2D const & p1, Point2D const & p2,
               Point2D const & point);

    // Return true if <aabb> covers <grid> completely.
    inline bool
    isGridWhollyInsideAABB(AABB const & grid, AABB const & aabb)
    {
        return    aabb.left() <= grid.left() && grid.right() <= aabb.right()
               && aabb.bottom() <= grid.bottom() && grid.top() <= aabb.top();
    }

    // Return true if the stretch of road segment overlaps or lies within the rectangle <aabb>.
    // The stretch is specified by <p1>, <p2>, and <halfWidth>; the line from <p1> to <p2>
    // traces the middle of the stretch.
    bool
    didRoadIntersectAABB(Point2D const & p1, Point2D const & p2, centimeter_t halfWidth,
                         AABB const & aabb);
}

inline void
StreetDirectory::Impl::partition(std::vector<RoadSegment*> const & segments, bool isForward)
{
    for (size_t i = 0; i < segments.size(); i++)
    {
        partition(*segments[i], isForward);
    }
}

inline void
StreetDirectory::Impl::partition(RoadNetwork const & network)
{
    std::vector<Link*> const & links = network.getLinks();
    for (size_t i = 0; i < links.size(); i++)
    {
        Link const * link = links[i];
        partition(link->getPath(true), true);
        partition(link->getPath(false), true);
    }
}

inline
StreetDirectory::Impl::Impl(RoadNetwork const & network,
                            centimeter_t gridWidth, centimeter_t gridHeight)
  : gridWidth_(gridWidth)
  , gridHeight_(gridHeight)
{
    partition(network);
}

inline bool
StreetDirectory::Impl::checkGrid(int m, int n, Point2D const & p1, Point2D const & p2,
                                 centimeter_t halfWidth) const
{
    AABB grid(Point2D(m * gridWidth_, n * gridHeight_), gridWidth_, gridHeight_);
    return didRoadIntersectAABB(p1, p2, halfWidth, grid);
}

void
StreetDirectory::Impl::partition(RoadSegment const & segment, bool isForward)
{
    centimeter_t halfWidth = getWidth(segment) / 2;

    for (size_t i = 0; i < segment.polyline.size() - 1; i++)
    {
        size_t startIndex = 0, endIndex = 0;
        if (isForward)
        {
            startIndex = i;
            endIndex = i + 1;
        }
        else
        {
            startIndex = segment.polyline.size() - 1 - i;
            endIndex = segment.polyline.size() - 1 - i - 1;
        }

        Point2D const & p1 = segment.polyline[startIndex];
        Point2D const & p2 = segment.polyline[endIndex];

        // We calculate the AABB that encloses the road segment stretch so that we can
        // quickly determine which grid cells the road segment stretch is in.
        AABB aabb = getBoundingBox(p1, p2, halfWidth);

        int left = aabb.lowerLeft_.getX() / gridWidth_;
        int right = aabb.upperRight_.getX() / gridWidth_;
        int bottom = aabb.lowerLeft_.getY() / gridHeight_;
        int top = aabb.upperRight_.getY() / gridHeight_;

        const RoadSegmentAndIndexPair pair(&segment, startIndex, endIndex);

        // The AABB containing the road segment stretch overlaps the following grid cells (m. n).
        for (int m = left; m <= right; m++)
        {
            for (int n = bottom; n <= top; n++)
            {
                // p1 is inside the (left, bottom) grid cell and p2 is inside (right, top)
                // grid cell.  So we push this road segment stretch into these grid cells.
                if ((m == left && n == bottom) || (m == right && n == top))
                    grid_[Grid2D(m, n)].push_back(pair);
                else if (checkGrid(m, n, p1, p2, halfWidth))
                {
                    grid_[Grid2D(m, n)].push_back(pair);
                }
            }
        }
    }
}

StreetDirectory::LaneAndIndexPair
StreetDirectory::Impl::getLane(Point2D const & point) const
{
    Grid2D cell(point.getX() / gridWidth_, point.getY() / gridHeight_);
    GridType::const_iterator iter = grid_.find(cell);
    if (iter == grid_.end())
    {
        // Either the road network for this grid cell was not loaded from the database
        // or there is no road network in the cell (for example, the cell could be a portion
        // of a large lake or reservoir).  In this version we assume the entire road network
        // was loaded in.
        return LaneAndIndexPair();
    }

    // We only need to check the road segments in this grid cell.
    RoadSegmentSet const & segments = iter->second;
    for (size_t i = 0; i < segments.size(); ++i)
    {
        RoadSegmentAndIndexPair const & pair = segments[i];
        RoadSegment const * segment = pair.segment_;
        size_t start = pair.startIndex_;
        size_t end = pair.endIndex_;

        centimeter_t halfWidth = getWidth(*segment) / 2;
        Point2D const & p1 = segment->polyline[start];
        Point2D const & p2 = segment->polyline[end];
        AABB aabb = getBoundingBox(p1, p2, halfWidth);

        // The outer test is inexpensive.  We quickly skip road segments that are too far
        // from <point>.  However <aabb> may be fairly large; worst case is when the line from
        // <p1> to <p2> is inclined 45 degrees to the axes.  Therefore it is possible that
        // <point> falls inside <aabb>, but is outside of the stretch of <segment>.  The inner
        // test is more thorough.
        if (isPointInsideAABB(point, aabb))
        {
            if (Lane const * lane = getTheLane(*segment, p1, p2, point))
                return StreetDirectory::LaneAndIndexPair(lane, start, end);
        }
    }

    return LaneAndIndexPair();
}

std::vector<StreetDirectory::RoadSegmentAndIndexPair>
StreetDirectory::Impl::closestRoadSegments(Point2D const & point,
                                           centimeter_t halfWidth, centimeter_t halfHeight) const
{
    // <aabb> is the search rectangle.
    AABB aabb(point, halfWidth, halfHeight);
    int left = aabb.lowerLeft_.getX() / gridWidth_;
    int right = aabb.upperRight_.getX() / gridWidth_;
    int bottom = aabb.lowerLeft_.getY() / gridHeight_;
    int top = aabb.upperRight_.getY() / gridHeight_;

    std::vector<RoadSegmentAndIndexPair> result;
    // The search rectangle overlaps the following grid cells (m. n).
    for (int m = left; m <= right; m++)
    {
        for (int n = bottom; n <= top; n++)
        {
            GridType::const_iterator iter = grid_.find(Grid2D(m, n));
            if (iter != grid_.end())
            {
                std::vector<RoadSegmentAndIndexPair> const & segments = iter->second;

                AABB grid(Point2D(m * gridWidth_, n * gridHeight_), gridWidth_, gridHeight_);
                if (isGridWhollyInsideAABB(grid, aabb))
                {
                    // Wow, the search rectangle is larger than the grid size.
                    result.insert(result.end(), segments.begin(), segments.end());
                }
                else
                {
                    // The search rectangle does not cover this grid cell completely.
                    // We need to figure out which road segment is really inside <aabb>.
                    for (size_t i = 0; i < segments.size(); i++)
                    {
                        RoadSegmentAndIndexPair const & pair = segments[i];
                        RoadSegment const * segment = pair.segment_;
                        size_t const start = pair.startIndex_;
                        size_t const end = pair.endIndex_;

                        Point2D const & p1 = segment->polyline[start];
                        Point2D const & p2 = segment->polyline[end];
                        centimeter_t halfWidth = getWidth(*segment) / 2;
                        if (didRoadIntersectAABB(p1, p2, halfWidth, aabb))
                        {
                            result.push_back(pair);
                        }
                    }
                }
            }
        }
    }
    return result;
}

/** \endcond ignoreStreetDirectoryInnards -- End of block to be ignored by doxygen.  */

namespace
{
    AABB
    getBoundingBox(Point2D const & p1, Point2D const & p2, centimeter_t halfWidth)
    {
        centimeter_t left = 0;
        centimeter_t right = 0;
        centimeter_t top = 0;
        centimeter_t bottom = 0;

        if (p1.getX() > p2.getX())
        {
            left = p2.getX();
            right = p1.getX();
        }
        else
        {
            left = p1.getX();
            right = p2.getX();
        }
        if (p1.getY() > p2.getY())
        {
            top = p1.getY();
            bottom = p2.getY();
        }
        else
        {
            top = p2.getY();
            bottom = p1.getY();
        }

        // If we have the four corners of the stretch of road segment, we can quickly calculate
        // the smallest AABB that contains the stretch.  However, we only have the middle line.
        // We could calculate the 4 corners, but that is expensive.  Instead we return a slightly
        // larger AABB.
        left -= halfWidth;
        right += halfWidth;
        top += halfWidth;
        bottom -= halfWidth;

        return AABB(left, right, bottom, top);
    }

    // Return the distance between <point> and the closest point on the line from <p1> to <p2>;
    // return -1 if the closest point is one of the end points.
    //
    // The closest point is the projection of <point> onto the line from <p1> to <p2>.  If the
    // projection falls outside of the line, then the closest point is one of the end points.
    // In that case the function returns -1.
    centimeter_t
    distanceOfPointFromLine(Point2D const & point, Point2D const & p1, Point2D const & p2)
    {
        // The Vector2D<T> class provides the dot product function.  We have to use T=float
        // because the multiplication in the dot product function may overflow on a 32-bit
        // platform.
        Vector2D<float> a(p1.getX(), p1.getY());
        Vector2D<float> b(p2.getX(), p2.getY());
        Vector2D<float> c(point.getX(), point.getY());
        Vector2D<float> line = b - a; // the road segment polyline
        Vector2D<float> vec = c - a; // the vector from <point> to the start <p1> of polyline

        float dot = line * vec;  // the dot product of <line> and <vec>.
        if (dot < 0.0)
        {
            // <point> is outside of the road segment, before the start point <p1>
            return -1;
        }

        float lineLength = line * line;
        if (dot > lineLength)
        {
            // <point> is outside of the road segment, after the end point <p2>
            return -1;
        }

        return sqrtf(vec * vec - dot / lineLength);
    }

    Lane const*
    getTheLane(RoadSegment const & segment, Point2D const & p1, Point2D const & p2,
               Point2D const & point)
    {
        centimeter_t dist = distanceOfPointFromLine(point, p1, p2);
        if (dist < 0)
            return 0;

        centimeter_t halfWidth = getWidth(segment) / 2;
        if (dist > halfWidth)
        {
            // although <point> is between the start and end points, it is outside of
            // the road segment
            return 0;
        }

        Vector2D<float> a(p1.getX(), p1.getY());
        Vector2D<float> b(p2.getX(), p2.getY());
        Vector2D<float> c(point.getX(), point.getY());
        Vector2D<float> line = b - a; // the road segment polyline
        Vector2D<float> vec = c - a; // the vector from <point> to the start <p1> of polyline

        // We now know that <point> is inside the road segment.  But is it on the left side or
        // right side of the polyline?
        // The vector perpendicular to a vector (x,y) is (-y, x), the perpendicular pointing to
        // the left of the vector.
        // <point> will be on the left of the polyline if and only if <vec> and <linePerp> are
        // pointing in the same direction.  That is, when the dot product of <vec> and <linePerp>
        // is negative.
        Vector2D<float> linePerp(-line.getY(), line.getX());
        bool isLeft = ((linePerp * vec) < 0);

        // Now we calculate the distance of <point> from the left side of the road segment.
        if (isLeft)
        {
            dist = halfWidth - dist;
        }
        else
        {
            dist = dist + halfWidth;
        }

        centimeter_t d = 0;
        // We can now determine which lane <point> is in.
        std::vector<Lane*> const & lanes = segment.getLanes();
        for (size_t i = 0; i < lanes.size(); ++i)
        {
            Lane const * lane = lanes[i];
            if (d + static_cast<centimeter_t>(lane->getWidth()) > dist)
                return lane;
            d += lane->getWidth();
        }

        assert(false); // We shouldn't reach here.
        return 0;
    }

    // Return true if 0.0 <= <num> / <denom> <= 1.0
    inline bool
    isInBetween(centimeter_t num, centimeter_t denom)
    {
        if (num > 0)
        {
            if (denom > 0 && num < denom)
                return true;
        }
        else
        {
            if (denom < 0 && num > denom)
                return true;
        }
        return false;
    }

    // Return true if the line from <p1> to <p2> intersects or lies within the rectangle <aabb>.
    inline bool
    didLineIntersectAABB(Point2D const & p1, Point2D const & p2, AABB const & aabb)
    {
        // If either end of the line is inside the rectangle, then the line must either
        // intersects or lies within the rectangle.
        if (isPointInsideAABB(p1, aabb) || isPointInsideAABB(p2, aabb))
            return true;

        // Consider the parameterized equation R(t) = p1 + t * (p2 - p1).  This equation
        // represents the ray passing through <p1> and <p2>, the line is just part of the ray
        // for 0.0 <= t <= 1.0.  Therefore We check if the line intersects one of the 4 sides
        // of <aabb> by checking if 0.0 <= t <= 1.0 for t = (R(t) - p1) / (p2 - p1).
        centimeter_t xDiff = p2.getX() - p1.getX();
        centimeter_t yDiff = p2.getY() - p1.getY();
        if (isInBetween(aabb.left() - p1.getX(), xDiff))
            return true;
        if (isInBetween(aabb.right() - p1.getX(), xDiff))
            return true;
        if (isInBetween(aabb.bottom() - p1.getY(), yDiff))
            return true;
        if (isInBetween(aabb.top() - p1.getY(), yDiff))
            return true;
        return false;
    }

    bool
    didRoadIntersectAABB(Point2D const & p1, Point2D const & p2, centimeter_t halfWidth,
                         AABB const & aabb)
    {
        if (didLineIntersectAABB(p1, p2, aabb))
            return true;

        // The line from <p1> to <p2> only trace the middle of the stretch of road segment.
        // This line may not have intersected <aabb>, but the stretch may still intersect <aabb>.

        // When the line is vertical, check if the middle line comes within <halfWidth> of the
        // left or right side of <aabb>.
        if (p1.getX() == p2.getX())
        {
            if (((aabb.left() - p1.getX()) < halfWidth) || ((p1.getX() - aabb.right()) < halfWidth))
                return true;
            return false;
        }
        // When the line is horizontal, check if the middle line comes within <halfWidth> of the
        // top or bottom side of <aabb>.
        if (p1.getY() == p2.getY())
        {
            if (((aabb.bottom() - p1.getY()) < halfWidth) || ((p1.getY() - aabb.top()) < halfWidth))
                return true;
            return false;
        }

        // We still need to check if the middle line comes within <halfWidth> of one of the
        // 4 corners of <aabb>.
        if (distanceOfPointFromLine(aabb.lowerLeft_, p1, p2) < halfWidth)
            return true;
        if (distanceOfPointFromLine(aabb.upperRight_, p1, p2) < halfWidth)
            return true;
        if (distanceOfPointFromLine(Point2D(aabb.left(), aabb.top()), p1, p2) < halfWidth)
            return true;
        if (distanceOfPointFromLine(Point2D(aabb.right(), aabb.bottom()), p1, p2) < halfWidth)
            return true;

        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
// StreetDirectory::ShortestPathImpl
////////////////////////////////////////////////////////////////////////////////////////////

/** \cond ignoreStreetDirectoryInnards -- Start of block to be ignored by doxygen.  */

class StreetDirectory::ShortestPathImpl : private boost::noncopyable
{
public:
    explicit ShortestPathImpl(RoadNetwork const & network);
    ~ShortestPathImpl();

    std::vector<WayPoint>
    shortestDrivingPath(Node const & fromNode, Node const & toNode) const;

    std::vector<WayPoint>
    shortestWalkingPath(Point2D const & fromPoint, Point2D const & toPoint) const;

private:
    // We attach a property to each vertex, its name.  We treat the name property as a mean
    // to identify the vertex.  However instead of a textual name, the actual value that we
    // attach to the vertex's name is the pointer to the Node where the vertex is located.
    // When we search the graph for a vertex, we use the Node::location in the search.
    typedef boost::property<boost::vertex_name_t, Node const *> VertexProperties;
    // Each edge has a weight property cascaded with a name property.  Just like the vertex's
    // name property, the edge's name property is the pointer to WayPoint.  The weight property is
    // the length of the road-segment, side-walk or crossing that each edge represent.
    typedef boost::property<boost::edge_weight_t, centimeter_t,
                boost::property<boost::edge_name_t, WayPoint> > EdgeProperties;
    // The graph contains arrays of integers for the vertices and edges and is a directed graph.
    typedef boost::adjacency_list<boost::vecS,
                                  boost::vecS,
                                  boost::directedS,
                                  VertexProperties,
                                  EdgeProperties,
                                  boost::no_property> Graph;
    typedef Graph::vertex_descriptor Vertex;  // A vertex is an integer into the vertex array.
    typedef Graph::edge_descriptor Edge; // An edge is an integer into the edge array.

private:
    Graph drivingMap_; // A map for drivers, containing road-segments as edges.
    Graph walkingMap_; // A map for pedestrians, containing side-walks and crossings as edges.
    std::vector<Node *> nodes_; // "Internal" uni-nodes that are created when building the maps.

private:
    void process(std::vector<RoadSegment*> const & roads, bool isForward);
    void process(RoadSegment const * road, bool isForward);

    // Oh wow!  Overloaded functions with different return types.
    Node const * findVertex(Graph const & graph, Point2D const & point, centimeter_t distance) const;
    Vertex findVertex(Graph const & graph, Node const * node) const;
    Vertex findVertex(Point2D const & point);

    Node const * findNode(Point2D const & point);

    void addRoadEdge(Node const * node1, Node const * node2,
                     WayPoint const & wp, centimeter_t length);

    void addSideWalk(Lane const * sideWalk, centimeter_t length);
    void addCrossing(Crossing const * crossing, centimeter_t length);

    void getVertices(Vertex & fromVertex, Vertex & toVertex,
                     Node const & fromNode, Node const & toNode) const;

    void getVertices(Vertex & fromVertex, Vertex & toVertex,
                     Point2D const & fromPoint, Point2D const & toPoint) const;

    void checkVertices(Vertex const fromVertex, Vertex const toVertex,
                       Node const & fromNode, Node const & toNode) const;

    std::vector<WayPoint>
    extractShortestPath(Vertex const fromVertex, Vertex const toVertex,
                        std::vector<Vertex> const & parent, Graph const & graph) const;

    std::vector<WayPoint>
    shortestPath(Vertex const fromVertex, Vertex const toVertex, Graph const & graph) const;
};

StreetDirectory::ShortestPathImpl::~ShortestPathImpl()
{
    // Cleanup to avoid memory leakage.
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        Node * node = nodes_[i];
        delete node->location;
        delete node;
    }
    nodes_.clear();
}

namespace
{
    // Return the square of Euclidean distance between p1 and p2.
    inline double hypot(Point2D const & p1, Point2D const & p2)
    {
        double x = p1.getX() - p2.getX();
        double y = p1.getY() - p2.getY();
        return (x*x) + (y*y);
    }

    Point2D
    getBusStopPosition(RoadSegment const * road, centimeter_t offset)
    {
        std::vector<Lane*> const & lanes = road->getLanes();
        Lane * const leftLane = lanes[lanes.size() - 1];

        // Walk along the left lane and stop after we have walked <offset> centimeters.
        // The bus stop should be at that point.
        std::vector<Point2D> const & polyline = leftLane->getPolyline();
        size_t i = 0;
        Point2D p1;
        Point2D p2;
        double len;
        while (i < polyline.size() - 1)
        {
            p1 = polyline[i];
            p2 = polyline[i + 1];
            len = sqrt(hypot(p1, p2));
            if (offset - len < 0)
            {
                // Walking to <p2> would be a bit too far.
                break;
            }
            offset -= len;
            ++i;
        }
        // Walk to the point before <p2> that would end at <offset>.
        double ratio = offset / len;
        centimeter_t x = p1.getX() + ratio * (p2.getX() - p1.getX());
        centimeter_t y = p1.getY() + ratio * (p2.getY() - p1.getY());
        return Point2D(x, y);
    }

    // Return true if <p1> and <p2> are within <distance>.  Quick and dirty calculation of the
    // distance between <p1> and <p2>.
    inline bool
    closeBy(Point2D const & p1, Point2D const & p2, centimeter_t distance)
    {
        return    (std::abs(p1.getX() - p2.getX()) < distance)
               && (std::abs(p1.getY() - p2.getY()) < distance);
    }
}

// Search the graph for a vertex located at a Node that is within <distance> from <point>
// and return that Node, if any; otherwise return 0.
Node const *
StreetDirectory::ShortestPathImpl::findVertex(Graph const & graph, Point2D const & point,
                                              centimeter_t distance)
const
{
    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(graph); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * node = boost::get(boost::vertex_name, graph, v);
        if (closeBy(*node->location, point, distance))
            return node;
    }
    return nullptr;
}

// If there is a Node in the drivingMap_ that is within 0.5 meter from <point>, return it;
// otherwise return a new "internal" node located at <point>.  "Internal" means the node exists
// only in the StreetDirectory::ShortestPathImpl object to be used as a location for a vertex.
//
// This function is a hack.  We can't use uni-nodes as vertices in the drivingMap_ graph and
// yet the lanes polylines are not "connected".  That is, the last point of the lane's polyline
// in one road-segment is not the first point of the lane's polyline in the next road-segment.
// I think the gap is small, hopefully it is less than 0.5 meter.
Node const *
StreetDirectory::ShortestPathImpl::findNode(Point2D const & point)
{
    Node const * node = findVertex(drivingMap_, point, 50);
    if (node)
        return node;

    Node * n = new UniNode();
    n->location = new Point2D(point.getX(), point.getY());
    nodes_.push_back(n);
    return n;
}

// Build up the drivingMap_ and walkingMap_ graphs.
//
// Road segments are the only edges in the drivingMap_ graph.  Multi-nodes are inserted as
// vertices in the graph; it is assumed that vehicles are allowed to make U-turns at the
// intersections if the link is bi-directional.  Uni-nodes in bi-directional links are split into
// 2 vertices, without any edge between the 2 vertices; at the current moment, vehicles are not
// allowed to make U-turns at uni-nodes.  We need to fix this; we need to add an edge from one
// vertex to the other if traffic is allowed to move in that direction.  The edge length would be
// 0 since the 2 vertices are located at the same uni-node.
//
// For the walkingMap_ graph, the side walks in each road segment, crossings at the
// intersections, and zebra crossings are edges.  Since pedestrians are not supposed to be
// walking on the road dividers between 2 side-by-side road-segments, the road divider is
// not treated as an edge.  Hence each link, whether bidirectional or one-way, has exactly
// 2 side walk edges.
//
// If there is a bus stop, the road segment and the side walk is split into 2 edges, split at
// the bus stop.  Therefore shortestDrivingPath() would include a bus stop as a waypoint,
// which would be useful for the BusDriver model but may not be needed by the Driver model.
//
// We assume that if a link has any crossing (signalized or zebra crossing), the link is
// split into several road-segments, split at the crossing.  Hence we assume that the
// crossing at the beginning or end of a road segment.  Therefore, side walks are not split
// by crossings.  This assumption needs to be reviewed.

inline void
StreetDirectory::ShortestPathImpl::process(std::vector<RoadSegment*> const & roads, bool isForward)
{
    for (size_t i = 0; i < roads.size(); ++i)
        process(roads[i], isForward);
}

inline StreetDirectory::ShortestPathImpl::ShortestPathImpl(RoadNetwork const & network)
{
    std::vector<Link*> const & links = network.getLinks();
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter)
    {
        Link const * link = *iter;
        process(link->getPath(true), true);
        process(link->getPath(false), false);
    }
}

void
StreetDirectory::ShortestPathImpl::process(RoadSegment const * road, bool isForward)
{
    // If this road-segment is inside a one-way Link, then there should be 2 side-walks.
    std::vector<Lane*> const & lanes = road->getLanes();
    for (size_t i = 0; i < lanes.size(); ++i)
    {
        Lane const * lane = lanes[i];
        if (lane->is_pedestrian_lane())
        {
            addSideWalk(lane, road->length);
        }
    }

    Node const * node1 = road->getStart();
    if (dynamic_cast<UniNode const *>(node1))
    {
        // If this road-segment is side-by-side to another road-segment going in the other
        // direction, we cannot insert this uni-node into the drivingMap_ graph.  Otherwise,
        // vehicles on both road-segments would be allowed to make U-turns at the uni-node,
        // which may not correct.  Currently we do not have info from the database about the
        // U-turns at the uni-nodes.  Instead of using the uni-node as a vertex in the drivingMap_,
        // we choose a point in one of the lane's polyline.
        std::vector<Point2D> const & polyline = road->getLanes()[0]->getPolyline();
        Point2D point = polyline[0];
        node1 = findNode(point);
    }

    centimeter_t offset = 0;
    while (offset < road->length)
    {
        RoadItemAndOffsetPair pair = road->nextObstacle(offset, isForward);
        if (0 == pair.item)
        {
            break;
        }


        if (Crossing const * crossing = dynamic_cast<Crossing const *>(pair.item))
        {
            // In a bi-directional link, a crossing would span the 2 side-by-side road-segments.
            // Therefore, we may have already inserted this crossing into the walkingMap_ when
            // we process the previous road segment.  We check if it is the graph, continuing only
            // if it wasn't.
            bool notInWalkingMap = true;
            Graph::edge_iterator iter, end;
            for (boost::tie(iter, end) = boost::edges(walkingMap_); iter != end; ++iter)
            {
                Edge e = *iter;
                WayPoint const & wp = boost::get(boost::edge_name, walkingMap_, e);
                if (WayPoint::CROSSING == wp.type_ && crossing == wp.crossing_)
                {
                    notInWalkingMap = false;
                    break;
                }
            }

            if (notInWalkingMap)
            {
                centimeter_t length = sqrt(hypot(crossing->nearLine.first, crossing->nearLine.second));
                addCrossing(crossing, length);
            }
        }
#if 0
        else if (ZebraCrossing const * crossing = dynamic_cast<ZebraCrossing const *>(pair.item))
        {
        }
#endif
        else if (BusStop const * busStop = dynamic_cast<BusStop const *>(pair.item))
        {
            const Point2D pos = getBusStopPosition(road, offset);
            Node * node2 = new UniNode();
            node2->location = new Point2D(pos.getX(), pos.getY());
            addRoadEdge(node1, node2, WayPoint(busStop), offset);
            nodes_.push_back(node2);
            node1 = node2;
        }
        offset = pair.offset + 1;
    }

    Node const * node2 = road->getEnd();
    if (dynamic_cast<UniNode const *>(node2))
    {
        // See comment above about the road-segment's start-node.
        std::vector<Point2D> const & polyline = road->getLanes()[0]->getPolyline();
        Point2D point = polyline[polyline.size() - 1];
        node2 = findNode(point);
    }
    addRoadEdge(node1, node2, WayPoint(road), offset);
}

// Search for <node> in <graph>.  If any vertex in <graph> has <node> attached to it, return it;
// otherwise insert a new vertex (with <node> attached to it) and return it.
StreetDirectory::ShortestPathImpl::Vertex
StreetDirectory::ShortestPathImpl::findVertex(Graph const & graph, Node const * node)
const
{
    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(graph); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * n = boost::get(boost::vertex_name, graph, v);
        if (node == n)
            return v;
    }

    Vertex v = boost::add_vertex(const_cast<Graph &>(graph));
    boost::put(boost::vertex_name, const_cast<Graph &>(graph), v, node);
    return v;
}

// Insert a directed edge into the drivingMap_ graph from <node1> to <node2>, which represent
// vertices in the graph.  <wp> is attached to the edge as its name property and <length> as
// its weight property.
void
StreetDirectory::ShortestPathImpl::addRoadEdge(Node const * node1, Node const * node2,
                                               WayPoint const & wp, centimeter_t length)
{
    Vertex u = findVertex(drivingMap_, node1);
    Vertex v = findVertex(drivingMap_, node2);

    Edge edge;
    bool ok;
    boost::tie(edge, ok) = boost::add_edge(u, v, drivingMap_);
    boost::put(boost::edge_name, drivingMap_, edge, wp);
    boost::put(boost::edge_weight, drivingMap_, edge, length);
}

// If there is a Node in the walkingMap_ that is within 10 meters from <point>, return the
// vertex with that node; otherwise create a new "internal" node located at <point>, insert a
// vertex with the new node, and return the vertex.  "Internal" means the node exists only in the
// StreetDirectory::ShortestPathImpl object to be used as a location for a vertex.
//
// This function is a hack.  The side-walks and crossings are not aligned, that is, they are not
// connected.  The gaps are quite large.  I hope that 10 meters would be a good choice.  It should
// be ok if there really is a crossing at the end of sidewalk (or vice versa, a side-walk at the
// end of the crossing).  But it may be too large that the function incorrectly returns a vertex
// that is on the opposite of a narrow road-segment.
StreetDirectory::ShortestPathImpl::Vertex
StreetDirectory::ShortestPathImpl::findVertex(Point2D const & point)
{
    Node const * node = findVertex(walkingMap_, point, 1000);
    if (!node)
    {
        Node * n = new UniNode();
        n->location = new Point2D(point.getX(), point.getY());
        nodes_.push_back(n);
        node = n;
    }

    return findVertex(walkingMap_, node);
}

// Insert a directed edge into the walkingMap_ graph from one end of <sideWalk> to the other end,
// both ends represent the vertices in the graph.  <sideWalk> is attached to the edge at its name
// property and <length> as its weight property.
//
// Side-walks are bi-directional for pedestrians.  So 2 edges are inserted for the 2 directions.
void
StreetDirectory::ShortestPathImpl::addSideWalk(Lane const * sideWalk, centimeter_t length)
{
    std::vector<Point2D> const & polyline = sideWalk->getPolyline();
    Vertex u = findVertex(polyline[0]);
    Vertex v = findVertex(polyline[polyline.size() - 1]);

    Edge edge;
    bool ok;
    WayPoint wp(sideWalk);

    boost::tie(edge, ok) = boost::add_edge(u, v, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);

    // Side walks are bi-directional for pedestrians.  Add another edge for the other direction.
    boost::tie(edge, ok) = boost::add_edge(v, u, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);
}

// Insert a directed edge into the walkingMap_ graph from one end of <crossing> to the other end,
// both ends represent the vertices in the graph.  <crossing> is attached to the edge at its name
// property and <length> as its weight property.
//
// Crossings are bi-directional for pedestrians.  So 2 edges are inserted for the 2 directions.
void
StreetDirectory::ShortestPathImpl::addCrossing(Crossing const * crossing, centimeter_t length)
{
    Vertex u = findVertex(crossing->nearLine.first);
    Vertex v = findVertex(crossing->nearLine.second);

    Edge edge;
    bool ok;
    WayPoint wp(crossing);

    boost::tie(edge, ok) = boost::add_edge(u, v, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);

    // Crossings are bi-directional for pedestrians.  Add another edge for the other direction.
    boost::tie(edge, ok) = boost::add_edge(v, u, walkingMap_);
    boost::put(boost::edge_name, walkingMap_, edge, wp);
    boost::put(boost::edge_weight, walkingMap_, edge, length);
}

std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::shortestDrivingPath(Node const & fromNode, Node const & toNode)
const
{
    if (&fromNode == &toNode)
        return std::vector<WayPoint>();

    // Convert the fromNode and toNode (positions in 2D geometry) to vertices in the drivingMap_
    // graph.  It is possible that fromNode and toNode are not represented by any vertex in the
    // graph.
    Vertex fromVertex, toVertex;
    getVertices(fromVertex, toVertex, fromNode, toNode);
    // If fromNode and toNode are not represented by any vertex in the graph, then throw an
    // error message.
    checkVertices(fromVertex, toVertex, fromNode, toNode);

    return shortestPath(fromVertex, toVertex, drivingMap_);
}

// Find the vertices in the drivingMap_ graph that represent <fromNode> and <toNode> and return
// them in <fromVertex> and <toVertex> respectively.  If the node is not represented by any vertex,
// then the returned vertex is set to a value larger than the number of vertices in the graph.
void
StreetDirectory::ShortestPathImpl::getVertices(Vertex & fromVertex, Vertex & toVertex,
                                               Node const & fromNode, Node const & toNode)
const
{
    Graph::vertices_size_type graphSize = boost::num_vertices(drivingMap_);
    fromVertex = graphSize + 1;
    toVertex = graphSize + 1;

    Node const * from = (dynamic_cast<UniNode const *>(&fromNode)) ? &fromNode : nullptr;
    Node const * to = (dynamic_cast<UniNode const *>(&fromNode)) ? &toNode : nullptr;

    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(drivingMap_); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * node = boost::get(boost::vertex_name, drivingMap_, v);
        // Uni-nodes were never inserted into the drivingMap_, but some other points close to
        // them.  Hopefully, they are within 10 meters and that the correct vertex is returned,
        // even in narrow links.
        if (from && closeBy(*from->location, *node->location, 1000))
        {
            fromVertex = v;
        }
        else if (to && closeBy(*to->location, *node->location, 1000))
        {
            toVertex = v;
        }
        else if (node == &fromNode)
        {
            fromVertex = v;
        }
        else if (node == &toNode)
        {
            toVertex = v;
        }
        if (fromVertex < graphSize && toVertex < graphSize)
            break;
    }
}

// Check that <fromVertex> and <toVertex> are valid vertex in the drivingMap_ graph (ie. if they
// are between 0 and the number of vertices in the graph).  If not, throw an error message.
// Since the error message is intended for the modellers and <fromVertex> and <toVertex> are internal
// data, the message is formatted with info from <fromNode> and <toNode>.
void
StreetDirectory::ShortestPathImpl::checkVertices(Vertex const fromVertex, Vertex const toVertex,
                                                 Node const & fromNode, Node const & toNode)
const
{
    Graph::vertices_size_type graphSize = boost::num_vertices(drivingMap_);
    if (fromVertex > graphSize || toVertex > graphSize)
    {
        std::ostringstream stream;
        stream << "StreetDirectory::shortestDrivingPath: "; 
        if (fromVertex > graphSize)
        {
            stream << "fromNode=" << *fromNode.location << " is not part of the known road network ";
        }
        if (toVertex > graphSize)
        {
            stream << "toNode=" << *toNode.location << " is not part of the known road network";
        }

        throw stream.str();
    }
}

// Computes the shortest path from <fromVertex> to <toVertex> in the graph.  If <toVertex> is not
// reachable from <fromVertex>, then return an empty array.
std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::shortestPath(Vertex const fromVertex, Vertex const toVertex,
                                                Graph const & graph)
const
{
    // The code here is based on the example in the book "The Boost Graph Library" by
    // Jeremy Siek, et al.
    std::vector<Vertex> parent(boost::num_vertices(graph));
    for (Graph::vertices_size_type i = 0; i < boost::num_vertices(graph); ++i)
    {
        parent[i] = i;
    }
    // If I have counted them correctly, dijkstra_shortest_paths() function has 12 parameters,
    // 10 of which have default values.  The Boost Graph Library has a facility (called named
    // parameter) that resembles python keyword argument.  You can pass an argument to one of
    // those parameters that have default values without worrying about the order of the parameters.
    // In the following, only the predecessor_map parameter is named and the parent array is
    // passed in as this parameter; the other parameters take their default values.
    boost::dijkstra_shortest_paths(graph, fromVertex, boost::predecessor_map(&parent[0]));

    return extractShortestPath(fromVertex, toVertex, parent, graph);
}

std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::extractShortestPath(Vertex const fromVertex,
                                                       Vertex const toVertex,
                                                       std::vector<Vertex> const & parent,
                                                       Graph const & graph)
const
{
    // The code here is based on the example in the book "The Boost Graph Library" by
    // Jeremy Siek, et al.  The dijkstra_shortest_path() function was called with the p = parent
    // array passed in as the predecessor_map paramter.  The shortest path from s = fromVertex
    // to v = toVertex consists of the vertices v, p[v], p[p[v]], ..., and so on until s is
    // reached, in reverse order (text from the Boost Graph Library documentation).  If p[u] = u
    // then u is either the source vertex (s) or a vertex that is not reachable from s.  Therefore
    // the while loop stops when p[u] = u.  If the while loop does not terminates with u equal to
    // fromVertex, then toVertex is not reachable from fromVertex and we return an empty path.

    Vertex v = toVertex;
    Vertex u = parent[v];
    std::vector<WayPoint> result;
    while (u != v)
    {
        Edge e;
        bool exists;
        boost::tie(e, exists) = boost::edge(u, v, graph);
        // Stop the loop if there is no path from u to v.
        if (!exists)
            break;
        WayPoint wp = boost::get(boost::edge_name, graph, e);
        result.push_back(wp);
        v = u;
        u = parent[v];
    }
    if (u != fromVertex)
        return std::vector<WayPoint>();

    // result contains the waypoints in the reverse order.  We return them in the correct order.
    return std::vector<WayPoint>(result.rbegin(), result.rend());
}

// Get the vertices in the walkingMap_ graph that are closest to <fromPoint> and <toPoint>
// and return them in <fromVertex> and <toVertex> respectively.
void
StreetDirectory::ShortestPathImpl::getVertices(Vertex & fromVertex, Vertex & toVertex,
                                               Point2D const & fromPoint, Point2D const & toPoint)
const
{
    double d1 = std::numeric_limits<double>::max();
    double d2 = std::numeric_limits<double>::max();
    double h;
    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(walkingMap_); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * node = boost::get(boost::vertex_name, walkingMap_, v);
        h = hypot(fromPoint, *node->location);
        if (d1 > h)
        {
            d1 = h;
            fromVertex = v;
        }
        h = hypot(toPoint, *node->location);
        if (d2 > h)
        {
            d2 = h;
            toVertex = v;
        }
    }
}

std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::shortestWalkingPath(Point2D const & fromPoint,
                                                       Point2D const & toPoint)
const
{
    if (fromPoint == toPoint)
        return std::vector<WayPoint>();

    // Convert the fromPoint and toPoint (positions in 2D geometry) to vertices in the walkingMap_
    // graph.  The fromVertex and toVertex corresponds to Node objects that are closest to the
    // fromPoint and toPoint positions in the graph.
    Vertex fromVertex, toVertex;
    getVertices(fromVertex, toVertex, fromPoint, toPoint);

    std::vector<WayPoint> path = shortestPath(fromVertex, toVertex, walkingMap_);
    if (path.empty())
        return path;

    // If the fromPoint is not exactly located at fromVertex, then we need to insert a Waypoint
    // that directs the pedestrian to move from fromPoint to fromVertex by some undefined mean.
    // Similarly if toPoint is not located exactly at toVertex, then we append a WayPpint to
    // move from tiVertex to toPoint.
    std::vector<WayPoint> result;
    Node const * node = boost::get(boost::vertex_name, walkingMap_, fromVertex);
    if (*node->location != fromPoint)
        result.push_back(WayPoint(node));
    result.insert(result.end(), path.begin(), path.end());
    node = boost::get(boost::vertex_name, walkingMap_, toVertex);
    if (*node->location != toPoint)
        result.push_back(WayPoint(node));
    return result;
}

/** \endcond ignoreStreetDirectoryInnards -- End of block to be ignored by doxygen.  */

////////////////////////////////////////////////////////////////////////////////////////////
// StreetDirectory
////////////////////////////////////////////////////////////////////////////////////////////

void
StreetDirectory::init(RoadNetwork const & network, bool keepStats /* = false */,
                      centimeter_t gridWidth, centimeter_t gridHeight)
{
    if (keepStats)
        stats_ = new Stats;
    pimpl_ = new Impl(network, gridWidth, gridHeight);
    spImpl_ = new ShortestPathImpl(network);
}

StreetDirectory::LaneAndIndexPair
StreetDirectory::getLane(Point2D const & point) const
{
    return pimpl_ ? pimpl_->getLane(point)
                  : LaneAndIndexPair();
}

std::vector<StreetDirectory::RoadSegmentAndIndexPair>
StreetDirectory::closestRoadSegments(Point2D const & point,
                                     centimeter_t halfWidth, centimeter_t halfHeight) const
{
    return pimpl_ ? pimpl_->closestRoadSegments(point, halfWidth, halfHeight)
                  : std::vector<RoadSegmentAndIndexPair>();
}

std::vector<WayPoint>
StreetDirectory::shortestDrivingPath(Node const & fromNode, Node const & toNode) const
{
    return spImpl_ ? spImpl_->shortestDrivingPath(fromNode, toNode)
                   : std::vector<WayPoint>();
}

std::vector<WayPoint>
StreetDirectory::shortestWalkingPath(Point2D const & fromPoint, Point2D const & toPoint) const
{
    return spImpl_ ? spImpl_->shortestWalkingPath(fromPoint, toPoint)
                   : std::vector<WayPoint>();
}

void
StreetDirectory::printStatistics() const
{
    if (stats_)
    {
        stats_->printStatistics();
    }
    else
    {
        std::cout << "No statistics was collected by the StreetDirectory singleton." << std::endl;
    }
}

void
StreetDirectory::registerSignal(Signal const & signal)
{
    Node const * node = &(signal.getNode());
    if (signals_.count(node) == 0)
    {
        signals_.insert(std::make_pair(node, &signal));
    }
}

}
