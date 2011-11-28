/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "constants.h"

#include "Point2D.hpp"


namespace sim_mob
{


/**
 * Base class for geospatial items which take up physical space but are not traversable.
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
public:
	virtual ~RoadItem() {} //A virtual destructor allows this type to be polymorphic

	const sim_mob::Point2D& getStart() { return start; }
	const sim_mob::Point2D& getEnd() { return end; }


protected:
	sim_mob::Point2D start;
	sim_mob::Point2D end;



};





}
