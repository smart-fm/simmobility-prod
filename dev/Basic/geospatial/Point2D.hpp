#pragma once


namespace sim_mob
{

/**
 * Base class for geospatial items in the road network. In general, everything within a road network
 * is defined by its Point2D location. Road segments are defined by 2 points; one at the beginning
 * of the segment and another at the end. Intersections are defined by a point at the intersection
 * of all roads that join at that point.
 *
 * The Point2D location of any item should be accurate enough to generate a reasonable visualization
 * of that item using its Point(s) alone.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 *
 * \todo
 * We created the Point2D class partly to allow easier sharing of Nodes. Should we treat 2
 * points at the same X/Y location as physically identical? Should we then have a function
 * like Point2D::GetPoint(x, y) which returns a new point ONLY if the point x/y doesn't
 * exist, and returns the pre-existing point otherwise? Just some thoughts.
 * \par
 * ~Seth
 */
class Point2D {
public:
	double xPos;
	double yPos;



};





}
