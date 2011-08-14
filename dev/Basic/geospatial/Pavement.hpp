/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <map>
#include <vector>

#include "RoadItem.hpp"
#include "Point2D.hpp"


namespace sim_mob
{




/**
 * Helper class that represents a RoadItem at a given offset.
 */
struct RoadItemAndOffsetPair
{
	RoadItemAndOffsetPair(const sim_mob::RoadItem* item, unsigned int offset) : item(item), offset(offset) {}

	///The next RoadItem
    const sim_mob::RoadItem* item;

    /// The offset from the Pavement::start where \c item is located.
    unsigned int offset;
};



/**
 * Represents RoadItems that may contain obstacles and have a complex geometry.
 *
 * \note
 * The length of a Pavement object is NOT the Euclidean distance between its start
 * and end nodes, but rather the total length of the polyline. An Agent's position
 * along a Pavement object is specified in relation to that Pavement's length.
 */
class Pavement : public sim_mob::RoadItem {
public:
	//NOTE: I'm importing AIMSUN data which has length as a double. Will need to consider why later. ~Seth
	double length;

	unsigned int width;
	std::vector<sim_mob::Point2D> polyline;
	std::map<unsigned int, const RoadItem*> obstacles;

	///Return the next obstacle from a given point on this Pavement.
	sim_mob::RoadItemAndOffsetPair nextObstacle(const sim_mob::Point2D& pos, bool isForward);

	///Return the next obstacle from a given offset along the current Pavement.
	sim_mob::RoadItemAndOffsetPair nextObstacle(unsigned int offset, bool isForward);

	///Helper method: build a polyline given a bulge and a center. Segments are generated
	///   of length segmentLength.
	///
	/// \todo segmentLength might be better represented as something like "maxAngle", which
	///       would allow for long non-curving segments and short curving ones.
	static void GeneratePolyline(Pavement* p, Point2D center, double bulge, int segmentLength);

private:



};





}
