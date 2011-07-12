#pragma once

#include <map>
#include <vector>

#include "RoadItem.hpp"
//#include "Point2D.hpp"


namespace sim_mob
{


//Forward declarations
class Point2D;



/**
 * Represents RoadItems that may contain obstacles and have a complex geometry.
 *
 * \note
 * The length of a Pavement object is NOT the Euclidean distance between its start
 * and end nodes, but rather the total length of the polyline. An Agent's position
 * along a Pavement object is specified in relation to that Pavement's length.
 *
 * \note
 * This is a skeleton class. All functions are defined in this header file.
 * When this class's full functionality is added, these header-defined functions should
 * be moved into a separate cpp file.
 */
class Pavement : public sim_mob::RoadItem {
public:
	int length;
	int width;
	std::vector<sim_mob::Point2D> polyline;
	std::map<unsigned int, const RoadItem*> obstacles;

	std::pair<unsigned int, const RoadItem*> nextObstacle(const Point2D& pos, bool isForward) { return std::pair<unsigned int, const RoadItem*>; }
	std::pair<unsigned int, const RoadItem*> nextObstacle(unsigned int offset, bool isForward) { return std::pair<unsigned int, const RoadItem*>; }

private:



};





}
