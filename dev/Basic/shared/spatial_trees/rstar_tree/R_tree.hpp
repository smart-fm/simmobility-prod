//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include "spatial_trees/rstar_tree/RStarTree.hpp"

namespace sim_mob
{

//Forward declarations.
class Agent;


////////////////////////////////////////////////////////////////////////////////////////////
// R*-Tree
////////////////////////////////////////////////////////////////////////////////////////////
// The AuraManager uses a 2-D R*-tree to create a spatial indexing of the agents.
// Each node (both non-leaf and leaf) in the R*-tree holds 8 to 16 items.

class R_tree: public RStarTree<Agent const *, 2, 15, 50>
{
public:
	// No need to define the ctor and dtor.

	// Insert an agent into the tree, based on the agent's position.
	void
	insert(Agent const * agent);

	// Return an array of agents that are located inside the search rectangle.
	// box.edges[].first is the lower-left corner and box.edges[].second is the
	// upper-right corner.  box.edges[0] is the x- component and box.edges[1] is the
	// y- component.
	std::vector<Agent const *>
	query(R_tree::BoundingBox const & box) const;

	//display the tree structure
	void display();

	//display a tree structure
	void display(Node * node);

private:
	// A visitor that simply collects the agent into an array, which was specified in the
	// constructor.
	struct Collecting_visitor
	{
		const bool ContinueVisiting;
		std::vector<Agent const *> & array;  // must be a reference.

		explicit Collecting_visitor(std::vector<Agent const *> & array) :
				ContinueVisiting(true), array(array)
		{
		}

		// When called, the visitor saves the agent in <array>.
		bool operator()(const R_tree::Leaf * const leaf) const
		{
			array.push_back(leaf->leaf);
			return true;
		}
	};
};

}
