/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "GenConfig.h"
#include "geospatial/RoadSegment.hpp"
#include "Point2D.hpp"
namespace geo
{
class crossing_t_pimpl;
}

namespace sim_mob
{
//Forward Declaration
//class RoadSegment;

/**
 * Base class for geospatial items which take up physical space but are not traversable.
 *
 * \author Seth N. Hetu
 *
 * RoadItems have a start and end position, but unlike the Traversable class, these are not
 * Nodes but Poin2Ds. Additional data may be used to fine-tune this item's shape
 * in 2-D space, or even to extend it into 3-D space.
 *
 * \note
 * Currently the Node base class only contains a Point2D, so the distinction is not perfectly
 * clear. However, we expect to add more functionality to Node, so consider Traversables to
 * have MultiNodes or UniNodes instead of simply Nodes and the distinction between a Traversable
 * and a RoadItem will become clearer.
 */
class RoadItem {
	friend class ::geo::crossing_t_pimpl;
public:
	virtual ~RoadItem() {} //A virtual destructor allows this type to be polymorphic
	virtual void setParentSegment(sim_mob::RoadSegment*) {};

	const sim_mob::Point2D& getStart() { return start; }
	const sim_mob::Point2D& getEnd() { return end; }
	const unsigned long getRoadItemID()const { return id;}
	void setRoadItemID(unsigned long id_) { id = id_;}
	static unsigned long generateRoadItemID(const sim_mob::RoadSegment rs)
	{
		return rs.getSegmentID() * 10 + rs.obstacles.size();
	}


protected:
	sim_mob::Point2D start;
	sim_mob::Point2D end;
	unsigned long id;



};





}
