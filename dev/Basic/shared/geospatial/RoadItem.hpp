//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "geospatial/Point2D.hpp"
#include "util/LangHelpers.hpp"

namespace geo {
class crossing_t_pimpl;
class RoadItem_t_pimpl;
}

namespace sim_mob
{
//Forward Declaration
class RoadSegment;

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
	friend class ::geo::RoadItem_t_pimpl;
public:
	RoadItem() : parentSegment_(nullptr) {}

	virtual ~RoadItem() {}

	void setParentSegment(RoadSegment *rs) { parentSegment_ = rs;}
    sim_mob::RoadSegment* getParentSegment() const { return parentSegment_; }

	const sim_mob::Point2D& getStart() const { return start; }
	const sim_mob::Point2D& getEnd() const { return end; }
	const unsigned long getRoadItemID()const { return id;}
	void setRoadItemID(unsigned long id_) { id = id_;}
	static unsigned long generateRoadItemID(const sim_mob::RoadSegment& rs);


//TODO: Fix for XML loader.
//protected:
	sim_mob::Point2D start;
	sim_mob::Point2D end;
	unsigned long id;

private:
	sim_mob::RoadSegment* parentSegment_;


};





}
