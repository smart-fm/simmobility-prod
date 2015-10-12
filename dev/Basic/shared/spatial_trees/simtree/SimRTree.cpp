//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "SimRTree.hpp"

#include <string>
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"
#include "geospatial/network/BusStop.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"

using namespace sim_mob;

namespace {

Point WayPointToLocation(const WayPoint& wp) {
	if (wp.type == WayPoint::NODE) {
		return *(wp.node->getLocation());
	}
	if (wp.type == WayPoint::BUS_STOP) {
		return wp.busStop->getStopLocation();
	}

	//TODO: Exception?
	return Point(0, 0);
}

} //End unnamed namespace

namespace {
//
//This class is used internally for counting the number of function calls
//
class Profiling_Counting_SIM {
public:
	static long counting;
	static std::vector<long> count_buff;
};

long Profiling_Counting_SIM::counting = 0;
std::vector<long> Profiling_Counting_SIM::count_buff;
}

long SimRTree::network_minimum_x = 0;
long SimRTree::network_minimum_y = 0;
long SimRTree::network_maximum_x = 0;
long SimRTree::network_maximum_y = 0;

int SimRTree::division_x_unit_ = 0;
int SimRTree::division_y_unit_ = 0;

double SimRTree::maximum_Rectanle_Weight = 8;
double SimRTree::minimum_Rectanle_Border_Length = 1;

int SimRTree::bigtable[DIVIDE_NETWORK_X_INTO_CELLS][DIVIDE_NETWORK_Y_INTO_CELLS];

struct BigTableUpdate: std::unary_function<const Entity *, void> {
	void operator()(const Entity * item) {
		const Person* person = dynamic_cast<const Person*>(item);
		if (person && person->xPos > 0 && person->yPos > 0) {

			int x = (person->xPos - SimRTree::network_minimum_x) / SimRTree::division_x_unit_;
			int y = (person->yPos - SimRTree::network_minimum_y) / SimRTree::division_y_unit_;

			if (x >= DIVIDE_NETWORK_X_INTO_CELLS || x < 0 || y >= DIVIDE_NETWORK_Y_INTO_CELLS || y < 0)
				return;

			SimRTree::bigtable[y][x]++;
		}
	}
};

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
		if (box.encloses(leaf->bound)) {

#ifdef MEAUSURE_COUNTS
			static long rangeQuery_saving_counts = 0;
			rangeQuery_saving_counts++;

			if(rangeQuery_saving_counts % 1000 == 0)
			PrintOut( "rangeQuery_saving_counts:" << rangeQuery_saving_counts << std::endl );
#endif

			array.insert(array.end(), leaf->agent_buffer.begin(), leaf->agent_buffer.end());
		}
		else {
			for (std::vector<Agent*>::iterator itr = leaf->agent_buffer.begin(); itr != leaf->agent_buffer.end(); itr++) {
				Agent * one_agent = (*itr);

				if (box.enclose_point(one_agent->xPos, one_agent->yPos)) {
					array.push_back(one_agent);
				}
			}
		}
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
		return m_bound.overlaps(leaf->bound);
	}
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
		if (accept(leaf)) {
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
			TreeLeaf * leaf = static_cast<TreeLeaf *>(item);
			VisitFunctor(accept, visitor)(leaf);
		}
		else {
			TreeNode * node = static_cast<TreeNode *>(item);

			if (visitor.ContinueVisiting && accept(node))
				for_each(node->items.begin(), node->items.end(), *this);
		}
	}
};

/**
 *Functor that used to rebuild the tree
 */
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
				}
				else {
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
				}
				else {
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

			if (difference == 0) {
				best_difference = 0;
				best_cut = middle_cut;
				break;
			}
			else if (difference < 0) {
				from_values_y_ = middle_cut;

				if (best_difference > -1 * difference) {
					best_cut = middle_cut;
					best_difference = -1 * difference;
				}
			}
			else if (difference > 0) {
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

			if (difference == 0) {
				best_difference = 0;
				best_cut = middle_cut;
				break;
			}
			else if (difference < 0) {
				from_values_x_ = middle_cut;

				if (best_difference > -1 * difference) {
					best_cut = middle_cut;
					best_difference = -1 * difference;
				}
			}
			else if (difference > 0) {
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

/**
 * New Version Rebuild
 */
struct RebuildSimTreeNodeFunctor {
	explicit RebuildSimTreeNodeFunctor() {
	}

	void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, TreeNode * parent_, bool must_stop) {
		//too small to cut, so make a leaf

		if ((must_stop == true) || (ShouldStopAsALeaf()(from_values_x, from_values_y, to_values_x, to_values_y) == true)) {
			TreeLeaf* one_leaf = new TreeLeaf();

			one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
			one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
			one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
			one_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
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

		//try to cut x
		int where_to_divide_v = -1;
		double cost_to_divide_v = std::numeric_limits<double>::max();
		DivideVertical()(from_values_x, from_values_y, to_values_x, to_values_y, &where_to_divide_v, &cost_to_divide_v);

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
			}
			else {
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
			}
			else {
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

struct GetRightestLeaf {
	explicit GetRightestLeaf() {
	}

	TreeLeaf* operator()(TreeItem* item) {
		if (!item->is_leaf) {
			TreeNode* node = static_cast<TreeNode*>(item);
			int child_size = node->items.size();
			assert(child_size > 0);

			return GetRightestLeaf()(node->items[child_size - 1]);
		}
		else {
			TreeLeaf* leaf = static_cast<TreeLeaf*>(item);
			return leaf;
		}
	}
};

struct GetLeftestLeaf {
	explicit GetLeftestLeaf() {
	}

	TreeLeaf* operator()(TreeItem* item) {
		if (!item->is_leaf) {
			TreeNode* node = static_cast<TreeNode*>(item);
			int child_size = node->items.size();
			assert(child_size > 0);

			return GetLeftestLeaf()(node->items[0]);
		}
		else {
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
			TreeLeaf* right_left = GetRightestLeaf()(one_node->items[i - 1]);
			TreeLeaf* left_left = GetLeftestLeaf()(one_node->items[i]);

			assert(right_left != nullptr);
			assert(left_left != nullptr);

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

void sim_mob::SimRTree::buildTreeStructure(const std::string& filename) {
	std::ifstream fin(filename.c_str());
	if (!fin) {
		std::cerr << "Cannot open Density Pattern file: " << filename << " " << std::endl;
		return;
	}

	int parent_id, self_id; //IDs used to build the structure
	int is_leaf; //1:YES; 0:NO
	long start_x, start_y, end_x, end_y;
	bool first_line = true;

	while (fin) {
		fin >> parent_id >> self_id >> start_x >> start_y >> end_x >> end_y >> is_leaf;
		if (!fin.good())
			continue; // skip newlines, etc.

		if (first_line) {
			first_line = false;

			SimRTree::network_minimum_x = start_x;
			SimRTree::network_minimum_y = start_y;
			SimRTree::network_maximum_x = end_x;
			SimRTree::network_maximum_y = end_y;

			SimRTree::division_x_unit_ = (int) (ceil((end_x - start_x) * 1.0 / DIVIDE_NETWORK_X_INTO_CELLS));
			SimRTree::division_y_unit_ = (int) (ceil((end_y - start_y) * 1.0 / DIVIDE_NETWORK_Y_INTO_CELLS));
		}

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
				m_root->father = nullptr;
			}
			else {
				TreeNode* one_node = new TreeNode();
				one_node->is_leaf = false;
				one_node->item_id = self_id;

				one_node->items.reserve(2);
				one_node->bound = box;

				addNodeToFather(parent_id, m_root, one_node);
			}
		}
		//if it is a leaf
		else {
			TreeLeaf* one_leaf = new TreeLeaf();
			one_leaf->next = nullptr;

			one_leaf->is_leaf = true;
			one_leaf->item_id = self_id;

			one_leaf->bound = box;

			addLeafToFather(parent_id, m_root, one_leaf);
		}
	}

	fin.close();

	//make sure it is done, after the tree is built.
	connectLeaf();

	//for re-balance
	countLeaf();
}

void sim_mob::SimRTree::buildTreeStructure() {
	//find the border

	int min_x = INT_MAX;
	int min_y = INT_MAX;
	int max_x = INT_MIN;
	int max_y = INT_MIN;

	/*
	std::vector<Node*>::const_iterator itr = ConfigManager::GetInstance().FullConfig().getNetwork().nodes.begin();
	for (; itr != ConfigManager::GetInstance().FullConfig().getNetwork().nodes.end(); itr++) {
		if (min_x > (*itr)->location.getX()) {
			min_x = (*itr)->location.getX();
		}
		if (min_y > (*itr)->location.getY()) {
			min_y = (*itr)->location.getY();
		}

		if (max_x < (*itr)->location.getX()) {
			max_x = (*itr)->location.getX();
		}
		if (max_y < (*itr)->location.getY()) {
			max_y = (*itr)->location.getY();
		}
	}*/

	min_x -= division_x_unit_;
	min_y -= division_y_unit_;
	max_x += division_x_unit_;
	max_y += division_y_unit_;

	PrintOut( "SimTree Manages The 2-D Area: (" << min_x << "," << min_y << "), (" << max_x << "," << max_y << ")" << std::endl);

	//build the root node
	SimRTree::BoundingBox root_box;
	root_box.edges[0].first = min_x;
	root_box.edges[0].second = max_x;
	root_box.edges[1].first = min_y;
	root_box.edges[1].second = max_y;

	m_root = new TreeNode();
	m_root->is_leaf = false;
	m_root->item_id = 1;

	m_root->items.reserve(2);
	m_root->bound = root_box;
	m_root->father = nullptr;

	//set boundary
	SimRTree::network_minimum_x = min_x;
	SimRTree::network_minimum_y = min_y;
	SimRTree::network_maximum_x = max_x;
	SimRTree::network_maximum_y = max_y;

	SimRTree::division_x_unit_ = (int) (ceil((max_x - min_x) * 1.0 / DIVIDE_NETWORK_X_INTO_CELLS));
	SimRTree::division_y_unit_ = (int) (ceil((max_y - min_y) * 1.0 / DIVIDE_NETWORK_Y_INTO_CELLS));

	//build two leaf nodes
	SimRTree::BoundingBox leaf_box_l;
	leaf_box_l.edges[0].first = min_x;
	leaf_box_l.edges[0].second = min_x + (max_x - min_x) / 2;
	leaf_box_l.edges[1].first = min_y;
	leaf_box_l.edges[1].second = max_y;

	TreeLeaf* left_leaf = new TreeLeaf();
	left_leaf->next = nullptr;
	left_leaf->is_leaf = true;
	left_leaf->item_id = 2;
	left_leaf->bound = leaf_box_l;
	addLeafToFather(1, m_root, left_leaf);

	SimRTree::BoundingBox leaf_box_r;
	leaf_box_r.edges[0].first = min_x + (max_x - min_x) / 2 + 1;
	leaf_box_r.edges[0].second = max_x;
	leaf_box_r.edges[1].first = min_y;
	leaf_box_r.edges[1].second = max_y;

	TreeLeaf* right_leaf = new TreeLeaf();
	right_leaf->next = nullptr;
	right_leaf->is_leaf = true;
	right_leaf->item_id = 3;
	right_leaf->bound = leaf_box_r;
	addLeafToFather(1, m_root, right_leaf);

	//
	//make sure it is done, after the tree is built.
	connectLeaf();

	//for re-balance
	countLeaf();
}

void sim_mob::SimRTree::connectLeaf() {
	ConnectLeafs()(m_root);
	first_leaf = GetLeftestLeaf()(m_root);
}

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
SimRTree::BoundingBox sim_mob::SimRTree::locationBoundingBox(Agent * agent) {
	SimRTree::BoundingBox box;
	box.edges[0].first = box.edges[0].second = agent->xPos.get(); //->xPos_Sim;
	box.edges[1].first = box.edges[1].second = agent->yPos.get(); //yPos_Sim;

	return box;
}

SimRTree::BoundingBox sim_mob::SimRTree::ODBoundingBox(Agent * agent) {
	//Retrieve the location (Point) of the OriginWayPoint.
	//TODO: This only occurs here, so I'm removing it from the WayPoint class.
	//      We need a better way to do this; it really has nothing to do with WayPoints ~Seth.
	Point originLoc = WayPointToLocation(agent->originNode);
	Point densiLoc = WayPointToLocation(agent->destNode);

	SimRTree::BoundingBox box;
	box.edges[0].first = box.edges[0].second = originLoc.getX();
	box.edges[1].first = box.edges[1].second = originLoc.getY();

	return box;
}

void sim_mob::SimRTree::insertAgent(Agent* agent, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap) {
	SimRTree::BoundingBox agent_box = locationBoundingBox(agent);
	insertAgentEncloseBox(agent, agent_box, (m_root), connectorMap);
}

void sim_mob::SimRTree::insertAgentBasedOnOD(Agent* agent, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap) {
	SimRTree::BoundingBox agent_box = ODBoundingBox(agent);
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

			(one_leaf->agent_buffer).push_back(agent);

			connectorMap[agent] = one_leaf;
			//agent->connector_to_Sim_Tree = one_leaf;

			return;
		}
		else {
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

			PrintOut( "Count:" << Profiling_Counting_SIM::count_buff.size() << ", Sum Cost:" << sum << std::endl);
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
	PrintOut( "rangeQuery_counts:" << rangeQuery_counts << std::endl );
#endif

	//Find the start node for Query
	TreeItem* from_node = item;
	while (from_node->bound.encloses(box) == false) {
		from_node = from_node->father;

#ifdef MEAUSURE_COUNTS
		static long rangeQuery_enclose_counts = 0;
		rangeQuery_enclose_counts++;

		if(rangeQuery_enclose_counts % 10000 == 0)
		PrintOut( "rangeQuery_enclose_counts:" << rangeQuery_enclose_counts << std::endl);
#endif

		//sometimes users are query outside of map boundary
		if (from_node == nullptr) {
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

			PrintOut( "Count:" << Profiling_Counting_SIM::count_buff.size() << ", Sum Cost:" << sum << std::endl);
		}
#endif
	}

	return result;
}

/**
 *
 */
void sim_mob::SimRTree::updateAllInternalAgents(std::map<const sim_mob::Agent*, TreeItem*>& connectorMap, const std::set<sim_mob::Agent*>& removedAgentPointers) {
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		SimRTree::BoundingBox box = one_leaf->bound;

		unsigned int offset = 0;
		while (offset < one_leaf->agent_buffer.size()) {
			//Case 1: the agent should be removed from the Sim-R Tree
			Agent * one_agent = (one_leaf->agent_buffer[offset]);

			if (removedAgentPointers.find(one_agent) != removedAgentPointers.end()) {
				std::map<const sim_mob::Agent*, TreeItem*>::iterator it = connectorMap.find(one_agent);
				if (it != connectorMap.end()) {
					connectorMap.erase(it);
				}

				one_leaf->agent_buffer.erase(one_leaf->agent_buffer.begin() + (offset));
				continue;
			}

			if (one_agent->xPos.get() <= 0 || one_agent->yPos.get() <= 0) {
				offset++;
				continue;
			}

			SimRTree::BoundingBox agent_box = locationBoundingBox(one_agent);

			//Case 2: the agent should be in the same box
			//Case 2: It should be the most happen case.
			if (box.encloses(agent_box)) {
				offset++;
				continue;
			}

			//Case 3: The agent should be moved to a different box
			//Case 3: If the agent was insert into a box which is not scanned, the agent will be checked again. It will cost some CPU. But, since Case 3 is not much, the cost should be accpatable.
			//Case 3: If the cost is heavy, need to define another variable in agent to skip re-check.
			else {
				Agent * one_agent = one_leaf->agent_buffer[offset];

				//one_agent->connector_to_Sim_Tree = nullptrptr;
				std::map<const sim_mob::Agent*, TreeItem*>::iterator it = connectorMap.find(one_agent);
				if (it != connectorMap.end()) {
					connectorMap.erase(it);
				}

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
	PrintOut( "#########################################" << std::endl);
	display(m_root, 1);
}

void sim_mob::SimRTree::display(TreeItem* item, int level) {
	if (!item->is_leaf) {
		PrintOut(
				"Node level:" << level << "(" << item->bound.edges[0].first << "," << item->bound.edges[1].first << "," << item->bound.edges[0].second << "," << item->bound.edges[1].second << ")" << std::endl);

		TreeNode* one_node = static_cast<TreeNode*>(item);
		int child_size = one_node->items.size();
		for (int i = 0; i < child_size; i++) {
			display((one_node->items[i]), level + 1);
		}
	}
	else {
		TreeLeaf* one_leaf = static_cast<TreeLeaf*>(item);
		PrintOut(
				"Leaf level:" << level << "(" << item->bound.edges[0].first << "," << item->bound.edges[1].first << "," << item->bound.edges[0].second << "," << item->bound.edges[1].second << ")" << "child size" << one_leaf->agent_buffer.size() << std::endl);
	}
}

/**
 * Debug
 */
void sim_mob::SimRTree::checkLeaf() {
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		int agents_size = one_leaf->agent_buffer.size();

		for (int i = 0; i < agents_size; i++) {
			if (one_leaf->agent_buffer[i]->isToBeRemoved()) {
				PrintOut( "one_leaf->agent_buffer[i]->isToBeRemoved():" << one_leaf->agent_buffer[i]->getId() << std::endl);
			}
		}

		one_leaf = one_leaf->next;
	}
}

/*
 * The parameters are only related with re-balance
 */
void sim_mob::SimRTree::initRebalanceSettings() {
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

	//In the current code, rebalance_threshold and rebalance_load_balance_maximum are not used.
	//WHY?
	//Users do not need to know what does these two parameters mean.
	//But they are not removed. So, if someone read the paper and want to have a try on different auto balance setting, they become useful

	//from minutes to simulation steps
	checking_frequency = checking_frequency * 60 / ConfigManager::GetInstance().FullConfig().baseGranSecond();
}

//#endif

double inline calculate_unbalance_ratio(std::vector<int> agents_counts_in_tree, double average_counts) {
	int size = agents_counts_in_tree.size();
	if (size > 0) {
		double sum_diff = 0.0;

		for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
			sum_diff += abs(*it - average_counts) / average_counts;
		}

		return sum_diff / size;
	}
	else {
		return 0.0;
	}
}

/**
 * Load Balance
 */
void sim_mob::SimRTree::measureUnbalance(int time_step, std::map<const sim_mob::Agent*, TreeItem*>& agent_connector_map) {
	//did not rebalance in the first tick
	if (time_step <= 1) {
		return;
	}

	if (time_step % checking_frequency != 0) {
		return;
	}

	rebalance(agent_connector_map);
}

void sim_mob::SimRTree::rebalance(std::map<const sim_mob::Agent*, TreeItem*>& agent_connector_map) {
	releaseTreeMemory();

	//Build A Big Table
	memset(bigtable, 0, sizeof(bigtable));

	//set the agent location
	//use structure & loop
	for_each(Agent::all_agents.begin(), Agent::all_agents.end(), BigTableUpdate());

	//build new tree & set root
	TreeNode* one_node = new TreeNode();
	one_node->bound.edges[0].first = SimRTree::network_minimum_x;
	one_node->bound.edges[0].second = SimRTree::network_maximum_x;
	one_node->bound.edges[1].first = SimRTree::network_minimum_y;
	one_node->bound.edges[1].second = SimRTree::network_maximum_y;
	one_node->item_id = 1;
	one_node->is_leaf = false;
	one_node->items.clear();
	one_node->father = nullptr;

	//
	RebuildSimTreeNodeFunctor()(0, 0, DIVIDE_NETWORK_X_INTO_CELLS, DIVIDE_NETWORK_Y_INTO_CELLS, one_node, false);
	m_root = one_node;

	//make sure it is done, after the tree is built.
	connectLeaf();

	//for re-balance
	countLeaf();

	//re-insert all agents inside
	for (std::set<Entity*>::iterator itr = Agent::all_agents.begin(); itr != Agent::all_agents.end(); itr++) {
		Agent* agent = dynamic_cast<Agent*>(*itr);
		if (agent && (agent)->isToBeRemoved() == false) {
			insertAgent(agent, agent_connector_map);
		}
	}
}
