//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SimRTree.h"

#include <string>
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "geospatial/BusStop.hpp"

//#include "../../../short/entities/roles/driver/Driver.hpp"

using namespace sim_mob;

namespace {

Point2D WayPointToLocation(const WayPoint& wp) {
	if (wp.type_ == WayPoint::NODE) {
		return wp.node_->location;
	}
	if (wp.type_ == WayPoint::BUS_STOP) {
		return Point2D(wp.busStop_->xPos, wp.busStop_->yPos);
	}

	//TODO: Exception?
	return Point2D(0, 0);
}

} //End unnamed namespace

namespace {
class Profiling_Counting_SIM{
public:
	static long counting;
	static std::vector<long> count_buff;
};

long Profiling_Counting_SIM::counting = 0;
std::vector<long> Profiling_Counting_SIM::count_buff;
}

#ifdef SIM_TREE_USE_REBALANCE

long SimRTree::network_minimum_x = 0;
long SimRTree::network_minimum_y = 0;
long SimRTree::network_maximum_x = 0;
long SimRTree::network_maximum_y = 0;

int SimRTree::division_x_unit_ = 0;
int SimRTree::division_y_unit_ = 0;

double SimRTree::maximum_Rectanle_Weight = 8;
double SimRTree::minimum_Rectanle_Border_Length = 1;

//int SimRTree::map_division_x_max = 1000;
//int SimRTree::map_division_y_max = 1000;

int SimRTree::bigtable[1000][1000];

struct BigTableUpdate: std::unary_function<const Entity *, void> {
	void operator()(const Entity * item) {
		const Person* person = dynamic_cast<const Person*>(item);
		if (person && person->xPos > 0 && person->yPos > 0) {

			int x = (person->xPos - SimRTree::network_minimum_x) / SimRTree::division_x_unit_;
			int y = (person->yPos - SimRTree::network_minimum_y) / SimRTree::division_y_unit_;

			if (x >= 1000 || x < 0 || y >= 1000 || y < 0)
				return;

//			std::cout << person->xPos << "," << person->yPos << "," << x << "," << y << std::endl;

			SimRTree::bigtable[y][x]++;
		}
	}
};
#endif

/**
 *
 */

struct Collecting_Visitor_sim {
	const bool ContinueVisiting;
	std::vector<Agent const *> & array; // must be a reference.
	SimRTree::BoundingBox & box;

	explicit Collecting_Visitor_sim(std::vector<Agent const *> & array_, SimRTree::BoundingBox & box_) :
			ContinueVisiting(true), array(array_), box(box_) {
	}

	// When called, the visitor saves the agent in <array>.
	bool operator()(TreeLeaf * leaf) {
		//		std::cout << "FFF"  << std::endl;

		if (box.encloses(leaf->bound)) {

#ifdef MEAUSURE_COUNTS
			static long rangeQuery_saving_counts = 0;
			rangeQuery_saving_counts++;

			if(rangeQuery_saving_counts % 1000 == 0)
			std::cout << "rangeQuery_saving_counts:" << rangeQuery_saving_counts << std::endl;
#endif

			array.insert(array.end(), leaf->agent_buffer.begin(), leaf->agent_buffer.end());
		} else {
			for (std::vector<Agent*>::iterator itr = leaf->agent_buffer.begin(); itr != leaf->agent_buffer.end(); itr++) {
				Agent * one_agent = (*itr);

				if (box.enclose_point(one_agent->xPos, one_agent->yPos)) {
					array.push_back(one_agent);
				}
			}
		}
		//		array.push_back(leaf->agent_buffer);
		return true;
	}
};

struct AcceptEnclosing {
	const SimRTree::BoundingBox &m_bound;
	explicit AcceptEnclosing(const SimRTree::BoundingBox &bound) :
			m_bound(bound) {
	}

	bool operator()(const TreeNode * node) const {
		return m_bound.overlaps(node->bound);
	}

	bool operator()(const TreeLeaf * leaf) const {
		//		std::cout << "EEE"  << std::endl;
		//		std::cout << m_bound.overlaps(leaf->bound)  << std::endl;
		//		std::cout << m_bound.encloses(leaf->bound)  << std::endl;
		return m_bound.overlaps(leaf->bound);
	}

	//private:
	//	AcceptEnclosing()
	//	{
	//	}
};

// visits a node if necessary
struct VisitFunctor: std::unary_function<const TreeItem *, void> {
	const AcceptEnclosing &accept;
	Collecting_Visitor_sim &visit;

	explicit VisitFunctor(const AcceptEnclosing &a, Collecting_Visitor_sim &v) :
			accept(a), visit(v) {
	}

	void operator()(TreeItem * item) {
		TreeLeaf * leaf = static_cast<TreeLeaf*>(item);

		//		std::cout << "DDD"  << std::endl;
		//		std::cout << "DDD:" << accept(leaf)  << std::endl;

		if (accept(leaf)) {
			//			std::cout << "DDD2222222222"  << std::endl;
			visit(leaf);
		}
	}
};

//loop the tree
struct QueryFunctor: std::unary_function<const TreeItem, void> {
	//NOTE: "visitor" should not be a reference, since it is set by a value-type.
	//      I am also making "accept" a value-type, even though this is not strictly
	//      required (because it seems that you are being careless with references). ~Seth
	const AcceptEnclosing accept;
	Collecting_Visitor_sim visitor;

	explicit QueryFunctor(AcceptEnclosing a, Collecting_Visitor_sim v) :
			accept(a), visitor(v) {
	}

	void operator()(TreeItem * item) {

#ifdef QUERY_PROFILING
		Profiling_Counting_SIM::counting++;
#endif

		if (item->is_leaf) {
			//			std::cout << "CCC"  << std::endl;

			TreeLeaf * leaf = static_cast<TreeLeaf *>(item);

			//			if (visitor.ContinueVisiting && accept(leaf))
			VisitFunctor(accept, visitor)(leaf);
		} else {
			//			std::cout << "BBB"  << std::endl;

			TreeNode * node = static_cast<TreeNode *>(item);

			if (visitor.ContinueVisiting && accept(node))
				for_each(node->items.begin(), node->items.end(), *this);
		}
	}
};

/**
 *Functor that used to rebuild the tree
 */
#ifdef SIM_TREE_USE_REBALANCE
struct DivideHorizontalDifference {
	explicit DivideHorizontalDifference() {
	}

	double operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int cut_index) {
		double cost_A = 0;
		double cost_B = 0;

		for (int i = from_values_y; i < to_values_y; i++) {
			for (int j = from_values_x; j < to_values_x; j++) {
				if (i < cut_index) {
					cost_A += SimRTree::bigtable[i][j];
				} else {
					cost_B += SimRTree::bigtable[i][j];
				}
			}
		}

		return cost_A - cost_B;
	}
};

struct DivideVerticalDifference {
	explicit DivideVerticalDifference() {
	}

	double operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int cut_index) {
		double cost_A = 0;
		double cost_B = 0;

		for (int i = from_values_y; i < to_values_y; i++) {
			for (int j = from_values_x; j < to_values_x; j++) {
				if (j < cut_index) {
					cost_A += SimRTree::bigtable[i][j];
				} else {
					cost_B += SimRTree::bigtable[i][j];
				}
			}
		}

		return cost_A - cost_B;
	}
};

struct ShouldStopAsALeaf {
	explicit ShouldStopAsALeaf() {
	}

	bool operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y) {
		if (to_values_x - from_values_x <= SimRTree::minimum_Rectanle_Border_Length && to_values_y - from_values_y <= SimRTree::minimum_Rectanle_Border_Length)
			return true;

		long sum_weight = 0;
		for (int i = from_values_y; i < to_values_y; i++) {
			for (int j = from_values_x; j < to_values_x; j++) {
				sum_weight += SimRTree::bigtable[i][j];
			}
		}

		if (sum_weight <= SimRTree::maximum_Rectanle_Weight)
			return true;

		return false;
	}
};

struct ShouldStopAsANode {
	explicit ShouldStopAsANode() {
	}

	bool operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y) {
//		if (to_values_x - from_values_x <= SimRTree::minimum_Rectanle_Border_Length * 2 && to_values_y - from_values_y <= SimRTree::minimum_Rectanle_Border_Length * 2)
//			return true;

		long sum_weight = 0;
		for (int i = from_values_y; i < to_values_y; i++) {
			for (int j = from_values_x; j < to_values_x; j++) {
				sum_weight += SimRTree::bigtable[i][j];
			}
		}

		if (sum_weight <= SimRTree::maximum_Rectanle_Weight * 2)
			return true;

		return false;
	}
};

struct DivideHorizontal {
	explicit DivideHorizontal() {
	}

	void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int* best_cut_point, double * best_cost) {
		//too small to cut
		if (to_values_y <= from_values_y + 1)
			return;

		int to_values_y_ = to_values_y;
		int from_values_y_ = from_values_y;
		double best_difference = std::numeric_limits<double>::max();
		int best_cut = -1;

		int middle_cut = 0;
		bool reach_border = false;

		while (to_values_y_ > from_values_y_ && reach_border == false) {
			if (to_values_y_ == from_values_y_ + 1)
				reach_border = true;

			middle_cut = (to_values_y_ + from_values_y_) / 2;
			double difference = DivideHorizontalDifference()(from_values_x, from_values_y, to_values_x, to_values_y, middle_cut);

//			std::cout << "difference" << difference << std::endl;

			if (difference == 0) {
				best_difference = 0;
				best_cut = middle_cut;
				break;
			} else if (difference < 0) {
				from_values_y_ = middle_cut;

				if (best_difference > -1 * difference) {
					best_cut = middle_cut;
					best_difference = -1 * difference;
				}
			} else if (difference > 0) {
				to_values_y_ = middle_cut;

				if (best_difference > difference) {
					best_cut = middle_cut;
					best_difference = difference;
				}
			}
		}

		*best_cut_point = best_cut;
		*best_cost = best_difference;
	}
};

struct DivideVertical {
	explicit DivideVertical() {
	}

	void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int* best_cut_point, double * best_cost) {
		//too small to cut
		if (to_values_x <= from_values_x + 1) {
//			std::cout << "to_values_x " << to_values_x << " from_values_x" << from_values_x << std::endl;
//			std::cout << "to_values_y " << to_values_x << " from_values_y" << from_values_x << std::endl;

			return;
		}

		int to_values_x_ = to_values_x;
		int from_values_x_ = from_values_x;
		double best_difference = std::numeric_limits<double>::max();
		int best_cut = -1;

		int middle_cut = 0;
		bool reach_border = false;

		while (to_values_x_ > from_values_x_ && reach_border == false) {
			if (to_values_x_ == from_values_x_ + 1)
				reach_border = true;

			middle_cut = (to_values_x_ + from_values_x_) / 2;
			double difference = DivideVerticalDifference()(from_values_x, from_values_y, to_values_x, to_values_y, middle_cut);

//			std::cout << "difference" << difference << std::endl;

			if (difference == 0) {
				best_difference = 0;
				best_cut = middle_cut;
				break;
			} else if (difference < 0) {
				from_values_x_ = middle_cut;

				if (best_difference > -1 * difference) {
					best_cut = middle_cut;
					best_difference = -1 * difference;
				}
			} else if (difference > 0) {
				to_values_x_ = middle_cut;

				if (best_difference > difference) {
					best_cut = middle_cut;
					best_difference = difference;
				}
			}
		}

		*best_cut_point = best_cut;
		*best_cost = best_difference;
	}
};

/*
 struct RebuildSimTreeNodeFunctor {
 //	int from_values_x;
 //	int from_values_y;
 //	int to_values_x;
 //	int to_values_y;
 //	int current_level;
 //	int target_level;
 //	TreeNode * parent;

 explicit RebuildSimTreeNodeFunctor() {
 }

 void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int current_level, int target_level, TreeNode * parent_) {
 //too small to cut, so make a leaf
 if (to_values_x - from_values_x <= 1 && to_values_y - from_values_y <= 1) {
 TreeLeaf* one_leaf = new TreeLeaf();

 one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
 one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
 one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
 one_leaf->bound.edges[1].second = SimRTree::network_maximum_y + SimRTree::division_y_unit_ * to_values_y;
 one_leaf->item_id = parent_->item_id * 2;
 one_leaf->is_leaf = true;
 one_leaf->agent_buffer.clear();

 parent_->items.push_back(one_leaf);

 return;
 }

 //try to cut y
 int where_to_divide_h = -1;
 double cost_to_divide_h = std::numeric_limits<double>::max();
 DivideHorizontal()(from_values_x, from_values_y, to_values_x, to_values_y, &where_to_divide_h, &cost_to_divide_h);

 //		std::cout << "where_to_divide_h:" << where_to_divide_h << std::endl;
 //		std::cout << "cost_to_divide_h:" << cost_to_divide_h << std::endl;

 //try to cut x
 int where_to_divide_v = -1;
 double cost_to_divide_v = std::numeric_limits<double>::max();
 DivideVertical()(from_values_x, from_values_y, to_values_x, to_values_y, &where_to_divide_v, &cost_to_divide_v);

 //		std::cout << "where_to_divide_v:" << where_to_divide_v << std::endl;
 //		std::cout << "cost_to_divide_v:" << cost_to_divide_v << std::endl;

 if (cost_to_divide_h >= cost_to_divide_v) {
 //			std::cout << "cost_to_divide_h is better:" << where_to_divide_h << std::endl;
 //build leaf
 if (current_level >= target_level) {
 TreeLeaf* one_leaf = new TreeLeaf();

 one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
 one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
 one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
 one_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
 one_leaf->item_id = parent_->item_id * 2;
 one_leaf->is_leaf = true;
 one_leaf->agent_buffer.clear();

 parent_->items.push_back(one_leaf);

 TreeLeaf* second_leaf = new TreeLeaf();

 second_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
 second_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
 second_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
 second_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
 second_leaf->item_id = parent_->item_id * 2 + 1;
 second_leaf->is_leaf = true;
 second_leaf->agent_buffer.clear();

 parent_->items.push_back(second_leaf);
 }
 //build node
 else {
 TreeNode* one_node = new TreeNode();
 one_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
 one_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
 one_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
 one_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
 one_node->item_id = parent_->item_id * 2;
 one_node->is_leaf = false;
 one_node->items.clear();
 parent_->items.push_back(one_node);
 RebuildSimTreeNodeFunctor()(from_values_x, from_values_y, to_values_x, where_to_divide_h, current_level + 1, target_level, one_node);

 TreeNode* second_node = new TreeNode();
 second_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
 second_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
 second_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
 second_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
 second_node->item_id = parent_->item_id * 2 + 1;
 second_node->is_leaf = false;
 second_node->items.clear();
 parent_->items.push_back(second_node);
 RebuildSimTreeNodeFunctor()(from_values_x, where_to_divide_h, to_values_x, to_values_y, current_level + 1, target_level, second_node);
 }
 }
 //cut Vertically
 else {
 //			std::cout << "cost_to_divide_v is better:" << std::endl;
 //			std::cout << "it is right to go here, but check where_to_divide_v:" << where_to_divide_v << ", and SimRTree::division_x_unit_:" << SimRTree::division_x_unit_ << std::endl;

 //build leaf
 if (current_level >= target_level) {
 TreeLeaf* one_leaf = new TreeLeaf();

 one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
 one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
 one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
 one_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
 one_leaf->item_id = parent_->item_id * 2;
 one_leaf->is_leaf = true;
 one_leaf->agent_buffer.clear();

 parent_->items.push_back(one_leaf);

 TreeLeaf* second_leaf = new TreeLeaf();

 second_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
 second_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
 second_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
 second_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
 second_leaf->item_id = parent_->item_id * 2 + 1;
 second_leaf->is_leaf = true;
 second_leaf->agent_buffer.clear();

 parent_->items.push_back(second_leaf);
 }
 //build node
 else {
 TreeNode* one_node = new TreeNode();
 one_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
 one_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
 one_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
 one_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
 one_node->item_id = parent_->item_id * 2;
 one_node->is_leaf = false;
 one_node->items.clear();
 parent_->items.push_back(one_node);
 RebuildSimTreeNodeFunctor()(from_values_x, from_values_y, where_to_divide_v, to_values_y, current_level + 1, target_level, one_node);

 TreeNode* second_node = new TreeNode();
 second_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
 second_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (to_values_x);
 second_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
 second_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
 second_node->item_id = parent_->item_id * 2 + 1;
 second_node->is_leaf = false;
 second_node->items.clear();
 parent_->items.push_back(second_node);
 RebuildSimTreeNodeFunctor()(where_to_divide_v, from_values_y, to_values_x, to_values_y, current_level + 1, target_level, second_node);
 }
 }
 }
 };
 */

/**
 * New Version Rebuild
 */
struct RebuildSimTreeNodeFunctor {
	//	int from_values_x;
	//	int from_values_y;
	//	int to_values_x;
	//	int to_values_y;
	//	int current_level;
	//	int target_level;
	//	TreeNode * parent;

	explicit RebuildSimTreeNodeFunctor() {
	}

	void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, TreeNode * parent_, bool must_stop) {
		//too small to cut, so make a leaf

		if ((must_stop == true) || (ShouldStopAsALeaf()(from_values_x, from_values_y, to_values_x, to_values_y) == true)) {
			TreeLeaf* one_leaf = new TreeLeaf();

			one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
			one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
			one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
			one_leaf->bound.edges[1].second = SimRTree::network_maximum_y + SimRTree::division_y_unit_ * to_values_y;
			one_leaf->item_id = parent_->item_id * 2;
			one_leaf->is_leaf = true;
			one_leaf->agent_buffer.clear();

			one_leaf->father = parent_;
			parent_->items.push_back(one_leaf);

			return;
		}

		//try to cut y
		int where_to_divide_h = -1;
		double cost_to_divide_h = std::numeric_limits<double>::max();
		DivideHorizontal()(from_values_x, from_values_y, to_values_x, to_values_y, &where_to_divide_h, &cost_to_divide_h);

//		std::cout << "where_to_divide_h:" << where_to_divide_h << std::endl;
//		std::cout << "cost_to_divide_h:" << cost_to_divide_h << std::endl;

		//try to cut x
		int where_to_divide_v = -1;
		double cost_to_divide_v = std::numeric_limits<double>::max();
		DivideVertical()(from_values_x, from_values_y, to_values_x, to_values_y, &where_to_divide_v, &cost_to_divide_v);

//		std::cout << "where_to_divide_v:" << where_to_divide_v << std::endl;
//		std::cout << "cost_to_divide_v:" << cost_to_divide_v << std::endl;

		//DivideHorizontal
		if (cost_to_divide_h <= cost_to_divide_v) {
			if (ShouldStopAsANode()(from_values_x, from_values_y, to_values_x, to_values_y)) {
				TreeLeaf* one_leaf = new TreeLeaf();

				one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				one_leaf->item_id = parent_->item_id * 2;
				one_leaf->is_leaf = true;
				one_leaf->agent_buffer.clear();
				one_leaf->father = parent_;

				parent_->items.push_back(one_leaf);

				TreeLeaf* second_leaf = new TreeLeaf();

				second_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				second_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				second_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				second_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_leaf->item_id = parent_->item_id * 2 + 1;
				second_leaf->is_leaf = true;
				second_leaf->agent_buffer.clear();
				second_leaf->father = parent_;

				parent_->items.push_back(second_leaf);
			} else {
				TreeNode* one_node = new TreeNode();
				one_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				one_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				one_node->item_id = parent_->item_id * 2;
				one_node->is_leaf = false;
				one_node->items.clear();
				one_node->father = parent_;
				parent_->items.push_back(one_node);
				RebuildSimTreeNodeFunctor()(from_values_x, from_values_y, to_values_x, where_to_divide_h, one_node, false);

				TreeNode* second_node = new TreeNode();
				second_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				second_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				second_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				second_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_node->item_id = parent_->item_id * 2 + 1;
				second_node->is_leaf = false;
				second_node->items.clear();
				second_node->father = parent_;
				parent_->items.push_back(second_node);
				RebuildSimTreeNodeFunctor()(from_values_x, where_to_divide_h, to_values_x, to_values_y, second_node, false);
			}
		}
		//DivideVertical
		else {
			if (ShouldStopAsANode()(from_values_x, from_values_y, to_values_x, to_values_y)) {
				TreeLeaf* one_leaf = new TreeLeaf();

				one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				one_leaf->item_id = parent_->item_id * 2;
				one_leaf->is_leaf = true;
				one_leaf->agent_buffer.clear();
				parent_->items.push_back(one_leaf);
				one_leaf->father = parent_;

				TreeLeaf* second_leaf = new TreeLeaf();

				second_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				second_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				second_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				second_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_leaf->item_id = parent_->item_id * 2 + 1;
				second_leaf->is_leaf = true;
				second_leaf->agent_buffer.clear();
				second_leaf->father = parent_;

				parent_->items.push_back(second_leaf);
			} else {
				TreeNode* one_node = new TreeNode();
				one_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				one_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				one_node->item_id = parent_->item_id * 2;
				one_node->is_leaf = false;
				one_node->items.clear();
				one_node->father = parent_;
				parent_->items.push_back(one_node);
				RebuildSimTreeNodeFunctor()(from_values_x, from_values_y, where_to_divide_v, to_values_y, one_node, false);

				TreeNode* second_node = new TreeNode();
				second_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				second_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (to_values_x);
				second_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				second_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_node->item_id = parent_->item_id * 2 + 1;
				second_node->is_leaf = false;
				second_node->items.clear();
				second_node->father = parent_;
				parent_->items.push_back(second_node);
				RebuildSimTreeNodeFunctor()(where_to_divide_v, from_values_y, to_values_x, to_values_y, second_node, false);
			}
		}
	}
};

#endif

struct Get_rightest_leaf {
	explicit Get_rightest_leaf() {
	}

	TreeLeaf* operator()(TreeItem* item) {
		if (!item->is_leaf) {
			TreeNode* node = static_cast<TreeNode*>(item);
			int child_size = node->items.size();
			assert(child_size > 0);

			return Get_rightest_leaf()(node->items[child_size - 1]);
		} else {
			TreeLeaf* leaf = static_cast<TreeLeaf*>(item);
			return leaf;
		}
	}
};

struct Get_leftest_leaf {
	explicit Get_leftest_leaf() {
	}

	TreeLeaf* operator()(TreeItem* item) {
		if (!item->is_leaf) {
			TreeNode* node = static_cast<TreeNode*>(item);
			int child_size = node->items.size();
			assert(child_size > 0);

			return Get_leftest_leaf()(node->items[0]);
		} else {
			TreeLeaf* leaf = static_cast<TreeLeaf*>(item);
			return leaf;
		}
	}
};

struct ConnectLeafs {
	explicit ConnectLeafs() {
	}

	void operator()(TreeNode * one_node) {
		int child_size = one_node->items.size();
		if (child_size <= 1)
			return;

		for (int i = 1; i < child_size; i++) {
			TreeLeaf* right_left = Get_rightest_leaf()(one_node->items[i - 1]);
			TreeLeaf* left_left = Get_leftest_leaf()(one_node->items[i]);

			assert(right_left != NULL);
			assert(left_left != NULL);

			right_left->next = left_left;
		}

		for (int i = 0; i < child_size; i++) {
			if (!one_node->items[i]->is_leaf) {
				TreeNode* node = static_cast<TreeNode*>(one_node->items[i]);
				ConnectLeafs()(node);
			}
		}
	}
};

/**
 * Implement
 */

void sim_mob::SimRTree::build_tree_structure(const std::string& filename) {
	std::ifstream fin(filename.c_str());
	if (!fin) {
		std::cerr << "Cannot open Density Pattern file: " << filename << " " << std::endl;
		return;
	}

	int parent_id, self_id; //IDs used to build the structure
	int is_leaf; //1:YES; 0:NO
	long start_x, start_y, end_x, end_y;

#ifdef SIM_TREE_USE_REBALANCE
	bool first_line = true;
#endif

	while (fin) {
		fin >> parent_id >> self_id >> start_x >> start_y >> end_x >> end_y >> is_leaf;
		if (!fin.good())
			continue; // skip newlines, etc.

		//the parameters are used in re-balance
#ifdef SIM_TREE_USE_REBALANCE
		if (first_line) {
			first_line = false;

			SimRTree::network_minimum_x = start_x;
			SimRTree::network_minimum_y = start_y;
			SimRTree::network_maximum_x = end_x;
			SimRTree::network_maximum_y = end_y;

			SimRTree::division_x_unit_ = (int) (ceil((end_x - start_x) * 1.0 / 1000));
			SimRTree::division_y_unit_ = (int) (ceil((end_y - start_y) * 1.0 / 1000));

//			std::cout << " SimRTree::division_x_unit_! " << SimRTree::division_x_unit_ << std::endl;
//			std::cout << " network_minimum_x " << SimRTree::network_minimum_x << std::endl;
//			std::cout << " network_minimum_y " << SimRTree::network_minimum_y << std::endl;
//			std::cout << " network_maximum_x " << SimRTree::network_maximum_x << std::endl;
//			std::cout << " network_maximum_y " << SimRTree::network_maximum_y << std::endl;
		}
#endif

		//used by both node and leaf
		SimRTree::BoundingBox box;
		box.edges[0].first = start_x;
		box.edges[0].second = end_x;
		box.edges[1].first = start_y;
		box.edges[1].second = end_y;

		if (is_leaf == 0) {
			//if the node is root
			if (parent_id == 0) {
				m_root = new TreeNode();
				m_root->is_leaf = false;
				m_root->item_id = self_id;

				m_root->items.reserve(2);
				m_root->bound = box;
				m_root->father = NULL;
			} else {
				TreeNode* one_node = new TreeNode();
				one_node->is_leaf = false;
				one_node->item_id = self_id;

				one_node->items.reserve(2);
				one_node->bound = box;

				addNodeToFather(parent_id, m_root, one_node);
				//				TreeNode* parent = static_cast<TreeNode*> (getNodeByID(parent_id));
				//				parent->items.push_back(one_node);
			}
		}
		//if it is a leaf
		else {
			TreeLeaf* one_leaf = new TreeLeaf();
			one_leaf->next = NULL;

			one_leaf->is_leaf = true;
			one_leaf->item_id = self_id;

			one_leaf->bound = box;

			addLeafToFather(parent_id, m_root, one_leaf);

			//			TreeNode* parent = static_cast<TreeNode*> (getNodeByID(parent_id));
			//			parent->items.push_back(one_leaf);
		}
	}

	fin.close();

	//xuyan:make sure it is done, after the tree is built.
	connectLeaf();

	//xuyan: for re-balance
	countLeaf();

	//temp, should remove
	//	rebalance();
}

void sim_mob::SimRTree::connectLeaf() {
	ConnectLeafs()(m_root);
	first_leaf = Get_leftest_leaf()(m_root);
}

//void sim_mob::SimRTree::connectLeafs(TreeNode * one_node) {
//	int child_size = one_node->items.size();
//	if (child_size <= 1)
//		return;
//
//	for (int i = 1; i < child_size; i++) {
//		TreeLeaf* right_left = Get_rightest_leaf()(one_node->items[i - 1]);
//		TreeLeaf* left_left = Get_leftest_leaf()(one_node->items[i]);
//
//		assert(right_left != NULL);
//		assert(left_left != NULL);
//
//		right_left->next = left_left;
//	}
//
//	for (int i = 0; i < child_size; i++) {
//		if (!one_node->items[i]->is_leaf) {
//			TreeNode* node = static_cast<TreeNode*>(one_node->items[i]);
//			connectLeafs(node);
//		}
//	}
//}

/**
 *
 */
//TreeLeaf* sim_mob::SimRTree::get_rightest_leaf(TreeItem* item) {
//	if (!item->is_leaf) {
//		TreeNode* node = static_cast<TreeNode*>(item);
//		int child_size = node->items.size();
//		assert(child_size > 0);
//
//		return get_rightest_leaf(node->items[child_size - 1]);
//	} else {
//		TreeLeaf* leaf = static_cast<TreeLeaf*>(item);
//		return leaf;
//	}
//}
//TreeLeaf* sim_mob::SimRTree::get_leftest_leaf(TreeItem* item) {
//	if (!item->is_leaf) {
//		TreeNode* node = static_cast<TreeNode*>(item);
//		int child_size = node->items.size();
//		assert(child_size > 0);
//
//		return get_leftest_leaf(node->items[0]);
//	} else {
//		TreeLeaf* leaf = static_cast<TreeLeaf*>(item);
//		return leaf;
//	}
//}
/**
 *not implemented
 */
void sim_mob::SimRTree::releaseTreeMemory() {
	releaseNodeMemory(m_root);
}

void sim_mob::SimRTree::releaseNodeMemory(TreeItem* item) {
	if (!item->is_leaf) {
		TreeNode* node = static_cast<TreeNode*>(item);
		for (unsigned int i = 0; i < node->items.size(); i++) {
			releaseNodeMemory(node->items[i]);
		}
	}

	if (item)
		delete item;
}

/**
 *
 */
void sim_mob::SimRTree::countLeaf() {
	leaf_counts = 0;
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		leaf_counts++;
		one_leaf = one_leaf->next;
	}
}

/**
 *
 */
void sim_mob::SimRTree::addNodeToFather(std::size_t father_id, TreeNode * from_node, TreeNode* new_node) {
	if (from_node->is_leaf)
		return;

	if (from_node->item_id == father_id) {
		from_node->items.push_back(new_node);
		new_node->father = from_node;
		return;
	}

	for (unsigned int i = 0; i < from_node->items.size(); i++) {
		if (!from_node->items[i]->is_leaf) {
			TreeNode* one_node = static_cast<TreeNode*>(from_node->items[i]);

			addNodeToFather(father_id, one_node, new_node);
		}
	}
}

/**
 *
 */
void sim_mob::SimRTree::addLeafToFather(std::size_t father_id, TreeNode * from_node, TreeLeaf* new_leaf) {
	if (from_node->is_leaf)
		return;

	if (from_node->item_id == father_id) {
		from_node->items.push_back(new_leaf);
		new_leaf->father = from_node;
		return;
	}

	for (unsigned int i = 0; i < from_node->items.size(); i++) {
		if (!from_node->items[i]->is_leaf) {
			TreeNode* one_node = static_cast<TreeNode*>(from_node->items[i]);

			addLeafToFather(father_id, one_node, new_leaf);
		}
	}
}

/**
 *
 */
SimRTree::BoundingBox sim_mob::SimRTree::location_bounding_box(Agent * agent) {

//	Person* one_person = dynamic_cast<Person *> (agent);
//	Driver* one_driver = dynamic_cast<Driver*> (one_person->getRole());
//
//	int x = 0;
//	int y = 0;
//
//	if(one_driver)
//	{
//		x = static_cast<int>(one_driver->getVehicle()->getX());
//		y = static_cast<int>(one_driver->getVehicle()->getY());
//	}

	SimRTree::BoundingBox box;
	box.edges[0].first = box.edges[0].second = agent->xPos.get(); //->xPos_Sim;
	box.edges[1].first = box.edges[1].second = agent->yPos.get(); //yPos_Sim;

//	std::cout << "ID:" << agent->getId() << ",Agent:xPos" << agent->xPos.get();
//	std::cout << ",Agent:yPos" << agent->yPos.get() << std::endl;

	return box;
}

SimRTree::BoundingBox sim_mob::SimRTree::OD_bounding_box(Agent * agent) {
	//Retrieve the location (Point) of the OriginWayPoint.
	//TODO: This only occurs here, so I'm removing it from the WayPoint class.
	//      We need a better way to do this; it really has nothing to do with WayPoints ~Seth.
	Point2D originLoc = WayPointToLocation(agent->originNode);
	Point2D densiLoc = WayPointToLocation(agent->destNode);

	SimRTree::BoundingBox box;
	box.edges[0].first = box.edges[0].second = originLoc.getX();
	box.edges[1].first = box.edges[1].second = originLoc.getY();

//	std::cout << "Agent:originLocX:" << originLoc.getX();
//	std::cout << ",Agent:originLocY:" << originLoc.getY();
//	std::cout << "Agent:densiLocX:" << densiLoc.getX();
//	std::cout << ",Agent:densiLocY:" << densiLoc.getY() << std::endl;

	return box;
}

void sim_mob::SimRTree::insertAgent(Agent* agent, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap) {
	SimRTree::BoundingBox agent_box = location_bounding_box(agent);
	insertAgentEncloseBox(agent, agent_box, (m_root), connectorMap);
}

void sim_mob::SimRTree::insertAgentBasedOnOD(Agent* agent, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap) {
	SimRTree::BoundingBox agent_box = OD_bounding_box(agent);
	insertAgentEncloseBox(agent, agent_box, (m_root), connectorMap);
}

void sim_mob::SimRTree::insertAgentEncloseBox(Agent* agent, SimRTree::BoundingBox & agent_box, TreeItem* item, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap) {
	std::queue<TreeItem*> items;

	items.push(m_root);

	while (!items.empty()) {
		TreeItem* item = items.front();
		items.pop();

		if (item->is_leaf) {
			TreeLeaf* one_leaf = static_cast<TreeLeaf*>(item);

//			std::cout << one_leaf->agent_buffer[0]

			(one_leaf->agent_buffer).push_back(agent);

			connectorMap[agent] = one_leaf;
			//agent->connector_to_Sim_Tree = one_leaf;

			return;
		} else {
			TreeNode* node = static_cast<TreeNode*>(item);
			for (unsigned int i = 0; i < node->items.size(); i++) {
				if (node->items[i]->bound.encloses(agent_box)) {
					items.push(node->items[i]);
				}
			}
		}
	} //end of WHILE LOOP
}

/**
 * Query
 */

std::vector<Agent const*> sim_mob::SimRTree::rangeQuery(SimRTree::BoundingBox & box) const {
	std::vector<Agent const*> result;

	if (m_root) {
#ifdef QUERY_PROFILING
		Profiling_Counting_SIM::counting = 0;
#endif

		QueryFunctor query(AcceptEnclosing(box), Collecting_Visitor_sim(result, box));
		query(m_root);

#ifdef QUERY_PROFILING
		long one = Profiling_Counting_SIM::counting;
		Profiling_Counting_SIM::count_buff.push_back(one);

		if (Profiling_Counting_SIM::count_buff.size() % 1000000 == 0) {
			double sum = 0;

			for (int i = 0; i < Profiling_Counting_SIM::count_buff.size(); i++) {
				sum += Profiling_Counting_SIM::count_buff[i];
			}

			std::cout << "Count:" << Profiling_Counting_SIM::count_buff.size() << ", Sum Cost:" << sum << std::endl;
		}
#endif
	}

	return result;
}

/**
 * Query
 */

std::vector<Agent const*> sim_mob::SimRTree::rangeQuery(SimRTree::BoundingBox & box, TreeItem* item) const {
	std::vector<Agent const*> result;

#ifdef MEAUSURE_COUNTS
	static long rangeQuery_counts = 0;
	rangeQuery_counts++;
	if(rangeQuery_counts % 10000 == 0)
	std::cout << "rangeQuery_counts:" << rangeQuery_counts << std::endl;
#endif

	//Find the start node for Query
	TreeItem* from_node = item;
//	std::cout << "------------------------" << std::endl;
//	std::cout << "box:" << box.edges[0].first << "," << box.edges[0].second << std::endl;
//	std::cout << "box:" << box.edges[1].first << "," << box.edges[1].second << std::endl;

	while (from_node->bound.encloses(box) == false) {
//		std::cout << "&&&&&&&&&&&&&&&" << std::endl;
//
//		std::cout << "box:" << from_node->bound.edges[0].first << "," << from_node->bound.edges[0].second << std::endl;
//		std::cout << "box:" << from_node->bound.edges[1].first << "," << from_node->bound.edges[1].second << std::endl;

		from_node = from_node->father;

#ifdef MEAUSURE_COUNTS
		static long rangeQuery_enclose_counts = 0;
		rangeQuery_enclose_counts++;

		if(rangeQuery_enclose_counts % 10000 == 0)
		std::cout << "rangeQuery_enclose_counts:" << rangeQuery_enclose_counts << std::endl;
#endif

		//sometimes users are query outside of map boundary
		if (from_node == NULL) {
			from_node = m_root;
			break;
		}
	}

	if (from_node) {
#ifdef QUERY_PROFILING
		Profiling_Counting_SIM::counting = 0;
#endif
		QueryFunctor query(AcceptEnclosing(box), Collecting_Visitor_sim(result, box));
		query(from_node);
#ifdef QUERY_PROFILING
		long one = Profiling_Counting_SIM::counting;
		Profiling_Counting_SIM::count_buff.push_back(one);

		if (Profiling_Counting_SIM::count_buff.size() % 1000000 == 0) {
			double sum = 0;

			for (int i = 0; i < Profiling_Counting_SIM::count_buff.size(); i++) {
				sum += Profiling_Counting_SIM::count_buff[i];
			}

			std::cout << "Count:" << Profiling_Counting_SIM::count_buff.size() << ", Sum Cost:" << sum << std::endl;
		}
#endif
	}

	return result;
}

/**
 *
 */
void sim_mob::SimRTree::updateAllInternalAgents(std::map<const sim_mob::Agent*, TreeItem*>& connectorMap) {
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
//		std::vector<Agent *> list = one_leaf->agent_buffer;
		SimRTree::BoundingBox box = one_leaf->bound;

		unsigned int offset = 0;
		while (offset < one_leaf->agent_buffer.size()) {
			//Case 1: the agent should be removed from the Sim-R Tree
			Agent * one_agent = (one_leaf->agent_buffer[offset]);

			if (one_agent->can_remove_by_RTREE) {
				//one_agent->connector_to_Sim_Tree = nullptr;
				std::map<const sim_mob::Agent*, TreeItem*>::iterator it = connectorMap.find(one_agent);
				if (it!=connectorMap.end()) {
					connectorMap.erase(it);
				}

//				std::cout << "one_agent->isToBeRemoved():" << one_agent->getId() << std::endl;
				one_leaf->agent_buffer.erase(one_leaf->agent_buffer.begin() + (offset));
				continue;
			}

			if (one_agent->xPos.get() <= 0 || one_agent->yPos.get() <= 0) {
//				std::cout << "not start yet! ID:" << one_agent->getId() << std::endl;
				offset++;
				continue;
			}

			SimRTree::BoundingBox agent_box = location_bounding_box(one_agent);

			//Case 2: the agent should be in the same box
			//Case 2: It should be the most happen case.
			if (box.encloses(agent_box)) {
//				std::cout << "encloses" << std::endl;
				offset++;
				continue;
			}

			//Case 3: The agent should be moved to a different box
			//Case 3: If the agent was insert into a box which is not scanned, the agent will be checked again. It will cost some CPU. But, since Case 3 is not much, the cost should be accpatable.
			//Case 3: If the cost is heavy, need to define another variable in agent to skip re-check.
			else {
				Agent * one_agent = one_leaf->agent_buffer[offset];

				//one_agent->connector_to_Sim_Tree = nullptr;
				std::map<const sim_mob::Agent*, TreeItem*>::iterator it = connectorMap.find(one_agent);
				if (it!=connectorMap.end()) {
					connectorMap.erase(it);
				}

				std::cout << "one_agent->check():" << one_agent->getId() << std::endl;
				one_leaf->agent_buffer.erase(one_leaf->agent_buffer.begin() + (offset));
				insertAgent(one_agent, connectorMap);
			}
		} // end of inside while

		one_leaf = one_leaf->next;
	} // end of out while loop
}
/**
 *
 */

void sim_mob::SimRTree::display() {
	std::cout << "#########################################" << std::endl;
	display(m_root, 1);
}

void sim_mob::SimRTree::display(TreeItem* item, int level) {
	if (!item->is_leaf) {
		std::cout << "level:" << level << "(" << item->bound.edges[0].first << "," << item->bound.edges[1].first << "," << item->bound.edges[0].second << "," << item->bound.edges[1].second << ")"
				<< std::endl;

		TreeNode* one_node = static_cast<TreeNode*>(item);
		int child_size = one_node->items.size();
		for (int i = 0; i < child_size; i++) {
			display((one_node->items[i]), level + 1);
		}
	} else {
		TreeLeaf* one_leaf = static_cast<TreeLeaf*>(item);
		std::cout << "level:" << level << "(" << item->bound.edges[0].first << "," << item->bound.edges[1].first << "," << item->bound.edges[0].second << "," << item->bound.edges[1].second << ")"
				<< "child size" << one_leaf->agent_buffer.size() << std::endl;

		//		std::cout << "--------------------" << std::endl;
		//
		//		for (unsigned int i = 0; i < one_leaf->agent_buffer.size(); i++)
		//		{
		//			std::cout << i << ":" << one_leaf->agent_buffer[i]->loc_x << "," << one_leaf->agent_buffer[i]->loc_y << std::endl;
		//		}
		//
		//		std::cout << "--------------------" << std::endl;
	}
}

/**
 * Debug
 */
void sim_mob::SimRTree::checkLeaf() {
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		int agents_size = one_leaf->agent_buffer.size();

//		std::cout << "agents_size:" << agents_size << std::endl;

		for (int i = 0; i < agents_size; i++) {
			if (one_leaf->agent_buffer[i]->isToBeRemoved()) {
				std::cout << "one_leaf->agent_buffer[i]->isToBeRemoved():" << one_leaf->agent_buffer[i]->getId() << std::endl;
			}
		}

		one_leaf = one_leaf->next;
	}
}

#ifdef SIM_TREE_USE_REBALANCE

/*
 * The parameters are only related with re-balance
 */
void sim_mob::SimRTree::init_rebalance_settings() {
	std::string lineBuf;

	std::ifstream fin("shared/spatial_trees/simtree/unbalance_settings.txt");
	if (!fin) {
		std::cerr << "Cannot open Density rebalance_settings file: " << std::endl;
		return;
	}

	if (fin.good()) {
		fin >> rebalance_threshold >> checking_frequency >> rebalance_load_balance_maximum;
	}

	fin.close();

//	std::ifstream fin("shared/spatial_trees/simtree/unbalance_settings.txt");
//	if (fin.is_open()) {
//		if (fin.good()) {
//			fin >> rebalance_threshold >> checking_frequency >> rebalance_load_balance_maximum;
//		}
//		fin.close();
//	}

	std::cout << "rebalance_threshold:" << rebalance_threshold << std::endl;
	std::cout << "checking_frequency:" << checking_frequency << std::endl;
	std::cout << "rebalance_load_balance_maximum:" << rebalance_load_balance_maximum << std::endl;
}

#endif

double inline calculate_unbalance_ratio(std::vector<int> agents_counts_in_tree, double average_counts) {
	int size = agents_counts_in_tree.size();
	if (size > 0) {
		double sum_diff = 0.0;

		for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
			sum_diff += abs(*it - average_counts) / average_counts;
		}

//		std::cout << "unbalance_ratio:" << (sum_diff / size) << std::endl;

		return sum_diff / size;
	} else {
		return 0.0;
	}
}

/**
 * Load Balance
 */
void sim_mob::SimRTree::measureUnbalance(int time_step) {
#ifdef SIM_TREE_USE_REBALANCE
	//check every 3 time step
	//hard coded
	//first k minutes, no rebalance;
	const int k = 1;
	if (time_step < k * 60 * 10)
		return;

	if (time_step % checking_frequency != 0)
		return;

	/**
	 * There is another measurement names: load ratio
	 */

	/*
	 double total_cost = 0;
	 double best_cost = 0;
	 int depth_cost = 20;

	 std::vector<int> agents_counts_in_tree;

	 leaf_agents_sum = 0;
	 unbalance_ratio = 0;

	 TreeLeaf* one_leaf = first_leaf;

	 while (one_leaf) {
	 int agents_size = one_leaf->agent_buffer.size();

	 total_cost += agents_size * (agents_size + depth_cost);

	 agents_counts_in_tree.push_back(agents_size);
	 leaf_agents_sum += agents_size;

	 one_leaf = one_leaf->next;
	 }

	 if (leaf_agents_sum <= 0)
	 return;

	 double average_counts = leaf_agents_sum / leaf_counts;
	 best_cost = average_counts * (average_counts + depth_cost) * leaf_counts;

	 //calculate un-balance ratio
	 double temp_value_for_unbalance_ratio = 0;
	 for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
	 temp_value_for_unbalance_ratio += pow((*it - average_counts), 2);
	 }

	 temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio * leaf_counts;
	 temp_value_for_unbalance_ratio = sqrt(temp_value_for_unbalance_ratio);
	 temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio / leaf_agents_sum;

	 unbalance_ratio = temp_value_for_unbalance_ratio;
	 */

	/**
	 * A simple version of measuring load balance
	 */
	std::vector<int> agents_counts_in_tree;

	leaf_agents_sum = 0;
	unbalance_ratio = 0;

	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		int agents_size = one_leaf->agent_buffer.size();
		agents_counts_in_tree.push_back(agents_size);
		leaf_agents_sum += agents_size;
		one_leaf = one_leaf->next;
	}

	//there is so few agents in the tree, so no need to balance anything
	if (leaf_agents_sum <= 10)
		return;

	double average_counts = leaf_agents_sum / leaf_counts;

//	std::cout << "average_counts:" << average_counts << std::endl;
//	std::cout << "leaf_agents_sum:" << leaf_agents_sum << std::endl;
//	std::cout << "leaf_counts:" << leaf_counts << std::endl;
//	std::cout << "unbalance_ratio:" << unbalance_ratio << std::endl;
//	std::cout << "total load cost:" << total_cost << std::endl;
//	std::cout << "best load cost:" << best_cost << std::endl;
//	std::cout << "average_leaf_counts:" << average_counts << std::endl;
//	std::cout << "load ratio:" << total_cost / best_cost << std::endl;
//
//	std::cout << "===========================" << std::endl;

	//	//need to re-build the tree
	if (average_counts > 32 || average_counts < 1 || calculate_unbalance_ratio(agents_counts_in_tree, average_counts) > rebalance_load_balance_maximum) {
		rebalance_counts++;

		if (rebalance_counts >= rebalance_threshold) {
			//			std::cout << "1->2" << std::endl;
			rebalance_counts = 0;

//			for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
//				std::cout << (*it) << ",	";
//			}

//			display();
//			std::cout << "========================================" << std::endl;
			rebalance();
//			std::cout << "========================================" << std::endl;
//			display();
		}
	} else {
		//		std::cout << "1->0" << std::endl;
		rebalance_counts = 0;
	}

	//temp always call re-balance

	//	display();

	//	rebalance();
#endif
}

void sim_mob::SimRTree::rebalance() {
	static int rebalance_sim_counts = 0;
	rebalance_sim_counts++;
	std::cout << "rebalance:" << rebalance_sim_counts << std::endl;

#ifdef SIM_TREE_USE_REBALANCE
	//release memory
	releaseTreeMemory();

	//Build A Big Table
	memset(bigtable, 0, sizeof(bigtable));

	//set the agent location
	//use structure & loop
	for_each(Agent::all_agents.begin(), Agent::all_agents.end(), BigTableUpdate());

//	std::cout << std::endl;
//	for (int i = 0; i < 1000; i++) {
//		for (int j = 0; j < 1000; j++) {
//
//			if (SimRTree::bigtable[i][j] > 0)
//				std::cout << SimRTree::bigtable[i][j] << ",";
//		}
//	}
//	std::cout << std::endl;

	// 11 signals
//	int agent_size = Agent::all_agents.size() - 11;
//	int tree_height = 1;
//
//	//16 is temp setting
//	while (agent_size > 16) {
//		agent_size = agent_size / 2;
//		tree_height++;
//
////		std::cout << "tree_height:" << tree_height << ",agent_size:" << agent_size << std::endl;
//	}

	//build new tree & set root
	TreeNode* one_node = new TreeNode();
	one_node->bound.edges[0].first = SimRTree::network_minimum_x;
	one_node->bound.edges[0].second = SimRTree::network_maximum_x;
	one_node->bound.edges[1].first = SimRTree::network_minimum_y;
	one_node->bound.edges[1].second = SimRTree::network_maximum_y;
	one_node->item_id = 1;
	one_node->is_leaf = false;
	one_node->items.clear();
	one_node->father = NULL;

	//the division is hard coded inside
	RebuildSimTreeNodeFunctor()(0, 0, 1000, 1000, one_node, false);

	//	rebuildSimTree(one_node, 1, tree_height, 0, 0, 10000, 1);
	m_root = one_node;

	//xuyan:make sure it is done, after the tree is built.
	connectLeaf();

	//xuyan: for re-balance
	countLeaf();

//std::cout << "Agent::all_agents:" << Agent::all_agents.size() << std::endl;

	//re-insert all agents inside
	for (std::set<Entity*>::iterator itr = Agent::all_agents.begin(); itr != Agent::all_agents.end(); itr++) {
		Agent* agent = dynamic_cast<Agent*>(*itr);
		if (agent && (agent)->isToBeRemoved() == false)
			insertAgent(agent);
	}

//	std::cout << "-----------------------------------------------" << std::endl;
//	display();
	//temp

//	double total_cost = 0;
//	double best_cost = 0;
//	int depth_cost = 20;
//
//	std::vector<int> agents_counts_in_tree;
//
//	leaf_agents_sum = 0;
//	unbalance_ratio = 0;
//
//	TreeLeaf* one_leaf = first_leaf;
//
//	while (one_leaf) {
//		int agents_size = one_leaf->agent_buffer.size();
//
//		total_cost += agents_size * (agents_size + depth_cost);
//
//		agents_counts_in_tree.push_back(agents_size);
//		leaf_agents_sum += agents_size;
//
//		one_leaf = one_leaf->next;
//	}
//
//	if (leaf_agents_sum <= 0)
//		return;
//
//	double average_counts = leaf_agents_sum / leaf_counts;
//	best_cost = average_counts * (average_counts + depth_cost) * leaf_counts;
//
//	//calculate un-balance ratio
//	double temp_value_for_unbalance_ratio = 0;
//	for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
//		temp_value_for_unbalance_ratio += pow((*it - average_counts), 2);
//	}
//
//	temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio * leaf_counts;
//	temp_value_for_unbalance_ratio = sqrt(temp_value_for_unbalance_ratio);
//	temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio / leaf_agents_sum;
//
//	unbalance_ratio = temp_value_for_unbalance_ratio;
//
//	std::cout << "After-------------------------------" << std::endl;
//
//	for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
//		std::cout << (*it) << ",	";
//	}
//	std::cout << std::endl;

//use re-balance
#endif

}
