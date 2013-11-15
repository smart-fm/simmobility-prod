//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "GridStreetDirectoryImpl.hpp"

#include <cmath>

//TODO: Prune this include list later; it was copied directly from StreetDirectory.cpp
#include "buffering/Vector2D.hpp"
#include "geospatial/coord/CoordinateTransform.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadNetwork.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/LaneConnector.hpp"
#include "geospatial/BusStop.hpp"
#include "geospatial/Crossing.hpp"
#include "geospatial/ZebraCrossing.hpp"
#include "geospatial/MultiNode.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/RoadRunnerRegion.hpp"
#include "logging/Log.hpp"
#include "util/GeomHelpers.hpp"

using std::map;
using std::set;
using std::vector;
using std::string;

using namespace sim_mob;


namespace {


//Returns -1, 1, or 0 depending on where the point lies in relation to the line.
//This function was modified from the openJDK project; it is licensed under the terms of the GNU GPL 2
//   and is copyright 1997, 2006, Oracle and/or its affiliates.
int relativeCCW(double x1, double y1, double x2, double y2, double px, double py) {
	x2 -= x1;
	y2 -= y1;
	px -= x1;
	py -= y1;
	double ccw = px*y2 - py*x2;
	if (ccw == 0.0) {
		// The point is colinear, classify based on which side of
		// the segment the point falls on.  We can calculate a
		// relative value using the projection of px,py onto the
		// segment - a negative value indicates the point projects
		// outside of the segment in the direction of the particular
		// endpoint used as the origin for the projection.
		ccw = px*x2 + py*y2;
		if (ccw > 0.0) {
			// Reverse the projection to be relative to the original x2,y2,x2 and y2 are simply negated.
			// px and py need to have (x2 - x1) or (y2 - y1) subtracted from them (based on the original values)
			// Since we really want to get a positive answer when the point is "beyond (x2,y2)", then we want to calculate
			//    the inverse anyway - thus we leave x2 & y2 negated.
			px -= x2;
			py -= y2;
			ccw = px*x2 + py*y2;
			if (ccw < 0.0) {
				ccw = 0.0;
			}
		}
	}
	return (ccw < 0.0) ? -1 : ((ccw > 0.0) ? 1 : 0);
}



//Check if 2 lines intersect.
//This function was modified from the openJDK project; it is licensed under the terms of the GNU GPL 2
//   and is copyright 1997, 2006, Oracle and/or its affiliates.
bool line_intersects_line(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
	return ((relativeCCW(x1, y1, x2, y2, x3, y3) *
			relativeCCW(x1, y1, x2, y2, x4, y4) <= 0)
			&& (relativeCCW(x3, y3, x4, y4, x1, y1) *
			relativeCCW(x3, y3, x4, y4, x2, y2) <= 0)
	);
}



//Check if a point is inside of a Region. Note that this code is copied from
// Jason's RoadRunner code; we might want to eventually use a more sophisticated
// Region check, but for now this is sufficient.
bool point_inside_region(RoadRunnerRegion r, LatLngLocation pt) {
	double x = pt.longitude;
	double y = pt.latitude;
	int polySides = r.points.size();
	bool oddTransitions = false;

	for (int i=0,j=polySides-1; i<polySides; j=i++) {
		if ((r.points.at(i).latitude < y && r.points.at(j).latitude >= y)
			|| (r.points.at(j).latitude < y && r.points.at(i).latitude >= y)) {
			if (r.points.at(i).longitude +
					(y - r.points.at(i).latitude) / (r.points.at(j).latitude
					- r.points.at(i).latitude)
					* (r.points.at(j).longitude - r.points.at(i).longitude) < x) {
				oddTransitions = !oddTransitions;
			}
		}
	}
	return oddTransitions;
}


//Check if a line intersects one of the Region's lines.
bool line_intersects_region(RoadRunnerRegion r, LatLngLocation start, LatLngLocation end) {
	int polySides = r.points.size();
	DynamicVector vec1(start.longitude, start.latitude, end.longitude, end.latitude);

	for (int i=0,j=polySides-1; i<polySides; j=i++) {
		DynamicVector vec2(r.points.at(i).longitude, r.points.at(i).latitude, r.points.at(j).longitude, r.points.at(j).latitude);
		if (line_intersects_line(vec1.getX(), vec1.getY(), vec1.getEndX(), vec1.getEndY(), vec2.getX(), vec2.getY(), vec2.getEndX(), vec2.getEndY())) {
			return true;
		}
	}
	return false;
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
} //End un-named namespace


sim_mob::GridStreetDirectoryImpl::GridStreetDirectoryImpl(const RoadNetwork& network, centimeter_t gridWidth, centimeter_t gridHeight)
  : gridWidth_(gridWidth), gridHeight_(gridHeight)
{
    partition(network);

    //Build additional lookups
    set<const Crossing*> completedCrossings;
    for (vector<Link*>::const_iterator iter = network.getLinks().begin(); iter != network.getLinks().end(); ++iter) {
    	buildLookups((*iter)->getSegments(), completedCrossings, network.roadRunnerRegions, network.getCoordTransform(false));
    }

	//TEMP:
	Print() <<"REGIONS MAP: \n";
	for (std::map<const RoadSegment*, RoadRunnerRegion>::const_iterator it=rrRegionLookup.begin(); it!=rrRegionLookup.end(); it++) {
		Print() <<"  " <<it->first <<" => " <<it->second.id <<"\n";
	}
	Print() <<"END REGIONS MAP\n";
}


void sim_mob::GridStreetDirectoryImpl::partition(const vector<RoadSegment*>& segments, bool isForward)
{
    for (size_t i=0; i<segments.size(); i++) {
        partition(*segments[i], isForward);
    }
}

void sim_mob::GridStreetDirectoryImpl::partition(const RoadNetwork& network)
{
	const vector<Link*>& links = network.getLinks();
    for (size_t i=0; i<links.size(); i++) {
    	const Link* link = links[i];
        if(link) {
        	partition(link->getSegments(), true);
        }
    }
}


namespace
{

// AABB are Axially-Aligned Bounding Boxes.
// They are aligned to the X- and Y- axes and they are bounding boxes of some object, that is,
// the object is wholly inside the AABB.
struct AABB {
	Point2D lowerLeft_;
	Point2D upperRight_;

	AABB(Point2D const & lowerLeft, Point2D const & upperRight)
		: lowerLeft_(lowerLeft), upperRight_(upperRight)
	{}

	AABB(Point2D const & lowerLeft, centimeter_t width, centimeter_t height)
		: lowerLeft_(lowerLeft), upperRight_(lowerLeft_.getX() + width, lowerLeft_.getY() + height)
	{}

	AABB(centimeter_t left, centimeter_t right, centimeter_t bottom, centimeter_t top)
		: lowerLeft_(left, bottom), upperRight_(right, top)
	{}

	centimeter_t left()   const { return lowerLeft_.getX();  }
	centimeter_t right()  const { return upperRight_.getX(); }
	centimeter_t bottom() const { return lowerLeft_.getY();  }
	centimeter_t top()    const { return upperRight_.getY(); }
    };



  //  AABB getBoundingBox(const Point2D& p1, const Point2D& p2, centimeter_t halfWidth);


    // Return true if <point> is inside the rectangle <aabb>.
    bool isPointInsideAABB(Point2D const & point, AABB const & aabb) {
    	return    aabb.left() <= point.getX()   && point.getX() <= aabb.right()
               && aabb.bottom() <= point.getY() && point.getY() <= aabb.top();
    }

    // Return the width of the specified road segment.
    centimeter_t getWidth(RoadSegment const & segment) {
    	if (segment.width != 0) {
    		return segment.width;
    	} else {
    		centimeter_t width = 0;
    		vector<Lane*> const & lanes = segment.getLanes();
    		for (size_t i = 0; i < lanes.size(); i++) {
    			width += lanes[i]->getWidth();
    		}
    		return width;
    	}
    }

    // Return the lane where <point> is located, 0 if the point is outside of the stretch of
    // the road segment.
    // The stretch is specified by <p1>, <p2>, and <segment->width>; the line from <p1> to <p2>
    // traces the middle of the stretch.
    const Lane* getTheLane(const RoadSegment& segment, const Point2D& p1, const Point2D& p2, const Point2D& point);

    // Return true if <aabb> covers <grid> completely.
    bool isGridWhollyInsideAABB(const AABB& grid, const AABB& aabb) {
        return    aabb.left() <= grid.left() && grid.right() <= aabb.right()
               && aabb.bottom() <= grid.bottom() && grid.top() <= aabb.top();
    }

    // Return true if the stretch of road segment overlaps or lies within the rectangle <aabb>.
    // The stretch is specified by <p1>, <p2>, and <halfWidth>; the line from <p1> to <p2>
    // traces the middle of the stretch.
    bool didRoadIntersectAABB(const Point2D& p1, const Point2D& p2, centimeter_t halfWidth, const AABB& aabb);

    const Lane* getTheLane(const RoadSegment& segment, const Point2D& p1, const Point2D& p2, const Point2D& point)
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
        if (isLeft) {
            dist = halfWidth - dist;
        } else {
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
    bool isInBetween(centimeter_t num, centimeter_t denom)
    {
        if (num > 0) {
            if (denom > 0 && num < denom)
                return true;
        } else {
            if (denom < 0 && num > denom)
                return true;
        }
        return false;
    }

    // Return true if the line from <p1> to <p2> intersects or lies within the rectangle <aabb>.
    bool didLineIntersectAABB(const Point2D& p1, const Point2D& p2, const AABB& aabb)
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


    // Calculate the AABB that would contain a stretch of a road segment.
    // The stretch is specified by <p1>, <p2>, and <halfWidth>; the line from <p1> to <p2>
    // traces the middle of the stretch.  The line may not be aligned to the X- and Y- axes.
    AABB getBoundingBox(const Point2D& p1, const Point2D& p2, centimeter_t halfWidth) {
         centimeter_t left = 0;
         centimeter_t right = 0;
         centimeter_t top = 0;
         centimeter_t bottom = 0;

         if (p1.getX() > p2.getX()) {
             left = p2.getX();
             right = p1.getX();
         } else {
             left = p1.getX();
             right = p2.getX();
         }

         if (p1.getY() > p2.getY()) {
             top = p1.getY();
             bottom = p2.getY();
         } else {
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

    bool didRoadIntersectAABB(const Point2D& p1, const Point2D& p2, centimeter_t halfWidth, const AABB& aabb)
    {
        if (didLineIntersectAABB(p1, p2, aabb)) {
            return true;
        }

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

} //End un-named namespace





bool sim_mob::GridStreetDirectoryImpl::checkGrid(int m, int n, const Point2D& p1, const Point2D& p2, centimeter_t halfWidth) const
{
    AABB grid(Point2D(m * gridWidth_, n * gridHeight_), gridWidth_, gridHeight_);
    return didRoadIntersectAABB(p1, p2, halfWidth, grid);
}



void sim_mob::GridStreetDirectoryImpl::buildLookups(const vector<RoadSegment*>& roadway, set<const Crossing*>& completed, const std::map<int, sim_mob::RoadRunnerRegion>& roadRunnerRegions, sim_mob::CoordinateTransform* coords)
{
	//Warn if we have no coordinate transform, but also Regions
	if (!roadRunnerRegions.empty() && !coords) {
		Warn() <<"RoadRunnerRegions are included in the network, but no coordinate transform exists.\n";
	}

	//Scan for each crossing (note: this copies the ShortestPathImpl_ somewhat, may want to consolidate later).
	for (vector<RoadSegment*>::const_iterator segIt=roadway.begin(); segIt!=roadway.end(); segIt++) {
		//Build a lookup for this RoadSegment's start/end Nodes.
    	nodes.insert((*segIt)->getStart());
    	nodes.insert((*segIt)->getEnd());

		//Save its associated RoadRunnerRegion.
    	if (coords && !roadRunnerRegions.empty()) {
    		//Get the midpoint of this Segment.
    		DynamicVector dv((*segIt)->getStart()->location, (*segIt)->getEnd()->location);
    		LatLngLocation start = coords->transform(DPoint(dv.getX(), dv.getY()));
    		LatLngLocation end = coords->transform(DPoint(dv.getEndX(), dv.getEndY()));
    		dv.scaleVectTo(dv.getMagnitude()/2.0);
    		LatLngLocation midpt = coords->transform(DPoint(dv.getX(), dv.getY()));

    		//Check each region until we find one that matches.
			for (std::map<int, sim_mob::RoadRunnerRegion>::const_iterator rrIt=roadRunnerRegions.begin(); rrIt!=roadRunnerRegions.end(); rrIt++) {
				if (point_inside_region(rrIt->second, midpt)) {
					rrRegionLookup[*segIt] = rrIt->second;
					break;
				}
				if (line_intersects_region(rrIt->second, start, end)) {
					rrRegionLookup[*segIt] = rrIt->second;
					break;
				}
			}
    	}

    	unsigned int id = (*segIt)->getSegmentAimsunId();
    	segmentByAimsunID.insert(std::make_pair(id, (*segIt)));

		//Save its obstacles
		for (map<centimeter_t, const RoadItem*>::const_iterator riIt=(*segIt)->obstacles.begin(); riIt!=(*segIt)->obstacles.end(); riIt++) {
			//Check if it's a crossing; check if we've already processed it; tag it.
			const Crossing* cr = dynamic_cast<const Crossing*>(riIt->second);
			if (cr && completed.find(cr)==completed.end()) {
				completed.insert(cr);

				//Find whatever MultiNode is closest.
				const MultiNode* atNode = StreetDirectory::FindNearestMultiNode(*segIt, cr);
				if (atNode) {
					//Tag it.
					crossings_to_multinodes[cr] = atNode;
				}
			}

			//Check if it's a BusStop; add it to the lookup.
		    const BusStop* bs = dynamic_cast<const BusStop*>(riIt->second);
		    if (bs) {
		    	busStops_.insert(bs);
		    }
		}
	}
}

const sim_mob::RoadSegment* sim_mob::GridStreetDirectoryImpl::getRoadSegment(const unsigned int id){

	std::map<const unsigned int, const sim_mob::RoadSegment*>::iterator it = segmentByAimsunID.find(id);
	if (it!=segmentByAimsunID.end()) {
		return it->second;
	}
	return nullptr;
}

void sim_mob::GridStreetDirectoryImpl::partition(const RoadSegment& segment, bool isForward) {
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

        const StreetDirectory::RoadSegmentAndIndexPair pair(&segment, startIndex, endIndex);

        // The AABB containing the road segment stretch overlaps the following grid cells (m. n).
        for (int m = left; m <= right; m++)
        {
            for (int n = bottom; n <= top; n++)
            {
                // p1 is inside the (left, bottom) grid cell and p2 is inside (right, top)
                // grid cell.  So we push this road segment stretch into these grid cells.
                if ((m == left && n == bottom) || (m == right && n == top))
                    grid_[Point2D(m, n)].push_back(pair);
                else if (checkGrid(m, n, p1, p2, halfWidth))
                {
                    grid_[Point2D(m, n)].push_back(pair);
                }
            }
        }
    }
}


std::pair<sim_mob::RoadRunnerRegion, bool> sim_mob::GridStreetDirectoryImpl::getRoadRunnerRegion(const sim_mob::RoadSegment* seg)
{
	if (seg) {
		//Try to find it.
		std::map<const RoadSegment*, RoadRunnerRegion>::const_iterator it = rrRegionLookup.find(seg);
		if (it!=rrRegionLookup.end()) {
			return std::make_pair(it->second, true);
		}
	}

	return std::make_pair(RoadRunnerRegion(), false);
}


const BusStop* sim_mob::GridStreetDirectoryImpl::getBusStop(const Point2D& position) const
{
	//This function currently searches point-by-point, since we don't have that many BusStops.
	//TODO: Ideally, it would use some kind of spatial index.
	const int Threshold = 10 * 100; //10m
	for (std::set<const BusStop*>::const_iterator it=busStops_.begin(); it!=busStops_.end(); it++) {
		if (dist(Point2D((*it)->xPos, (*it)->yPos), position) < Threshold) {
			return *it;
		}
	}
	return nullptr;
}


const Node* sim_mob::GridStreetDirectoryImpl::getNode(const int id) const
{
	for (std::set<const Node*>::const_iterator it=nodes.begin(); it!=nodes.end(); it++) {
		if ((*it)->getID() == id) {
			return *it;
		}
	}
	return nullptr;
}



StreetDirectory::LaneAndIndexPair sim_mob::GridStreetDirectoryImpl::getLane(const Point2D& point) const {
    Point2D cell(point.getX() / gridWidth_, point.getY() / gridHeight_);
    GridType::const_iterator iter = grid_.find(cell);
    if (iter == grid_.end()) {
        // Either the road network for this grid cell was not loaded from the database
        // or there is no road network in the cell (for example, the cell could be a portion
        // of a large lake or reservoir).  In this version we assume the entire road network
        // was loaded in.
        return StreetDirectory::LaneAndIndexPair();
    }

    // We only need to check the road segments in this grid cell.
    const RoadSegmentSet& segments = iter->second;
    for (size_t i = 0; i < segments.size(); ++i) {
        StreetDirectory::RoadSegmentAndIndexPair const & pair = segments[i];
        RoadSegment const * segment = pair.segment_;
        size_t start = pair.startIndex_;
        size_t end = pair.endIndex_;

        centimeter_t halfWidth = getWidth(*segment) / 2;
        const Point2D& p1 = segment->polyline[start];
        const Point2D& p2 = segment->polyline[end];
        AABB aabb = getBoundingBox(p1, p2, halfWidth);

        // The outer test is inexpensive.  We quickly skip road segments that are too far
        // from <point>.  However <aabb> may be fairly large; worst case is when the line from
        // <p1> to <p2> is inclined 45 degrees to the axes.  Therefore it is possible that
        // <point> falls inside <aabb>, but is outside of the stretch of <segment>.  The inner
        // test is more thorough.
        if (isPointInsideAABB(point, aabb)) {
            if (const Lane* lane = getTheLane(*segment, p1, p2, point))
                return StreetDirectory::LaneAndIndexPair(lane, start, end);
        }
    }

    return StreetDirectory::LaneAndIndexPair();
}



const MultiNode* sim_mob::GridStreetDirectoryImpl::GetCrossingNode(const Crossing* cross) const
{
	std::map<const Crossing*, const MultiNode*>::const_iterator res = crossings_to_multinodes.find(cross);
	if (res!=crossings_to_multinodes.end()) {
		return res->second;
	}
	return nullptr;
}



vector<StreetDirectory::RoadSegmentAndIndexPair> sim_mob::GridStreetDirectoryImpl::closestRoadSegments(const Point2D& point, centimeter_t halfWidth, centimeter_t halfHeight) const
{
    // <aabb> is the search rectangle.
    AABB aabb(point, halfWidth, halfHeight);
    int left = aabb.lowerLeft_.getX() / gridWidth_;
    int right = aabb.upperRight_.getX() / gridWidth_;
    int bottom = aabb.lowerLeft_.getY() / gridHeight_;
    int top = aabb.upperRight_.getY() / gridHeight_;

    vector<StreetDirectory::RoadSegmentAndIndexPair> result;
    // The search rectangle overlaps the following grid cells (m. n).
    for (int m = left; m <= right; m++) {
        for (int n = bottom; n <= top; n++) {
            GridType::const_iterator iter = grid_.find(Point2D(m, n));
            if (iter != grid_.end()) {
                const vector<StreetDirectory::RoadSegmentAndIndexPair>& segments = iter->second;

                AABB grid(Point2D(m * gridWidth_, n * gridHeight_), gridWidth_, gridHeight_);
                if (isGridWhollyInsideAABB(grid, aabb)) {
                    // Wow, the search rectangle is larger than the grid size.
                    result.insert(result.end(), segments.begin(), segments.end());
                } else {
                    // The search rectangle does not cover this grid cell completely.
                    // We need to figure out which road segment is really inside <aabb>.
                    for (size_t i = 0; i < segments.size(); i++) {
                        StreetDirectory::RoadSegmentAndIndexPair const & pair = segments[i];
                        const RoadSegment* segment = pair.segment_;
                        const size_t start = pair.startIndex_;
                        const size_t end = pair.endIndex_;

                        const Point2D& p1 = segment->polyline[start];
                        const Point2D& p2 = segment->polyline[end];
                        centimeter_t halfWidth = getWidth(*segment) / 2;
                        if (didRoadIntersectAABB(p1, p2, halfWidth, aabb)) {
                            result.push_back(pair);
                        }
                    }
                }
            }
        }
    }
    return result;
}
