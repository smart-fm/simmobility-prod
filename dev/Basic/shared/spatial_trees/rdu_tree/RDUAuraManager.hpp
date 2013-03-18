#pragma once

#include "R_tree_DU.h"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"
#include "buffering/Vector2D.hpp"
#include "entities/Person.hpp"
#include <vector>

namespace sim_mob
{

class RDUAuraManager
{
public:
	//xuyan: Build the Tree in the very beginning
	void init_du(const char* file_url);

	void update_du();

	bool has_one_agent_du(int agent_id);

//	std::vector<Agent const*>
//	agentsInRect(double from_x, double from_y, double to_x, double to_y) const;

	std::vector<Agent const *>
	agentsInRect_du(Point2D const & lowerLeft, Point2D const & upperRight) const;

	std::vector<Agent const *>
	nearbyAgents_du(Point2D const & position, Lane const & lane, centimeter_t distanceInFront, centimeter_t distanceBehind) const;

private:

	sim_mob::R_tree_DU tree_du;
};

}
