#pragman once
#include <vector>
#include "geospatial/network/RoadItem.hpp"

namespace sim_mob
{

struct LaneItem
{
	LaneItem() : laneId(0), speedLimit(0)
	{
	}
	unsigned int laneId;
	float speedLimit;
};

class Incident : public sim_mob::RoadItem{

public:
	unsigned int incidentId;
	float visibilityDistance;
	unsigned int segmentId;
	float position;
	unsigned int severity;
	float capFactor;
	unsigned int startTime;
	unsigned int duration;
	float length;
	float compliance;
	float accessibility;

	std::vector<LaneItem> laneItems;

public:

	explicit Incident() : RoadItem(), incidentId(0), visibilityDistance(0.0),
			segmentId(0), position(0.0), severity(0.0), capFactor(0.0), startTime(0), duration(0), length(0.0), compliance(0), accessibility(0)
	{;}

	virtual ~Incident()
	{
	}
};

} /* namespace sim_mob */
