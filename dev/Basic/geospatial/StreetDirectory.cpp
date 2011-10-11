/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <vector>
#include <iostream>
#include <boost/unordered_map.hpp>

#include "Lane.hpp"
#include "RoadNetwork.hpp"
#include "StreetDirectory.hpp"
#include "../buffering/Vector2D.hpp"
#include "../entities/Signal.hpp"

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
// StreetDirectory
////////////////////////////////////////////////////////////////////////////////////////////

void
StreetDirectory::init(RoadNetwork const & network, bool keepStats /* = false */,
                      centimeter_t gridWidth, centimeter_t gridHeight)
{
    if (keepStats)
        stats_ = new Stats;
    pimpl_ = new Impl(network, gridWidth, gridHeight);
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
