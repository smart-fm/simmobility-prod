#pragma once

#include <vector>
#include <algorithm>

#include "RStarTreeDownUp.h"

namespace sim_mob
{

//Forward declarations.
class Agent;

////////////////////////////////////////////////////////////////////////////////////////////
// R*-Tree
////////////////////////////////////////////////////////////////////////////////////////////

// The AuraManager uses a 2-D R*-tree to create a spatial indexing of the agents.
// Each node (both non-leaf and leaf) in the R*-tree holds 8 to 16 items.
class R_tree_DU: public RStarTreeDownUp<Agent const *, 2, 30, 100>
{
public:
	// No need to define the ctor and dtor.

	// Insert an agent into the tree, based on the agent's position.
	void
	insert(Agent const * agent);

	//update new location, no need to delete and then insert
	void update(int object_id, int new_location_x, int new_location_y);

	//remove an Agent
	void remove(Agent const * agent);

	//
	bool has_one_agent(int agent_id);

	// Return an array of agents that are located inside the search rectangle.
	// box.edges[].first is the lower-left corner and box.edges[].second is the
	// upper-right corner.  box.edges[0] is the x- component and box.edges[1] is the
	// y- component.
	std::vector<Agent  const*>
	query(R_tree_DU::BoundingBox const & box) const;
};

}
