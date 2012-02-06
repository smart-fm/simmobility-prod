/*
 * loader.cpp
 *
 *  Created on: 03-Feb-2012
 *      Author: xuyan
 */

#include "DBConnection.hpp"
#include <string>

namespace partitioning {
DBConnection::DBConnection() :
	sql_(soci::postgresql, "host=172.18.127.157 port=5432 dbname=SimMobility_DB user=postgres password=S!Mm0bility") {
}

vector<RoadNode> DBConnection::getAllRoadNode() {
	vector<RoadNode> all_nodes;

	std::string sql_str = "select * from ";
	sql_str += partitioning::Configuration::instance().getNodeSQL;

	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);
	int index = 0;

	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); it++) {
		soci::row const& row = *it;

		RoadNode one_node;
		one_node.node_id = row.get<int> (0);
		one_node.node_index = index;
		one_node.x_pos = row.get<double> (1);
		one_node.y_pos = row.get<double> (2);
		one_node.node_weight = 0;

		all_nodes.push_back(one_node);
		index ++;
	}

	return all_nodes;
}

vector<RoadSection> DBConnection::getAllRoadSection() {
	vector<RoadSection> all_sections;

	std::string sql_str = "select * from ";
	sql_str += partitioning::Configuration::instance().getSectionSQL;

	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);

	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); it++) {
		soci::row const& row = *it;

		RoadSection one_section;
		one_section.section_id = row.get<int> (0);
		one_section.lane_size = row.get<int> (2);
		one_section.from_node_id = row.get<int> (5);
		one_section.to_node_id = row.get<int> (6);
		one_section.section_length = row.get<double> (7);

		all_sections.push_back(one_section);
	}

	return all_sections;
}

}
