/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "StreetDirectory.hpp"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <limits>
#include <boost/unordered_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <math.h>

#include "Lane.hpp"
#include "RoadNetwork.hpp"
#include "buffering/Vector2D.hpp"
#include "util/OutputUtil.hpp"
#include "util/GeomHelpers.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/LaneConnector.hpp"

#ifdef SIMMOB_NEW_SIGNAL
#include "entities/signal/Signal.hpp"
#else
#include "entities/Signal.hpp"
#endif

#include "entities/TrafficWatch.hpp"
#include "BusStop.hpp"
#include "Crossing.hpp"
#include "ZebraCrossing.hpp"
#include "UniNode.hpp"

//Define this if you want to attempt to "fix" the broken DAG.
//NOTE: Do *not* put this into the config file or CMake; once the DAG is fixed we'll remove the old code.
//TODO: The new model leaks memory (all code that does this is marked). Change to value types once you're ready to remove this #define.
#define STDIR_FIX_BROKEN




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

    const MultiNode* GetCrossingNode(const Crossing* cross) const;

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

    // Called once for each unique RoadSegment
    void buildLookups(const std::vector<RoadSegment*>& roadway, std::set<const Crossing*> completed);


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

    //Map of Crossings->MultiNode. May not contain all crossings.
    std::map<const Crossing*, const MultiNode*> crossings_to_multinodes;

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
//	RoadSegment temp_rs;
//	RoadSegment* temp_rs_ptr;
    for (size_t i = 0; i < segments.size(); i++)
    {
//    	temp_rs_ptr = segments[i];;
//    	RoadSegment &temp_rs = *temp_rs_ptr;
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
        if(link != 0)
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

    //Build additional lookups
    std::set<const Crossing*> completedCrossings;
    for (std::vector<Link*>::const_iterator iter = network.getLinks().begin(); iter != network.getLinks().end(); ++iter) {
    	buildLookups((*iter)->getPath(true), completedCrossings);
    	buildLookups((*iter)->getPath(false), completedCrossings);
    }
}

inline bool
StreetDirectory::Impl::checkGrid(int m, int n, Point2D const & p1, Point2D const & p2,
                                 centimeter_t halfWidth) const
{
    AABB grid(Point2D(m * gridWidth_, n * gridHeight_), gridWidth_, gridHeight_);
    return didRoadIntersectAABB(p1, p2, halfWidth, grid);
}

namespace {

//Helper: Find the point closest to the origin.
double GetShortestDistance(const Point2D& origin, const Point2D& p1, const Point2D& p2, const Point2D& p3, const Point2D& p4) {
	double res = sim_mob::dist(origin, p1);
	res = std::min(res, sim_mob::dist(origin, p2));
	res = std::min(res, sim_mob::dist(origin, p3));
	res = std::min(res, sim_mob::dist(origin, p4));
	return res;
}

//Helper: find the nearest MultiNode to this Segment.
const MultiNode* FindNearestMultiNode(const RoadSegment* seg, const Crossing* cr) {
	//Error case:
	const MultiNode* start = dynamic_cast<const MultiNode*>(seg->getStart());
	const MultiNode* end   = dynamic_cast<const MultiNode*>(seg->getEnd());
	if (!start && !end) {
		return nullptr;
	}

	//Easy case
	if (start && !end) {
		return start;
	}
	if (end && !start) {
		return end;
	}

	//Slightly harder case: compare distances.
	double dStart = GetShortestDistance(start->getLocation(), cr->nearLine.first, cr->nearLine.second, cr->farLine.first, cr->farLine.second);
	double dEnd   = GetShortestDistance(end->getLocation(),   cr->nearLine.first, cr->nearLine.second, cr->farLine.first, cr->farLine.second);
	return dStart < dEnd ? start : end;
}

} //End un-named namespace

void StreetDirectory::Impl::buildLookups(const std::vector<RoadSegment*>& roadway, std::set<const Crossing*> completed)
{
	//Scan for each crossing (note: this copies the ShortestPathImpl_ somewhat, may want to consolidate later).
	for (std::vector<RoadSegment*>::const_iterator segIt=roadway.begin(); segIt!=roadway.end(); segIt++) {
		for (std::map<centimeter_t, const RoadItem*>::const_iterator riIt=(*segIt)->obstacles.begin(); riIt!=(*segIt)->obstacles.end(); riIt++) {
			//Check if it's a crossing; check if we've already processed it; tag it.
			const Crossing* cr = dynamic_cast<const Crossing*>(riIt->second);
			if (!cr || completed.find(cr)!=completed.end()) {
				continue;
			}
			completed.insert(cr);

			//Find whatever MultiNode is closest.
			const MultiNode* atNode = FindNearestMultiNode(*segIt, cr);
			if (atNode) {
				//Tag it.
				crossings_to_multinodes[cr] = atNode;
			}
		}
	}
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



const MultiNode* StreetDirectory::Impl::GetCrossingNode(const Crossing* cross) const
{
	std::map<const Crossing*, const MultiNode*>::const_iterator res = crossings_to_multinodes.find(cross);
	if (res!=crossings_to_multinodes.end()) {
		return res->second;
	}
	return nullptr;
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
    GetShortestDrivingPath(Node const & fromNode, Node const & toNode) const;

    std::vector<WayPoint>
    shortestWalkingPath(Point2D const & fromPoint, Point2D const & toPoint) const;

    void updateEdgeProperty();
    void GeneratePathChoiceSet();

private:
    // We attach a property to each vertex, its name.  We treat the name property as a mean
    // to identify the vertex.  However instead of a textual name, the actual value that we
    // attach to the vertex's name is the pointer to the Node where the vertex is located.
    // When we search the graph for a vertex, we use the Node::location in the search.
    typedef boost::property<boost::vertex_name_t, Node const *> VertexProperties;

    // Each edge has a weight property cascaded with a name property.  Just like the vertex's
    // name property, the edge's name property is the pointer to WayPoint.  The weight property is
    // the length of the road-segment, side-walk or crossing that each edge represent.
//    typedef boost::property<boost::edge_weight_t, centimeter_t,
//                boost::property<boost::edge_name_t, WayPoint> > EdgeProperties;
    typedef boost::property<boost::edge_weight_t, double,
    		boost::property<boost::edge_name_t, WayPoint> > EdgeProperties;

    // The graph contains arrays of integers for the vertices and edges and is a directed graph.
    typedef boost::adjacency_list<boost::vecS,
                                  boost::vecS,
                                  boost::directedS,
                                  VertexProperties,
                                  EdgeProperties> Graph;
    typedef Graph::vertex_descriptor Vertex;  // A vertex is an integer into the vertex array.
    typedef Graph::edge_descriptor Edge; // An edge is an integer into the edge array.

    ///Helper class:identify a Node exactly. This requires the incoming/outgoing RoadSegment(s), and the generated Vertex.
    struct NodeDescriptor {
    	const RoadSegment* before;
    	const RoadSegment* after;
    	int beforeLaneID; //Only used by the Walking graph
    	int afterLaneID;  //Only used by the Walking graph
    	Point2D tempPos; //Only used by Walking graph UniNodes, temporarily.
    	Vertex v;

    	//Equality comparison (be careful here!)
    	bool operator== (const NodeDescriptor& other) const {
    		return  (this->before==other.before) && (this->after==other.after) &&
    				(this->beforeLaneID==other.beforeLaneID) && (this->afterLaneID==other.afterLaneID);
    	}
    };

    ///Helper class:represents a vertex in our graph as a "Node" in Sim Mobility. Maintains mappings to the
    ///   original node and, if applicable, to any offshoot (child) nodes.
    struct VertexLookup {
    	const sim_mob::Node* origNode;
    	bool isUni;

    	//Lookup information. This is interpreted in two different ways:
    	//  1) UniNodes store the segment before/after this Node. This is inherent in the structure of the UniNode itself.
    	//  2) MultiNodes store the segment before/after this Intersection. This is derived from the Lane Connectors.
    	std::vector<NodeDescriptor> vertices;
    };

public:
    //Print using the old output format.
    void printGraph(const std::string& graphType, const Graph& graph);

public:
    Graph drivingMap_; // A map for drivers, containing road-segments as edges.
    Graph walkingMap_; // A map for pedestrians, containing side-walks and crossings as edges.
    std::vector<Node *> nodes_; // "Internal" uni-nodes that are created when building the maps.
    std::vector<std::vector<std::vector<std::vector<WayPoint> > > > choiceSet;

private:
    //Initialize
#ifndef STDIR_FIX_BROKEN
    void initNetworkOld(const std::vector<Link*>& links);
#endif
    void initDrivingNetworkNew(const std::vector<Link*>& links);
    void initWalkingNetworkNew(const std::vector<Link*>& links);

    //New processing code: Driving path
    void procAddDrivingNodes(Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddDrivingLinks(Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddDrivingLaneConnectors(Graph& graph, const MultiNode* node, const std::map<const Node*, VertexLookup>& nodeLookup);

    //New processing code: Walking path
    void procAddWalkingNodes(Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup, std::map<const Node*, VertexLookup>& tempNodes);
    void procResolveWalkingMultiNodes(Graph& graph, const std::map<const Node*, VertexLookup>& unresolvedNodes, std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddWalkingLinks(Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup);
    void procAddWalkingCrossings(Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::set<const Crossing*>& completed);

    //Old processing code
    void process(std::vector<RoadSegment*> const & roads, bool isForward);
    void process(RoadSegment const * road, bool isForward);
#ifndef STDIR_FIX_BROKEN
    void linkCrossingToRoadSegment(RoadSegment *road, bool isForward);
#endif
    void clearChoiceSet();

    bool checkIfExist(std::vector<std::vector<WayPoint> > & paths, std::vector<WayPoint> & path);
    // Oh wow!  Overloaded functions with different return types.
    Node const * findVertex(Graph const & graph, Point2D const & point, centimeter_t distance) const;
    Vertex findVertex(Graph const & graph, Node const * node) const;
    Vertex findVertex(Point2D const & point);

    Node const * findNode(Point2D const & point);

    void addRoadEdge(Node const * node1, Node const * node2,
                     WayPoint const & wp, centimeter_t length);

    void addRoadEdgeWithTravelTime(Node const * node1, Node const * node2,
    		         WayPoint const & wp, double travelTime);



    void addSideWalk(Lane const * sideWalk, centimeter_t length);
    void addCrossing(Crossing const * crossing, centimeter_t length);

    void getVertices(Vertex & fromVertex, Vertex & toVertex,
                     Node const & fromNode, Node const & toNode) const;

    void getVertices(Vertex & fromVertex, Vertex & toVertex,
                     Point2D const & fromPoint, Point2D const & toPoint) const;

    bool checkVertices(Vertex const fromVertex, Vertex const toVertex,
                       Node const & fromNode, Node const & toNode) const;

    std::vector<WayPoint>
    extractShortestPath(Vertex const fromVertex, Vertex const toVertex,
                        std::vector<Vertex> const & parent, Graph const & graph) const;

    std::vector<WayPoint>
    shortestPath(Vertex const fromVertex, Vertex const toVertex, Graph const & graph) const;

    std::vector<WayPoint>
    extractShortestPath(Vertex const fromVertex, Vertex const toVertex,
    		std::vector<Vertex> const & parent, Graph const & graph, std::vector<Edge> & edges) const;
};





StreetDirectory::ShortestPathImpl::~ShortestPathImpl()
{
    // Cleanup to avoid memory leakage.
    for (size_t i = 0; i < nodes_.size(); ++i)
    {
        Node * node = nodes_[i];
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
        if (closeBy(node->location, point, distance))
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

    Node * n = new UniNode(point.getX(), point.getY());
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
// TODO: The above may need to change. ~Seth
//
// We assume that if a link has any crossing (signalized or zebra crossing), the link is
// split into several road-segments, split at the crossing.  Hence we assume that the
// crossing at the beginning or end of a road segment.  Therefore, side walks are not split
// by crossings.  This assumption needs to be reviewed.
// TODO: Zebra crossings complicate this; we should never have the same segment twice in a row in
//       a returned path. ~Seth

#ifndef STDIR_FIX_BROKEN
inline void
StreetDirectory::ShortestPathImpl::process(std::vector<RoadSegment*> const & roads, bool isForward)
{
	for (size_t i = 0; i < roads.size(); ++i)
	{
		linkCrossingToRoadSegment(const_cast<RoadSegment*>(roads[i]),isForward);
		process(roads[i], isForward);
	}
}
#endif

void StreetDirectory::ShortestPathImpl::procAddDrivingNodes(Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Scan each pair of RoadSegments at each Node (the Node forms the joint between these two). This includes "null" options (for the first/last node).
	//So, (null, X) is the first Node (before segment X), and (Y, null) is the last one. (W,Z) is the Node between segments W and Z.
	for (size_t i=0; i<=roadway.size(); i++) {
		//before/after/node/isUni forms a complete Node descriptor.
		NodeDescriptor nd;
		nd.before = (i==0) ? nullptr : const_cast<RoadSegment*>(roadway.at(i-1));
		nd.after  = (i>=roadway.size()) ? nullptr : const_cast<RoadSegment*>(roadway.at(i));
		const Node* origNode = nd.before ? nd.before->getEnd() : nd.after->getStart();
		if (nodeLookup.count(origNode)==0) {
			nodeLookup[origNode] = VertexLookup();
			nodeLookup[origNode].origNode = origNode;
			nodeLookup[origNode].isUni = dynamic_cast<const UniNode*>(origNode);
		}

		//Construction varies drastically depending on whether or not this is a UniNode
		if (nodeLookup[origNode].isUni) {
			//We currently don't allow U-turns at UniNodes, so for now each unique Node descriptor represents a unique path.
			nd.v = boost::add_vertex(const_cast<Graph &>(graph));
			nodeLookup[origNode].vertices.push_back(nd);

			//We'll create a fake Node for this location (so it'll be represented properly). Once we've fully switched to the
			//  new algorithm, we'll have to switch this to a value-based type; using "new" will leak memory.
			Point2D newPos;
			//TODO: re-enable const after fixing RoadNetwork's sidewalks.
			if (!nd.before && nd.after) {
				newPos = const_cast<RoadSegment*>(nd.after)->getLaneEdgePolyline(1).front();
			} else if (nd.before && !nd.after) {
				newPos = const_cast<RoadSegment*>(nd.before)->getLaneEdgePolyline(1).back();
			} else {
				//Estimate
				DynamicVector vec(const_cast<RoadSegment*>(nd.before)->getLaneEdgePolyline(1).back(), const_cast<RoadSegment*>(nd.after)->getLaneEdgePolyline(1).front());
				vec.scaleVectTo(vec.getMagnitude()/2.0).translateVect();
				newPos = Point2D(vec.getX(), vec.getY());
			}

			//TODO: Leaks memory!
			Node* vNode = new UniNode(newPos.getX(), newPos.getY());
			boost::put(boost::vertex_name, const_cast<Graph &>(graph), nd.v, vNode);
		} else {
			//Each incoming and outgoing RoadSegment has exactly one Node at the Intersection. In this case, the unused before/after
			//   RoadSegment is used to identify whether this is an incoming or outgoing Vertex.
			nd.v = boost::add_vertex(const_cast<Graph &>(graph));
			nodeLookup[origNode].vertices.push_back(nd);

			//Our Node positions are actually the same compared to UniNodes; we may merge this code later.
			Point2D newPos;
			if (!nd.before && nd.after) {
				newPos = const_cast<RoadSegment*>(nd.after)->getLaneEdgePolyline(1).front();
			} else if (nd.before && !nd.after) {
				newPos = const_cast<RoadSegment*>(nd.before)->getLaneEdgePolyline(1).back();
			} else {
				//This, however, is different.
				throw std::runtime_error("MultiNode vertices can't have both \"before\" and \"after\" segments.");
			}

			//TODO: Leaks memory!
			Node* vNode = new UniNode(newPos.getX(), newPos.getY());
			boost::put(boost::vertex_name, const_cast<Graph &>(graph), nd.v, vNode);
		}
	}
}


void StreetDirectory::ShortestPathImpl::procAddDrivingLinks(Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Here, we are simply assigning one Edge per RoadSegment in the Link. This is mildly complicated by the fact that a Node*
	//  may be represented by multiple vertices; overall, though, it's a conceptually simple procedure.
	for (std::vector<RoadSegment*>::const_iterator it=roadway.begin(); it!=roadway.end(); it++) {
		const RoadSegment* rs = *it;
		std::map<const Node*, VertexLookup>::const_iterator from = nodeLookup.find(rs->getStart());
		std::map<const Node*, VertexLookup>::const_iterator to = nodeLookup.find(rs->getEnd());
		if (from==nodeLookup.end() || to==nodeLookup.end()) {
			throw std::runtime_error("Road Segment's nodes are unknown by the vertex map.");
		}
		if (from->second.vertices.empty() || to->second.vertices.empty()) {
			std::cout <<"Warning: Road Segment's nodes have no known mapped vertices." <<std::endl;
			continue;
		}

		//For simply nodes, this will be sufficient.
		Vertex fromVertex = from->second.vertices.front().v;
		Vertex toVertex = to->second.vertices.front().v;

		//If there are multiple options, search for the right one.
		//To accomplish this, just match our "before/after" tagged data. Note that before/after may be null.
		if (from->second.vertices.size()>1) {
			bool error=true;
			for (std::vector<NodeDescriptor>::const_iterator it=from->second.vertices.begin(); it!=from->second.vertices.end(); it++) {
				if (rs == it->after) {
					fromVertex = it->v;
					error = false;
				}
			}
			if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"from\" vertex map."); }
		}
		if (to->second.vertices.size()>1) {
			bool error=true;
			for (std::vector<NodeDescriptor>::const_iterator it=to->second.vertices.begin(); it!=to->second.vertices.end(); it++) {
				if (rs == it->before) {
					toVertex = it->v;
					error = false;
				}
			}
			if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"to\" vertex map."); }
		}

		//Create an edge.
	    Edge edge;
	    bool ok;
	    boost::tie(edge, ok) = boost::add_edge(fromVertex, toVertex, graph);
	    boost::put(boost::edge_name, graph, edge, WayPoint(rs));
	    boost::put(boost::edge_weight, graph, edge, rs->length);
	}
}

void StreetDirectory::ShortestPathImpl::procAddDrivingLaneConnectors(Graph& graph, const MultiNode* node, const std::map<const Node*, VertexLookup>& nodeLookup)
{
	//Skip nulled Nodes (may be UniNodes).
	if (!node) {
		return;
	}

	//We actually only care about RoadSegment->RoadSegment connections.
	std::set< std::pair<RoadSegment*, RoadSegment*> > connectors;
	for (std::map<const sim_mob::RoadSegment*, std::set<sim_mob::LaneConnector*> >::const_iterator conIt=node->getConnectors().begin(); conIt!=node->getConnectors().end(); conIt++) {
		for (std::set<sim_mob::LaneConnector*>::const_iterator it=conIt->second.begin(); it!=conIt->second.end(); it++) {
			connectors.insert(std::make_pair((*it)->getLaneFrom()->getRoadSegment(), (*it)->getLaneTo()->getRoadSegment()));
		}
	}

	//Now, add each "RoadSegment" connector.
	for (std::set< std::pair<RoadSegment*, RoadSegment*> >::iterator it=connectors.begin(); it!=connectors.end(); it++) {
		//Sanity check:
		if (it->first->getEnd()!=node || it->second->getStart()!=node) {
			throw std::runtime_error("Node/Road Segment mismatch in Edge constructor.");
		}

		//Various bookkeeping requirements:
		std::pair<Vertex, bool> fromVertex;
		fromVertex.second = false;
		std::pair<Vertex, bool> toVertex;
		toVertex.second = false;
		std::map<const Node*, VertexLookup>::const_iterator vertCandidates = nodeLookup.find(node);
		if (vertCandidates==nodeLookup.end()) {
			throw std::runtime_error("Intersection's Node is unknown by the vertex map.");
		}

		//Find the "from" and "to" segments' associated end vertices. Keep track of each.
		for (std::vector<NodeDescriptor>::const_iterator ndIt=vertCandidates->second.vertices.begin(); ndIt!=vertCandidates->second.vertices.end(); ndIt++) {
			if (it->first == ndIt->before) {
				fromVertex.first = ndIt->v;
				fromVertex.second = true;
			}
			if (it->second == ndIt->after) {
				toVertex.first = ndIt->v;
				toVertex.second = true;
			}
		}

		//Ensure we have both
		if (!fromVertex.second || !toVertex.second) {
			throw std::runtime_error("Lane connector has no associated vertex.");
		}

		//Create an edge.
	    Edge edge;
	    bool ok;
	    boost::tie(edge, ok) = boost::add_edge(fromVertex.first, toVertex.first, graph);

	    //Calculate the edge length. Treat this as a Node WayPoint.
	    DynamicVector lc(fromVertex.second, toVertex.second);
	    boost::put(boost::edge_name, graph, edge, WayPoint(node));
	    boost::put(boost::edge_weight, graph, edge, lc.getMagnitude());
	}
}


namespace {

//Helper function: Retrieve a set of sidewalk lane pairs (fromLane, toLane) given two RoadSegments.
//If both inputs are non-null, then from/to *must* exist (e.g., UniNodes).
//TODO: Right now, this function is quite hackish, and only checks the outer and inner lanes.
//      We need to improve it to work for any number of sidewalk lanes (e.g., median sidewalks), but
//      for now we don't even have the data.
//TODO: The proper way to do this is with an improved version of UniNode lane connectors.
std::vector< std::pair<int, int> > GetSidewalkLanePairs(const RoadSegment* before, const RoadSegment* after) {
	//Error check: at least one segment must exist
	if (!before && !after) { throw std::runtime_error("Can't GetSidewalkLanePairs on two null segments."); }

	//Store two partial lists
	std::vector<int> beforeLanes;
	std::vector<int> afterLanes;
	std::vector< std::pair<int, int> > res;

	//Build up before
	if (before) {
		for (size_t i=0; i<before->getLanes().size(); i++) {
			if (before->getLanes().at(i)->is_pedestrian_lane()) {
				beforeLanes.push_back(i);
			}
		}
	}

	//Build up after
	if (after) {
		for (size_t i=0; i<after->getLanes().size(); i++) {
			if (after->getLanes().at(i)->is_pedestrian_lane()) {
				afterLanes.push_back(i);
			}
		}
	}

	//It's possible that we have no results
	if ((before&&beforeLanes.empty()) || (after&&afterLanes.empty())) {
		return res;
	}

	//If we have both before and after, only pairs can be added (no null values).
	// We can manage this implicitly by either counting up or down, and stopping when we have no more values.
	// For now, we just ensure they're equal or add NONE
	if (before && after) {
		if (beforeLanes.size()==afterLanes.size()) {
			for (size_t i=0; i<beforeLanes.size(); i++) {
				res.push_back(std::make_pair(beforeLanes.at(i), afterLanes.at(i)));
			}
		}
		return res;
	}

	//Otherwise, just build a partial list
	for (size_t i=0; i<beforeLanes.size() || i<afterLanes.size(); i++) {
		if (before) {
			res.push_back(std::make_pair(beforeLanes.at(i), -1));
		} else {
			res.push_back(std::make_pair(-1, afterLanes.at(i)));
		}
	}
	return res;
}

} //End un-named namespace
void StreetDirectory::ShortestPathImpl::procAddWalkingNodes(Graph& graph, const std::vector<RoadSegment*>& roadway, std::map<const Node*, VertexLookup>& nodeLookup, std::map<const Node*, VertexLookup>& tempNodes)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Scan each pair of RoadSegments at each Node (the Node forms the joint between these two). This includes "null" options (for the first/last node).
	//So, (null, X) is the first Node (before segment X), and (Y, null) is the last one. (W,Z) is the Node between segments W and Z.
	for (size_t i=0; i<=roadway.size(); i++) {
		//before/after/node/isUni forms a complete Node descriptor.
		NodeDescriptor nd;
		nd.before = (i==0) ? nullptr : const_cast<RoadSegment*>(roadway.at(i-1));
		nd.after  = (i>=roadway.size()) ? nullptr : const_cast<RoadSegment*>(roadway.at(i));
		const Node* origNode = nd.before ? nd.before->getEnd() : nd.after->getStart();
		if (nodeLookup.count(origNode)==0) {
			nodeLookup[origNode] = VertexLookup();
			nodeLookup[origNode].origNode = origNode;
			nodeLookup[origNode].isUni = dynamic_cast<const UniNode*>(origNode);
		}

		//Construction varies drastically depending on whether or not this is a UniNode
		if (nodeLookup[origNode].isUni) {
			//There may be several (currently 0, 1 or 2) Pedestrian lanes connecting at this Node. We'll need a Node for each,
			//  since Pedestrians can't normally cross Driving lanes without jaywalking.
			std::vector< std::pair<int, int> > lanePairs = GetSidewalkLanePairs(nd.before, nd.after);

			//Add each potential lane Vertex
			for (std::vector< std::pair<int, int> >::iterator it=lanePairs.begin(); it!=lanePairs.end(); it++) {
				//Copy this node descriptor, modify it by adding in the from/to lanes.
				NodeDescriptor newNd(nd);
				newNd.beforeLaneID = it->first;
				newNd.afterLaneID = it->second;
				newNd.v = boost::add_vertex(const_cast<Graph &>(graph));
				nodeLookup[origNode].vertices.push_back(newNd);

				//We'll create a fake Node for this location (so it'll be represented properly). Once we've fully switched to the
				//  new algorithm, we'll have to switch this to a value-based type; using "new" will leak memory.
				Point2D newPos;
				//TODO: re-enable const after fixing RoadNetwork's sidewalks.
				if (!nd.before && nd.after) {
					newPos = const_cast<RoadSegment*>(nd.after)->getLanes().at(it->second)->getPolyline().front();
				} else if (nd.before && !nd.after) {
					newPos = const_cast<RoadSegment*>(nd.before)->getLanes().at(it->first)->getPolyline().back();
				} else {
					//Estimate
					DynamicVector vec(const_cast<RoadSegment*>(nd.before)->getLanes().at(it->first)->getPolyline().back(), const_cast<RoadSegment*>(nd.after)->getLanes().at(it->second)->getPolyline().front());
					vec.scaleVectTo(vec.getMagnitude()/2.0).translateVect();
					newPos = Point2D(vec.getX(), vec.getY());
				}

				//TODO: Leaks memory!
				Node* vNode = new UniNode(newPos.getX(), newPos.getY());
				boost::put(boost::vertex_name, const_cast<Graph &>(graph), newNd.v, vNode);
			}
		} else {
			//MultiNodes are much more complex. For now, we just collect all vertices into a "potential" list.
			//Fortunately, we only have to scan one list this time.
			std::vector< std::pair<int, int> > lanePairs = GetSidewalkLanePairs(nd.before, nd.after);

			//Make sure our temp lookup list has this.
			if (tempNodes.count(origNode)==0) {
				tempNodes[origNode] = VertexLookup();
				tempNodes[origNode].origNode = origNode;
				tempNodes[origNode].isUni = nodeLookup[origNode].isUni;
			}

			for (std::vector< std::pair<int, int> >::iterator it=lanePairs.begin(); it!=lanePairs.end(); it++) {
				//Copy this node descriptor, modify it by adding in the from/to lanes.
				NodeDescriptor newNd(nd);
				newNd.beforeLaneID = nd.before ? it->first : -1;
				newNd.afterLaneID = nd.after ? it->second : -1;
				//newNd.v = boost::add_vertex(const_cast<Graph &>(graph)); //Don't add it yet.

				//Our Node positions are actually the same compared to UniNodes; we may merge this code later.
				Point2D newPos;
				//TODO: re-enable const after fixing RoadNetwork's sidewalks.
				if (!nd.before && nd.after) {
					newPos = const_cast<RoadSegment*>(nd.after)->getLanes().at(newNd.afterLaneID)->getPolyline().front();
				} else if (nd.before && !nd.after) {
					newPos = const_cast<RoadSegment*>(nd.before)->getLanes().at(newNd.beforeLaneID)->getPolyline().back();
				} else {
					//This, however, is different.
					throw std::runtime_error("MultiNode vertices can't have both \"before\" and \"after\" segments.");
				}

				//Save in an alternate location for now, since we'll merge these later.
				newNd.tempPos = newPos;
				tempNodes[origNode].vertices.push_back(newNd); //Save in our temp list.
			}
		}
	}
}

namespace {
//Helper (we can't use std::set, so we use vector::find)
template <class T>
bool VectorContains(const std::vector<T>& vec, const T& value) {
	return std::find(vec.begin(), vec.end(), value) != vec.end();
}

//Helper: do these two segments start/end at the same pair of nodes?
bool SegmentCompare(const RoadSegment* self, const RoadSegment* other) {
	if ((self->getStart()==other->getStart()) && (self->getEnd()==other->getEnd())) {
		return true;  //Exact same
	}
	if ((self->getStart()==other->getEnd()) && (self->getEnd()==other->getStart())) {
		return true;  //Reversed, but same
	}
	return false; //Different.
}
} //End un-named namespace
void StreetDirectory::ShortestPathImpl::procResolveWalkingMultiNodes(Graph& graph, const std::map<const Node*, VertexLookup>& unresolvedNodes, std::map<const Node*, VertexLookup>& nodeLookup)
{
	//We need to merge the potential vertices at all unresolved MultiNodes. At the moment, this requires some geometric assumption (but for roundabouts, later, this will no longer be acceptible)
	for (std::map<const Node*, VertexLookup>::const_iterator mnIt=unresolvedNodes.begin(); mnIt!=unresolvedNodes.end(); mnIt++) {
		//First, we need to compute the distance between every pair of Vertices.
		const Node* node = mnIt->first;
		std::map<double, std::pair<NodeDescriptor, NodeDescriptor> > distLookup;
		for (std::vector<NodeDescriptor>::const_iterator it1=mnIt->second.vertices.begin(); it1!=mnIt->second.vertices.end(); it1++) {
			for (std::vector<NodeDescriptor>::const_iterator it2=it1+1; it2!=mnIt->second.vertices.end(); it2++) {
				//No need to be exact here; if there are collisions, simply modify the result until it's unique.
				double dist = sim_mob::dist(it1->tempPos, it2->tempPos);
				while (distLookup.count(dist)>0) {
					dist += 0.000001;
				}

				//Save it.
				distLookup[dist] = std::make_pair(*it1, *it2);
			}
		}

		//Iterate in order, pairing the two closest elements if their total distance is less than 1/2 of the maximum distance.
		//Note that map::begin/end is essentially in order. (Also, we keep a list of what's been tagged already).
		std::map<double, std::pair<NodeDescriptor, NodeDescriptor> >::const_iterator lastValue = distLookup.end();
		lastValue--;
		double maxDist = lastValue->first / 2.0;
		std::vector<NodeDescriptor> alreadyMerged;
		for (std::map<double, std::pair<NodeDescriptor, NodeDescriptor> >::const_iterator it=distLookup.begin(); it!=distLookup.end(); it++) {
			//Find a Vertex we haven't merged yet.
			if (VectorContains(alreadyMerged, it->second.first) || VectorContains(alreadyMerged, it->second.second)) {
				continue;
			}

			//Now check the distance between our two candidate Vertices.
			if (it->first > maxDist) {
				break; //All distances after this will be greater, since the map is sorted.
			}

			//Create a new Node Descriptor for this merged Node. "before" and "after" are arbitrary, since Pedestrians can walk bidirectionally on their edges.
			NodeDescriptor newNd;
			newNd.before = it->second.first.before ? it->second.first.before : it->second.first.after;
			newNd.after = it->second.second.before ? it->second.second.before : it->second.second.after;
			newNd.beforeLaneID = it->second.first.before ? it->second.first.beforeLaneID : it->second.first.afterLaneID;
			newNd.afterLaneID = it->second.second.before ? it->second.second.beforeLaneID : it->second.second.afterLaneID;

			//Heuristical check: If the two segments in question start/end at the same pair of nodes, then don't add this
			//  (we must use Crossings in this case).
			//Note that this won't catch cases where additional Nodes are added, but it will also never cause any harm.
			if (SegmentCompare(newNd.before, newNd.after)) {
				continue;
			}

			//Add it to our boost::graph
			newNd.v = boost::add_vertex(const_cast<Graph &>(graph));

			//Put the actual point halfway between the two candidate points.
			DynamicVector vec(it->second.first.tempPos, it->second.second.tempPos);
			vec.scaleVectTo(vec.getMagnitude()/2.0).translateVect();
			Node* vNode = new UniNode(vec.getX(), vec.getY());   //TODO: Leaks memory!
			boost::put(boost::vertex_name, const_cast<Graph &>(graph), newNd.v, vNode);

			//Tag each unmerged Vertex so that we don't re-use them.
			alreadyMerged.push_back(it->second.first);
			alreadyMerged.push_back(it->second.second);

			//Also add this to our list of known vertices, so that we can find it later.
			nodeLookup[node].vertices.push_back(newNd);
		}

		//Finally, some Nodes may not have been merged at all. Just add these as-is.
		for (std::vector<NodeDescriptor>::const_iterator it=mnIt->second.vertices.begin(); it!=mnIt->second.vertices.end(); it++) {
			if (VectorContains(alreadyMerged, *it)) {
				continue;
			}

			//before/after should be set properly in this case.
			NodeDescriptor newNd(*it);
			newNd.v = boost::add_vertex(const_cast<Graph &>(graph));
			Node* vNode = new UniNode(it->tempPos.getX(), it->tempPos.getY());   //TODO: Leaks memory!
			boost::put(boost::vertex_name, const_cast<Graph &>(graph), newNd.v, vNode);

			//Save it for later.
			nodeLookup[node].vertices.push_back(newNd);
		}
	}
}



void StreetDirectory::ShortestPathImpl::procAddWalkingLinks(Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup)
{
	//Skip empty roadways
	if (roadway.empty()) {
		return;
	}

	//Here, we are simply assigning one Edge per RoadSegment in the Link. This is mildly complicated by the fact that a Node*
	//  may be represented by multiple vertices; overall, though, it's a conceptually simple procedure.
	//Note that Walking edges are two-directional; for now, we accomplish this by adding 2 edges (we can change it to an undirected graph later).
	for (std::vector<RoadSegment*>::const_iterator it=roadway.begin(); it!=roadway.end(); it++) {
		const RoadSegment* rs = *it;
		std::map<const Node*, VertexLookup>::const_iterator from = nodeLookup.find(rs->getStart());
		std::map<const Node*, VertexLookup>::const_iterator to = nodeLookup.find(rs->getEnd());
		if (from==nodeLookup.end() || to==nodeLookup.end()) {
			throw std::runtime_error("Road Segment's nodes are unknown by the vertex map.");
		}
		if (from->second.vertices.empty() || to->second.vertices.empty()) {
			std::cout <<"Warning: Road Segment's nodes have no known mapped vertices." <<std::endl;
			continue;
		}

		//Of course, we still need to deal with Lanes
		std::vector< std::pair<int, int> > lanePairs = GetSidewalkLanePairs(rs, nullptr);
		for (std::vector< std::pair<int, int> >::iterator it=lanePairs.begin(); it!=lanePairs.end(); it++) {
			int laneID = it->first;
			//For simply nodes, this will be sufficient.
			Vertex fromVertex = from->second.vertices.front().v;
			Vertex toVertex = to->second.vertices.front().v;

			//If there are multiple options, search for the right one.
			//Note that for walking nodes, before OR after may match (due to the way we merge MultiNodes).
			//Note that before/after may be null.
			if (from->second.vertices.size()>1) {
				bool error=true;
				for (std::vector<NodeDescriptor>::const_iterator it=from->second.vertices.begin(); it!=from->second.vertices.end(); it++) {
					if ((rs==it->after && laneID==it->afterLaneID) || (rs==it->before && laneID==it->beforeLaneID)) {
						fromVertex = it->v;
						error = false;
					}
				}
				if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"from\" vertex map."); }
			}
			if (to->second.vertices.size()>1) {
				bool error=true;
				for (std::vector<NodeDescriptor>::const_iterator it=to->second.vertices.begin(); it!=to->second.vertices.end(); it++) {
					if ((rs==it->before && laneID==it->beforeLaneID) || (rs==it->after && laneID==it->afterLaneID)) {
						toVertex = it->v;
						error = false;
					}
				}
				if (error) { throw std::runtime_error("Unable to find Node with proper outgoing RoadSegment in \"to\" vertex map."); }
			}

			//Create an edge.
			{
			Edge edge;
			bool ok;
			boost::tie(edge, ok) = boost::add_edge(fromVertex, toVertex, graph);
			boost::put(boost::edge_name, graph, edge, WayPoint(rs));
			boost::put(boost::edge_weight, graph, edge, rs->length);
			}

			//Create the reverse edge
			{
			Edge edge;
			bool ok;
			boost::tie(edge, ok) = boost::add_edge(toVertex, fromVertex, graph);
			boost::put(boost::edge_name, graph, edge, WayPoint(rs));
			boost::put(boost::edge_weight, graph, edge, rs->length);
			}
		}
	}
}


void StreetDirectory::ShortestPathImpl::procAddWalkingCrossings(Graph& graph, const std::vector<RoadSegment*>& roadway, const std::map<const Node*, VertexLookup>& nodeLookup, std::set<const Crossing*>& completed)
{
	//Skip empty paths
	if (roadway.empty()) {
		return;
	}

	//We need to scan each RoadSegment in our roadway for any possible Crossings. The "nextObstacle" function can do this.
	for (std::vector<RoadSegment*>::const_iterator segIt=roadway.begin(); segIt!=roadway.end(); segIt++) {
		//NOTE: For now, it's just easier to scan the obstacles list manually.
		for (std::map<centimeter_t, const RoadItem*>::const_iterator riIt=(*segIt)->obstacles.begin(); riIt!=(*segIt)->obstacles.end(); riIt++) {
			//Check if it's a crossing; check if we've already processed it; tag it.
			const Crossing* cr = dynamic_cast<const Crossing*>(riIt->second);
			if (!cr || completed.find(cr)!=completed.end()) {
				continue;
			}
			completed.insert(cr);

			//At least one of the Segment's endpoints must be a MultiNode. Pick the closest one.
			//TODO: Currently we can only handle Crossings at the ends of RoadSegments.
			//      Zebra crossings require either a UniNode or a different approach entirely.
			const MultiNode* atNode = FindNearestMultiNode(*segIt, cr);
			if (!atNode) {
				//TODO: We have a UniNode with a crossing; we should really add this later.
				std::cout <<"Warning: Road Segment has a Crossing, but neither a start nor end MultiNode. Skipping for now." <<std::endl;
				continue;
			}

			//Crossings must from one Lane to another Lane in order to be useful.
			//Therefore, we need to find the "reverse" road segment to this one.
			//This can technically be the same segment, if it's a one-way street (it'll have a different laneID).
			//We will still use the same "from/to" syntax to keep things simple.
			const RoadSegment* fromSeg = *segIt;
			int fromLane = -1;
			for (size_t i=0; i<fromSeg->getLanes().size(); i++) {
				if (fromSeg->getLanes().at(i)->is_pedestrian_lane()) {
					fromLane = i;
					break;
				}
			}
			if (fromLane==-1) { throw std::runtime_error("Sanity check failed: Crossing should not be generated with no sidewalk lane."); }

			//Now find the "to" lane. This is optional.
			//TODO: This all needs to be stored at a higher level later; a Crossing doesn't always have to cross Segments with
			//      the same start/end Nodes.
			const RoadSegment* toSeg = nullptr;
			int toLane = -1;
			for (std::set<RoadSegment*>::const_iterator it=atNode->getRoadSegments().begin(); toLane==-1 && it!=atNode->getRoadSegments().end(); it++) {
				//Light matching criteria
				toSeg = *it;
				if ((toSeg->getStart()==fromSeg->getStart() && toSeg->getEnd()==fromSeg->getEnd()) ||
					(toSeg->getStart()==fromSeg->getEnd() && toSeg->getEnd()==fromSeg->getStart())) {
					//Scan lanes until we find an empty one (this covers the case where fromSeg and toSeg are the same).
					for (size_t i=0; i<toSeg->getLanes().size(); i++) {
						if (toSeg->getLanes().at(i)->is_pedestrian_lane()) {
							//Avoid adding the exact same from/to pair:
							if (fromSeg==toSeg && fromLane==i) {
								continue;
							}

							//It's unique; add it.
							toLane = i;
							break;
						}
					}
				}
			}

			//If we have something, add this crossing as a pair of edges.
			if (toLane!=-1) {
				std::cout <<"Adding Crossing: \n";
				std::cout <<"  from: " <<fromSeg <<" lane: " <<fromLane <<std::endl;
				std::cout <<"  to:   " <<toSeg <<" lane: " <<toLane <<std::endl;

				//First, retrieve the fromVertex and toVertex
				std::pair<Vertex, bool> fromVertex;
				fromVertex.second = false;
				std::pair<Vertex, bool> toVertex;
				toVertex.second = false;
				std::map<const Node*, VertexLookup>::const_iterator vertCandidates = nodeLookup.find(atNode);
				if (vertCandidates==nodeLookup.end()) {
					throw std::runtime_error("Intersection's Node is unknown by the vertex map.");
				}

				//Find the "from" and "to" segments' associated end vertices.
				//In this case, we only need a weak guarantee (e.g., that ONE of the before/after pair matches our segment).
				//(But we also need the strong guarantee of Lane IDs).
				for (std::vector<NodeDescriptor>::const_iterator ndIt=vertCandidates->second.vertices.begin(); ndIt!=vertCandidates->second.vertices.end(); ndIt++) {
					if ((fromSeg==ndIt->before && fromLane==ndIt->beforeLaneID) || (fromSeg==ndIt->after && fromLane==ndIt->afterLaneID)) {
						fromVertex.first = ndIt->v;
						fromVertex.second = true;
					}
					if ((toSeg==ndIt->before && toLane==ndIt->beforeLaneID) || (toSeg==ndIt->after && toLane==ndIt->afterLaneID)) {
						toVertex.first = ndIt->v;
						toVertex.second = true;
					}
				}

				//Ensure we have both
				if (!fromVertex.second || !toVertex.second) {
					throw std::runtime_error("Crossing has no associated vertex.");
				}

				//Estimate the length of the crossing.
				double length = sim_mob::dist(cr->nearLine.first, cr->nearLine.second);

				//Create an edge.
				{
				Edge edge;
				bool ok;
				boost::tie(edge, ok) = boost::add_edge(fromVertex.first, toVertex.first, graph);
				boost::put(boost::edge_name, graph, edge, WayPoint(cr));
				boost::put(boost::edge_weight, graph, edge, length);
				}

				//Create the reverse edge
				{
				Edge edge;
				bool ok;
				boost::tie(edge, ok) = boost::add_edge(toVertex.first, fromVertex.first, graph);
				boost::put(boost::edge_name, graph, edge, WayPoint(cr));
				boost::put(boost::edge_weight, graph, edge, length);
				}
			}
		}
	}
}


inline StreetDirectory::ShortestPathImpl::ShortestPathImpl(RoadNetwork const & network)
{
#ifdef STDIR_FIX_BROKEN
	initDrivingNetworkNew(network.getLinks());
	initWalkingNetworkNew(network.getLinks());
#else
	initNetworkOld(network.getLinks());
#endif
//    GeneratePathChoiceSet();
}

#ifndef STDIR_FIX_BROKEN
void StreetDirectory::ShortestPathImpl::initNetworkOld(const std::vector<Link*>& links)
{
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter)
    {
        Link const * link = *iter;
        process(link->getPath(true), true);
        process(link->getPath(false), false);
    }
}
#endif

void StreetDirectory::ShortestPathImpl::initDrivingNetworkNew(const std::vector<Link*>& links)
{
	//Various lookup structures
	std::map<const Node*, VertexLookup> nodeLookup;

	//Add our initial set of vertices. Iterate through Links to ensure no un-used Node are added.
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddDrivingNodes(drivingMap_, (*iter)->getPath(true), nodeLookup);
    	procAddDrivingNodes(drivingMap_, (*iter)->getPath(false), nodeLookup);
    }

    //Proceed through our Links, adding each RoadSegment path. Split vertices as required.
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddDrivingLinks(drivingMap_, (*iter)->getPath(true), nodeLookup);
    	procAddDrivingLinks(drivingMap_, (*iter)->getPath(false), nodeLookup);
    }

    //Now add all Intersection edges (lane connectors)
    for (std::map<const Node*, VertexLookup>::const_iterator it=nodeLookup.begin(); it!=nodeLookup.end(); it++) {
    	procAddDrivingLaneConnectors(drivingMap_, dynamic_cast<const MultiNode*>(it->first), nodeLookup);
    }
}

void StreetDirectory::ShortestPathImpl::initWalkingNetworkNew(const std::vector<Link*>& links)
{
	//Various lookup structures
	std::map<const Node*, VertexLookup> nodeLookup;

	{
	//Building MultiNodes requires one additional step.
	std::map<const Node*, VertexLookup> unresolvedNodes;

	//Add our initial set of vertices. Iterate through Links to ensure no un-used Node are added.
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddWalkingNodes(walkingMap_, (*iter)->getPath(true), nodeLookup, unresolvedNodes);
    	procAddWalkingNodes(walkingMap_, (*iter)->getPath(false), nodeLookup, unresolvedNodes);
    }

    //Resolve MultiNodes here:
    procResolveWalkingMultiNodes(walkingMap_, unresolvedNodes, nodeLookup);
	}

    //Proceed through our Links, adding each RoadSegment path. Split vertices as required.
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddWalkingLinks(walkingMap_, (*iter)->getPath(true), nodeLookup);
    	procAddWalkingLinks(walkingMap_, (*iter)->getPath(false), nodeLookup);
    }

    //Now add all Crossings
    {
    std::set<const Crossing*> completedCrossings;
    for (std::vector<Link*>::const_iterator iter = links.begin(); iter != links.end(); ++iter) {
    	procAddWalkingCrossings(walkingMap_, (*iter)->getPath(true), nodeLookup, completedCrossings);
    	procAddWalkingCrossings(walkingMap_, (*iter)->getPath(false), nodeLookup, completedCrossings);
    }
    }
}

#ifndef STDIR_FIX_BROKEN
void StreetDirectory::ShortestPathImpl::linkCrossingToRoadSegment(RoadSegment *road, bool isForward)
{
	centimeter_t offset = 0;
	while (offset < road->length)
	{
		RoadItemAndOffsetPair pair = road->nextObstacle(offset, isForward);
		if (0 == pair.item)
		{
			offset = road->length;
			break;
		}


		if (Crossing * crossing = const_cast<Crossing*>(dynamic_cast<Crossing const *>(pair.item)))
		{
			crossing->setRoadSegment(road);
		}

		offset = pair.offset + 1;
	}
}
#endif


void
StreetDirectory::ShortestPathImpl::process(RoadSegment const * road, bool isForward)
{
	double avgSpeed;
	std::map<const RoadSegment*, double>::iterator avgSpeedRSMapIt;
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
        	offset = road->length;
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
            Node * node2 = new UniNode(pos.getX(), pos.getY());
            addRoadEdge(node1, node2, WayPoint(busStop), offset);


            avgSpeed = 100*road->maxSpeed/3.6;
            if(avgSpeed<=0)
            	avgSpeed = 10;
            addRoadEdgeWithTravelTime(node1, node2, WayPoint(busStop), offset/avgSpeed);
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


    avgSpeed = 100*road->maxSpeed/3.6;
    if(avgSpeed<=0)
    	avgSpeed = 10;

//    std::cout<<"node1 "<<node1->location.getX()<<" to node2 "<<node2->location.getX()<<" is "<<offset/(100*road->maxSpeed/3.6)<<std::endl;
    addRoadEdge(node1, node2, WayPoint(road), offset);
//    addRoadEdgeWithTravelTime(node1, node2, WayPoint(road), offset/avgSpeed);
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

    //NOTE: With some combinations of boost+gcc+optimizations, this sometimes adds
    //      a null node. Rather than silently crashing, we will directly check the
    //      added value here and explicitly fail if corruption occurred. ~Seth
    //VERY IMPORTANT NOTE: If you are using boost 1.42.0, gcc 4.5.2, and -O2, gcc will
    //     optimize out the previous boost::put and you wiil get a lot of WayPoint::Invalid
    //     edges. The following lines of code ensure that, by checking the value of the inserted
    //     WayPoint, it is not optimized away. This is a bug in gcc, so please do not remove the
    //     following lines of code. ~Seth
    WayPoint cp = boost::get(boost::edge_name, drivingMap_, edge);
    if (cp.type_ != wp.type_) {
    	throw std::runtime_error("StreetDirectory::addRoadEdge; boost::put corrupted data."
    		"This sometimes happens with certain versions of boost, gcc, and optimization level 2.");
    }
}


// Insert a directed edge into the drivingMap_ graph from <node1> to <node2>, which represent
// vertices in the graph.  <wp> is attached to the edge as its name property and <length> as
// its weight property.
void
StreetDirectory::ShortestPathImpl::addRoadEdgeWithTravelTime(Node const * node1, Node const * node2,
                                               WayPoint const & wp, double travelTime)
{
    Vertex u = findVertex(drivingMap_, node1);
    Vertex v = findVertex(drivingMap_, node2);

    Edge edge;
    bool ok;
    boost::tie(edge, ok) = boost::add_edge(u, v, drivingMap_);
    boost::put(boost::edge_name, drivingMap_, edge, wp);
    boost::put(boost::edge_weight, drivingMap_, edge, travelTime);

    //NOTE: With some combinations of boost+gcc+optimizations, this sometimes adds
    //      a null node. Rather than silently crashing, we will directly check the
    //      added value here and explicitly fail if corruption occurred. ~Seth
    //VERY IMPORTANT NOTE: If you are using boost 1.42.0, gcc 4.5.2, and -O2, gcc will
    //     optimize out the previous boost::put and you wiil get a lot of WayPoint::Invalid
    //     edges. The following lines of code ensure that, by checking the value of the inserted
    //     WayPoint, it is not optimized away. This is a bug in gcc, so please do not remove the
    //     following lines of code. ~Seth
    WayPoint cp = boost::get(boost::edge_name, drivingMap_, edge);
    if (cp.type_ != wp.type_) {
    	throw std::runtime_error("StreetDirectory::addRoadEdge; boost::put corrupted data."
    		"This sometimes happens with certain versions of boost, gcc, and optimization level 2.");
    }
}


void
StreetDirectory::ShortestPathImpl::updateEdgeProperty()
{
	double avgSpeed, travelTime;
	std::map<const RoadSegment*, double>::iterator avgSpeedRSMapIt;
	Graph::edge_iterator iter, end;
	for (boost::tie(iter, end) = boost::edges(drivingMap_); iter != end; ++iter)
	{
//		std::cout<<"edge"<<std::endl;
		Edge e = *iter;
		WayPoint wp = boost::get(boost::edge_name, drivingMap_, e);
		if (wp.type_ != WayPoint::ROAD_SEGMENT)
			continue;
		const RoadSegment * rs = wp.roadSegment_;
		avgSpeedRSMapIt = sim_mob::TrafficWatch::instance().getAvgSpeedRS().find(rs);
		if(avgSpeedRSMapIt != sim_mob::TrafficWatch::instance().getAvgSpeedRS().end())
			avgSpeed = avgSpeedRSMapIt->second;
		else
			avgSpeed = 100*rs->maxSpeed/3.6;
		if(avgSpeed<=0)
			avgSpeed = 10;
		travelTime = rs->length / avgSpeed;
		boost::put(boost::edge_weight, drivingMap_, e, travelTime);
	}
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
        Node * n = new UniNode(point.getX(), point.getY());
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
    wp.directionReverse = true;
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
    wp.directionReverse = true;
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
    if (!checkVertices(fromVertex, toVertex, fromNode, toNode)) {
    	//Fallback: If the RoadNetwork knows about the from/to node(s) but the Street Directory
    	//  does not, it is not an error (but it means no path can possibly be found).
    	return std::vector<WayPoint>();
    }

    return shortestPath(fromVertex, toVertex, drivingMap_);
}

std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::GetShortestDrivingPath(Node const & fromNode, Node const & toNode)
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
    if (!checkVertices(fromVertex, toVertex, fromNode, toNode)) {
    	//Fallback: If the RoadNetwork knows about the from/to node(s) but the Street Directory
    	//  does not, it is not an error (but it means no path can possibly be found).
    	return std::vector<WayPoint>();
    }

    //std::cout<<"size of choice set "<<choiceSet[fromVertex][toVertex].size()<<std::endl;
    return choiceSet[fromVertex][toVertex][0];
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
    Node const * to = (dynamic_cast<UniNode const *>(&toNode)) ? &toNode : nullptr;

    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(drivingMap_); iter != end; ++iter)
    {
        Vertex v = *iter;
        Node const * node = boost::get(boost::vertex_name, drivingMap_, v);
        // Uni-nodes were never inserted into the drivingMap_, but some other points close to
        // them.  Hopefully, they are within 10 meters and that the correct vertex is returned,
        // even in narrow links.
        if (from && closeBy(from->location, node->location, 1000))
        {
            fromVertex = v;
        }
        else if (to && closeBy(to->location, node->location, 1000))
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
// Returns false in some cases to indicate that the nodes exist, but cannot be found.
bool
StreetDirectory::ShortestPathImpl::checkVertices(Vertex const fromVertex, Vertex const toVertex,
                                                 Node const & fromNode, Node const & toNode)
const
{
    Graph::vertices_size_type graphSize = boost::num_vertices(drivingMap_);
    if (fromVertex > graphSize || toVertex > graphSize)
    {
    	bool critical = true;
        std::ostringstream stream;
        stream << "StreetDirectory::shortestDrivingPath: "; 
        if (fromVertex > graphSize)
        {
        	critical = true;
            stream << "fromNode=" << fromNode.location << " is not part of the known road network ";

            //Check if the ConfigManager can find it.
            if (ConfigParams::GetInstance().getNetwork().locateNode(fromNode.location, true, 10)) {
            	critical = false;
            	stream <<"  (...but it is listed in the RoadNetwork)";
            }
        }
        if (toVertex > graphSize)
        {
        	critical = true;
            stream << "toNode=" << toNode.location << " is not part of the known road network";

            //Check if the ConfigManager can find it.
            if (ConfigParams::GetInstance().getNetwork().locateNode(toNode.location, true, 10)) {
            	critical = false;
            	stream <<"  (...but it is listed in the RoadNetwork)";
            }
        }

        //Should we actually throw this
        if (critical) {
        	throw std::runtime_error(stream.str().c_str());
        }

        //Either way it's a problem.
        return false;
    }

    //Nothing wrong here.
    return true;
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


std::vector<WayPoint>
StreetDirectory::ShortestPathImpl::extractShortestPath(Vertex const fromVertex,
                                                       Vertex const toVertex,
                                                       std::vector<Vertex> const & parent,
                                                       Graph const & graph,std::vector<Edge> & edges)
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

    edges.clear();
    Vertex v = toVertex;
    Vertex u = parent[v];
    std::vector<WayPoint> result;
    while (u != v)
    {
        Edge e;
        bool exists;
        boost::tie(e, exists) = boost::edge(u, v, graph);
        edges.push_back(e);
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
        h = hypot(fromPoint, node->location);
        if (d1 > h)
        {
            d1 = h;
            fromVertex = v;
        }
        h = hypot(toPoint, node->location);
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
    if (node->location != fromPoint)
        result.push_back(WayPoint(node));
    result.insert(result.end(), path.begin(), path.end());
    node = boost::get(boost::vertex_name, walkingMap_, toVertex);
    if (node->location != toPoint)
        result.push_back(WayPoint(node));
    return result;
}


bool StreetDirectory::ShortestPathImpl::checkIfExist(std::vector<std::vector<WayPoint> > & paths, std::vector<WayPoint> & path)
{
	for(size_t i=0;i<paths.size();i++)
	{
		std::vector<WayPoint> temp = paths.at(i);
		if(temp.size()!=path.size())
			continue;
		bool same = true;
		for(size_t j=0;j<temp.size();j++)
		{
			if(temp.at(j).roadSegment_ != path.at(j).roadSegment_)
			{
				same = false;
				break;
			}
		}
		if(same)
			return true;
	}
	return false;
}

void StreetDirectory::ShortestPathImpl::clearChoiceSet()
{
	choiceSet.clear();

}


void StreetDirectory::ShortestPathImpl::GeneratePathChoiceSet()
{
	clearChoiceSet();
	Graph::vertex_iterator iter, end;
	for (boost::tie(iter, end) = boost::vertices(drivingMap_); iter != end; ++iter)
	{
		Vertex v = *iter;
		Node const * node = boost::get(boost::vertex_name, drivingMap_, v);
		std::vector<std::vector<std::vector<WayPoint> > > paths_from_v;

		std::vector<Vertex> parent(boost::num_vertices(drivingMap_));
		for (Graph::vertices_size_type i = 0; i < boost::num_vertices(drivingMap_); ++i)
			parent[i] = i;

		//std::cout<<"vertex "<<v<<std::endl;
		boost::dijkstra_shortest_paths(drivingMap_, v, boost::predecessor_map(&parent[0]));
		for (Graph::vertices_size_type i = 0; i < boost::num_vertices(drivingMap_); ++i)
		{
			std::vector<std::vector<WayPoint> > temp_paths;
			std::vector<Edge> edges;
			temp_paths.push_back(extractShortestPath(v, i, parent, drivingMap_,edges));
			for(size_t j=0; j<edges.size(); j++)
			{
				centimeter_t length = boost::get(boost::edge_weight, drivingMap_, edges.at(j));
				boost::put(boost::edge_weight, drivingMap_, edges.at(j), 10*length);
				std::vector<Vertex> parent(boost::num_vertices(drivingMap_));
				for (Graph::vertices_size_type k = 0; k < boost::num_vertices(drivingMap_); ++k)
					parent[k] = k;
				boost::dijkstra_shortest_paths(drivingMap_, v, boost::predecessor_map(&parent[0]));
				std::vector<WayPoint> path = extractShortestPath(v, i, parent, drivingMap_);
				if(!checkIfExist(temp_paths, path))
					temp_paths.push_back(path);
				boost::put(boost::edge_weight, drivingMap_, edges.at(j), length);
			}
			//std::cout<<"size of path "<<temp_paths.size()<<std::endl;
			paths_from_v.push_back(temp_paths);
		}
		choiceSet.push_back(paths_from_v);
	}
}

void StreetDirectory::ShortestPathImpl::printGraph(const std::string& graphType, const Graph& graph)
{
	//Print an identifier
	LogOutNotSync("(\"sd-graph\""
		<<","<<0
		<<","<<&graph
		<<",{"
		<<"\"type\":\""<<graphType
		<<"\"})"<<std::endl);

	//Print each vertex
	//NOTE: Vertices appear to just be integers in boost's adjacency lists.
	//      Not sure if we can rely on this (we can use property maps if necessary).
	{
    Graph::vertex_iterator iter, end;
    for (boost::tie(iter, end) = boost::vertices(graph); iter != end; ++iter) {
    	Vertex v = *iter;
    	const Node* n = boost::get(boost::vertex_name, graph, v);
    	LogOutNotSync("(\"sd-vertex\""
    		<<","<<0
    		<<","<<v
    		<<",{"
    		<<"\"parent\":\""<<&graph
    		<<"\",\"xPos\":\""<<n->location.getX()
    		<<"\",\"yPos\":\""<<n->location.getY()
    		<<"\"})"<<std::endl);
    }
	}

    //Print each edge
	//NOTE: Edges are currently identified by their "from/to" nodes (as a pair), so we'll just make up a
	//      suitable ID for them (it doesn't actually matter).
    {
    Graph::edge_iterator iter, end;
    unsigned int id=0;
    for (boost::tie(iter, end) = boost::edges(graph); iter != end; ++iter) {
    	Edge ed = *iter;
    	Vertex srcV = boost::source(ed, graph);
    	Vertex destV = boost::target(ed, graph);
    	LogOutNotSync("(\"sd-edge\""
    		<<","<<0
    		<<","<<id++
    		<<",{"
    		<<"\"parent\":\""<<&graph
    		<<"\",\"fromVertex\":\""<<srcV
    		<<"\",\"toVertex\":\""<<destV
    		<<"\"})"<<std::endl);
    }
    }
}


/** \endcond ignoreStreetDirectoryInnards -- End of block to be ignored by doxygen.  */




////////////////////////////////////////////////////////////////////////////////////////////
// StreetDirectory
////////////////////////////////////////////////////////////////////////////////////////////

void
StreetDirectory::init(RoadNetwork const & network, bool keepStats /* = false */,
                      centimeter_t gridWidth, centimeter_t gridHeight)
{
    if (keepStats) {
        stats_ = new Stats;
    }
    pimpl_ = new Impl(network, gridWidth, gridHeight);
    spImpl_ = new ShortestPathImpl(network);
}

void
StreetDirectory::updateDrivingMap()
{
	if(spImpl_) {
		spImpl_->updateEdgeProperty();
		spImpl_->GeneratePathChoiceSet();
	}
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

const MultiNode* StreetDirectory::GetCrossingNode(const Crossing* cross) const
{
	return pimpl_ ? pimpl_->GetCrossingNode(cross) : nullptr;
}

Signal const *
StreetDirectory::signalAt(Node const & node) const
{
//	std::cout << "StreetDirectory: " << signals_.size() << std::endl;
	std::map<const Node *, Signal const *>::const_iterator iter = signals_.find(&node);
    if (signals_.end() == iter) {
        return nullptr;
    }
    return iter->second;
}

std::vector<WayPoint>
StreetDirectory::shortestDrivingPath(Node const & fromNode, Node const & toNode) const
{
    return spImpl_ ? spImpl_->shortestDrivingPath(fromNode, toNode)
                   : std::vector<WayPoint>();
}

std::vector<WayPoint>
StreetDirectory::GetShortestDrivingPath(Node const & fromNode, Node const & toNode) const
{
    return spImpl_ ? spImpl_->GetShortestDrivingPath(fromNode, toNode)
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

void
StreetDirectory::printDrivingGraph()
{
	if (spImpl_) {
		spImpl_->printGraph("driving", spImpl_->drivingMap_);
	}
}

void
StreetDirectory::printWalkingGraph()
{
	if (spImpl_) {
		spImpl_->printGraph("walking", spImpl_->walkingMap_);
	}
}



}
