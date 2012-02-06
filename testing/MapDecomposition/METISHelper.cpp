/*
 * METISHelper.cpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#include "METISHelper.hpp"

namespace partitioning {

//int METISHelper::getNodeIndexByID(int node_id) {
//	vector<partitioning::RoadNode>::iterator itr = (*all_nodes).begin();
//	int index = 0;
//	for (; itr != (*all_nodes).end(); itr++) {
//		if ((*itr).node_id == node_id)
//			break;
//
//		index++;
//	}
//
//	return index;
//}

void METISHelper::getMETISParameters(idx_t* node_array, idx_t* link_array, idx_t* node_weight, int node_size,
		int link_size) {
	for (int i = 0; i < node_size; i++)
		node_weight[i] = 0;

	//calculate node weight
	vector<partitioning::RoadSection>::iterator itr = (*all_sections).begin();
	for (; itr != (*all_sections).end(); itr++) {
		int from_index = (*itr).from_node->node_index;
		int to_index = (*itr).to_node->node_index;
		double weight = ((*itr).section_length) * ((*itr).lane_size) / 2;

		node_weight[from_index] += weight;
		node_weight[to_index] += weight;
	}

	vector<RoadNode> adj_nodes[node_size];

	//Calculate the neighbour nodes of each node
	itr = (*all_sections).begin();
	for (; itr != (*all_sections).end(); itr++) {

		//add to node to from vector
		int from_index = (*itr).from_node->node_index;
		vector<RoadNode>::iterator itr_node = adj_nodes[from_index].begin();

		bool hasInsert = false;
		for (; itr_node != adj_nodes[from_index].end(); itr_node++) {
			if ((*itr_node).node_id == (*itr).to_node_id) {
				hasInsert = true;
				break;
			}
		}

		if (hasInsert == false) {
			adj_nodes[from_index].push_back(*((*itr).to_node));
		}

		//add from node to to vector
		int to_index = (*itr).to_node->node_index;
		itr_node = adj_nodes[to_index].begin();

		hasInsert = false;
		for (; itr_node != adj_nodes[to_index].end(); itr_node++) {
			if ((*itr_node).node_id == (*itr).from_node_id) {
				hasInsert = true;
				break;
			}
		}

		if (hasInsert == false) {
			adj_nodes[to_index].push_back(*((*itr).from_node));
		}
	}

	//Calculate weights and METIS vectors
	int start_count = 0;
	node_array[0] = 0;

	for (int i = 1; i <= node_size; i++) {
		start_count += adj_nodes[i - 1].size();
		node_array[i] = start_count;
	}

//	if (start_count != link_size * 2 + 1) {
//		cout << "Error, (node-adj vector size) should be 2 * (link size)" << start_count << endl;
//		exit(0);
//	}

	for (int i = 0; i < node_size; i++) {
		vector<RoadNode>::iterator itr = adj_nodes[i].begin();
		int start_index = node_array[i];

		for (; itr != adj_nodes[i].end(); itr++) {
			link_array[start_index] = (*itr).node_index;
			start_index++;
		}
	}
}

}
