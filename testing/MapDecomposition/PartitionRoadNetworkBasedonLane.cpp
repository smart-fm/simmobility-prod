/*
 * partitioning.cpp
 * Input:
 * [1]Partition Size
 * [2]Output Format (0: Console, 1:File, 2:Database)
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#include "metis.h"
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <string>

#include "DBConnection.hpp"
#include "RoadNode.hpp"
#include "RoadSection.hpp"
#include "METISHelper.hpp"
#include "FileOutputHelper.hpp"
#include "Configuration.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

vector<partitioning::RoadNode> all_nodes;
vector<partitioning::RoadSection> all_sections;

void load_in_configuration();
void build_node_connection();
partitioning::RoadNode* getRoadNodeById(int node_id);
void partition_output(idx_t node_partition[]);

int main(int argc, char* argv[]) {

	load_in_configuration();
	partitioning::DBConnection connection;

	all_nodes = connection.getAllRoadNode();
	all_sections = connection.getAllRoadSection();

	cout << "Node Size:" << all_nodes.size() << endl;
	cout << "Section Size:" << all_sections.size() << endl;

	build_node_connection();

	//prepare the inputs of METIS
	idx_t node_size = all_nodes.size();
	idx_t link_size = all_sections.size();
	idx_t node_array[node_size + 1];
	idx_t link_array[link_size * 2];
	idx_t node_weight[node_size];
	idx_t node_partition[node_size];

	partitioning::METISHelper helper;
	helper.all_nodes = &all_nodes;
	helper.all_sections = &all_sections;

	helper.getMETISParameters(node_array, link_array, node_weight, all_nodes.size(), all_sections.size());

	//Call METIS
	idx_t options[METIS_NOPTIONS];
	//options[0] = 0;
	METIS_SetDefaultOptions(options);

	idx_t number_cons = 1;
	idx_t cost = 0;

	//default 2 parts
	idx_t target_parts = partitioning::Configuration::instance().partition_size;

	int result = METIS_PartGraphRecursive(&node_size, &number_cons, node_array, link_array, node_weight, NULL, NULL,
			&target_parts, NULL, NULL, options, &cost, node_partition);

	if (result != METIS_OK) {
		std::cout << "Error at METIS_PartGraphRecursive;" << std::endl;
		return -1;
	}

	partition_output(node_partition);
	std::cout << "END" << std::endl;

	return 1;
}

void load_in_configuration() {
	std::string buffer_line;
	ifstream infile;
	infile.open("test_configuration.txt");

	partitioning::Configuration config = partitioning::Configuration::instance();

	while (!infile.eof()) {
		getline(infile, buffer_line); // Saves the line in STRING.

		if (boost::starts_with(buffer_line, "#"))
			continue;

		if (boost::starts_with(buffer_line, "Nodes")) {
			unsigned int start_location = buffer_line.find(":");
			if (start_location != string::npos) {
				partitioning::Configuration::instance().getNodeSQL = buffer_line.substr(start_location + 1);
			} else {
				partitioning::Configuration::instance().getNodeSQL = "get_node()";
			}
		} else if (boost::starts_with(buffer_line, "Sections")) {
			unsigned int start_location = buffer_line.find(":");
			if (start_location != string::npos) {
				partitioning::Configuration::instance().getSectionSQL = buffer_line.substr(start_location + 1);
			} else {
				partitioning::Configuration::instance().getSectionSQL = "get_section()";
			}
		} else if (boost::starts_with(buffer_line, "Partition_Size")) {
			unsigned int start_location = buffer_line.find(":");
			if (start_location != string::npos) {
				partitioning::Configuration::instance().partition_size = boost::lexical_cast<int>(buffer_line.substr(
						start_location + 1));
			} else {
				partitioning::Configuration::instance().partition_size = 2;
			}

			//cout << "partitioning::Configuration::instance().partition_size:" << partitioning::Configuration::instance().partition_size << endl;
		} else if (boost::starts_with(buffer_line, "Cut_Lane_Min_Length")) {
			unsigned int start_location = buffer_line.find(":");
			if (start_location != string::npos) {
				partitioning::Configuration::instance().cut_lane_minimum_size = boost::lexical_cast<double>(
						buffer_line.substr(start_location + 1));
			} else {
				partitioning::Configuration::instance().cut_lane_minimum_size = 1000;
			}
		} else if (boost::starts_with(buffer_line, "Cut_Lane_Can_Curved")) {
			unsigned int start_location = buffer_line.find(":");
			if (start_location != string::npos) {
				partitioning::Configuration::instance().cut_lane_can_curve = buffer_line.substr(start_location + 1);
			} else {
				partitioning::Configuration::instance().cut_lane_can_curve = "YES";
			}
		} else if (boost::starts_with(buffer_line, "Output_Format")) {
			unsigned int start_location = buffer_line.find(":");
			if (start_location != string::npos) {
				partitioning::Configuration::instance().output_format = boost::lexical_cast<int>(buffer_line.substr(
						start_location + 1));
			} else {
				partitioning::Configuration::instance().output_format = 1;
			}
		}
	} // End of Reading File
	infile.close();
}

void build_node_connection() {
	vector<partitioning::RoadSection>::iterator itr = all_sections.begin();
	for (; itr != all_sections.end(); itr++) {
		int start_node = (*itr).from_node_id;
		int end_node = (*itr).to_node_id;

		(*itr).from_node = getRoadNodeById(start_node);
		(*itr).to_node = getRoadNodeById(end_node);
	}
}

partitioning::RoadNode* getRoadNodeById(int node_id) {
	vector<partitioning::RoadNode>::iterator itr = all_nodes.begin();
	for (; itr != all_nodes.end(); itr++) {
		if (node_id == (*itr).node_id) {
			return &(*itr);
		}
	}

	return NULL;
}

void partition_output(idx_t node_partition[]) {
	//analysis result and output
	//output to console
	partitioning::Configuration config = partitioning::Configuration::instance();

	if (config.output_format == 1) {

		std::cout << "Partition Result" << std::endl;
		std::cout << "-----------------------------------------------" << std::endl;

		vector<partitioning::RoadSection>::iterator itr = all_sections.begin();
		for (; itr != all_sections.end(); itr++) {
			//输出结果
			int from_node_index = (*itr).from_node->node_index;
			int to_node_index = (*itr).to_node->node_index;

			if (node_partition[from_node_index] == node_partition[to_node_index]) {
				std::stringstream ss;
				ss << (*itr).section_id << "	,";
				ss << (*itr).from_node->x_pos << "	,";
				ss << (*itr).from_node->y_pos << "	,";
				ss << (*itr).to_node->x_pos << "	,";
				ss << (*itr).to_node->y_pos << "	,";
				ss << node_partition[from_node_index];

				std::cout << ss.str() << std::endl;
			} else {
				std::stringstream ss;
				ss << (*itr).section_id << "	,";
				ss << (*itr).from_node->x_pos << "	,";
				ss << (*itr).from_node->y_pos << "	,";
				ss << (*itr).to_node->x_pos << "	,";
				ss << (*itr).to_node->y_pos << "	,";
				ss << -1;

				std::cout << ss.str() << std::endl;
			}
		}
	}
	//file format
	else if (config.output_format == 2) {
		partitioning::FileOutputHelper output;
		output.openFile("partition_result.txt");

		vector<partitioning::RoadSection>::iterator itr = all_sections.begin();
		for (; itr != all_sections.end(); itr++) {
			//输出结果
			int from_node_index = (*itr).from_node->node_index;
			int to_node_index = (*itr).to_node->node_index;

			if (node_partition[from_node_index] == node_partition[to_node_index]) {
				std::stringstream ss;
				ss << (*itr).section_id << ",";
				ss << (*itr).from_node->x_pos << ",";
				ss << (*itr).from_node->y_pos << ",";
				ss << (*itr).to_node->x_pos << ",";
				ss << (*itr).to_node->y_pos << ",";
				ss << node_partition[from_node_index];

				output.output_to_file(ss.str());
			} else {
				std::stringstream ss;
				ss << (*itr).section_id << ",";
				ss << (*itr).from_node->x_pos << ",";
				ss << (*itr).from_node->y_pos << ",";
				ss << (*itr).to_node->x_pos << ",";
				ss << (*itr).to_node->y_pos << ",";
				ss << -1;

				output.output_to_file(ss.str());
			}
		}
		output.closeFile();
	}
	//To database
	else if (config.output_format == 3) {

	}
}

