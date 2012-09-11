/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "geospatial/RoadSegment.hpp"

namespace sim_mob{

namespace medium{

class LaneGroup{
public:
	explicit LaneGroup(sim_mob::RoadSegment* parent, unsigned long id);
	///Return the Link this RoadSegment is part of.
	sim_mob::RoadSegment* getRoadSegment() const { return parentSegment; }
	void setLanes(std::vector<sim_mob::Lane*>);

	///Retrieve the Lanes within this segment.
	//TEMP: For now, returning a const vector of non-const lanes. Will fix later. ~Seth
	void end1(){};
	const std::vector<sim_mob::Lane*>& getLanes() const {
		return lanes; }

	double getOutputCounter ( ) const
	    { return this->outputCounter; }

	void setOutputCounter (double outputCounter)
	    { this->outputCounter = outputCounter; }

	double getAcceptRate () const {return this->acceptRate; }

	void setAcceptRate(double acceptRate)
	    {this->acceptRate = acceptRate;}

	int getNumOfEmptySpaces(double length, double meanVehicleLength) const;

private:
	///Which link this appears in
	sim_mob::RoadSegment* parentSegment;
	unsigned long lgID;
	double outputCounter;
	double acceptRate;

	std::vector<sim_mob::Lane*> lanes;
};

}
}
