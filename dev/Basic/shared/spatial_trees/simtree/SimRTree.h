/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "util/LangHelpers.hpp"
#include "spatial_trees/rstar_tree/RStarBoundingBox.h"

//it is very important, if no need for REBALANCE, change it to
//#define NO_USE_REBALANCE
#define USE_REBALANCE

//Note: this class is designed for SimMobility.
//Agent class has been compiled into Tree Structure

/**
 * Tree Node Definition
 */

namespace sim_mob
{

//Forward declarations.
class Agent;

//Forward declare structs used in this class.
struct TreeItem;
struct TreeLeaf;
struct TreeNode;


/**
 * Tree Definition
 */

class SimRTree {
private:
	TreeNode* m_root;
	TreeLeaf* first_leaf;

private:
	long leaf_counts;
	double leaf_agents_sum;
	double unbalance_ratio;

#ifdef USE_REBALANCE
	//parameters for re-balance
public:
	//the border of the network
	static long network_minimum_x;
	static long network_minimum_y;
	static long network_maximum_x;
	static long network_maximum_y;

	static int division_x_unit_;
	static int division_y_unit_;

//	static int map_division_x_max;
//	static int map_division_y_max;

	//xuyan:it is hard coded inside
	static int bigtable[1000][1000];

	//if re-balance happens %rebalance_threshold times continuously, then need to rebuild the tree
	int rebalance_threshold;
	static const int checking_frequency;

	int rebalance_counts;
#endif

public:
	SimRTree() : m_root(nullptr), first_leaf(nullptr), leaf_counts(0), leaf_agents_sum(0), unbalance_ratio(0)
#ifdef USE_REBALANCE
		,rebalance_counts(0), rebalance_threshold(2)
#endif
{}

	//Typedef to refer to our Bounding boxes.
	typedef RStarBoundingBox<2> BoundingBox;


public:
	/*
	 *build the tree structure using data from the file
	 *File Line Format:
	 *(Parent-Item-ID, Own-ID, BOX_X1, BOX_Y1, BOX_X2, BOX_Y2)
	 */
	void build_tree_structure(const std::string& filename);

	/**
	 * Assumption:
	 * The tree contains the whole Road Network;
	 */
	void insertAgent(Agent * agent);

	//	/**
	//	 *Currently, when new vehicle is insert, the original node is used for locationing.
	//	 */
	void insertAgentBasedOnOD(Agent * agent);

	/**
	 * Return all agents in the box, called by each agent each time step;
	 * Thus, should be efficient
	 */
	std::vector<Agent const *> rangeQuery(BoundingBox & box) const;

	/**
	 * Automatically Update Internal Agents' Locations
	 */
	void updateAllInternalAgents();

	/**
	 *DEBUG purpose
	 */
	void display();

	//	/**
	//	 *DEBUG purpose
	//	 */
	//	void compareWithActiveAgent();

	/**
	 *
	 */
	void measureUnbalance(int time_step);

	/**
	 *
	 */
	void rebalance();

	/**
	 * DEBUG
	 */
	void checkLeaf();

private:

	//release memory
//	void rebuildSimTree(TreeNode * root, int tree_height, int target_tree_height, int from_index_x, int from_index_y, int to_index_x, int to_index_y);

	//release memory
	void releaseTreeMemory();

	//
	void releaseNodeMemory(TreeItem* item);

	//
	void countLeaf();

	//
	void addNodeToFather(std::size_t father_id, TreeNode * from_node, TreeItem* new_node);

	//must be a node ID
	//	TreeNode * getNodeByID(std::size_t item_id);

	//
	//	void searchNodeByID(TreeNode * target_node, TreeNode * from_node, std::size_t target_node_id);

	//
	void connectLeaf();

	//
	void connectLeafs(TreeNode * one_node);

	//
	TreeLeaf* get_rightest_leaf(TreeItem* item);

	//
	TreeLeaf* get_leftest_leaf(TreeItem* item);

	//
	BoundingBox location_bounding_box(Agent * agent);

	//
	BoundingBox OD_bounding_box(Agent * agent);

	//
	void insertAgentEncloseBox(Agent * agent, BoundingBox & agent_box, TreeItem* item);

	//
	//	void rangeQueryOverlapBox(BoundingBox  & box, TreeItem* item, std::vector<Agent  *>& result) ;

	//
	void display(TreeItem* item, int level);
};



struct TreeItem {
	SimRTree::BoundingBox bound;
	bool is_leaf;
	std::size_t item_id;
};

struct TreeLeaf: TreeItem {
	std::vector<Agent*> agent_buffer;
	TreeLeaf* next;
};

struct TreeNode: TreeItem {
	std::vector<TreeItem*> items;
};



}
