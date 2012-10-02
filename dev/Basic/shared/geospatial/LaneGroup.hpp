/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "geospatial/RoadSegment.hpp"

namespace sim_mob{

class LaneGroup{
public:
	LaneGroup(const sim_mob::RoadSegment* parent, int id);
	///Return the Link this RoadSegment is part of.
	const sim_mob::RoadSegment* getRoadSegment() const { return parentSegment; }
	void setLanes(std::vector<const sim_mob::Lane*>);

	///Retrieve the Lanes within this segment.
	//TEMP: For now, returning a const vector of non-const lanes. Will fix later. ~Seth
	const std::vector<const sim_mob::Lane*>& getLanes() const {
		return lanes; }
	int getNumOfEmptySpaces(double length, double meanVehicleLength) const;
	std::vector<sim_mob::RoadSegment*> getOutGoingSegments();
	void setOutgoingSegments(std::vector<RoadSegment*> outRS);


private:
	///Which link this appears in
	const sim_mob::RoadSegment* parentSegment;
	int lgID;
	std::vector<const sim_mob::Lane*> lanes;
	std::vector<RoadSegment*> outgoingRoadSegments;
};

}
