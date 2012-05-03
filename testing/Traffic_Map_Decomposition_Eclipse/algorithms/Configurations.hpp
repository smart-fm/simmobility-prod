/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"

namespace sim_mob_partitioning {
class Configurations
{
public:
	std::string node_file;
	std::string flow_file;
	std::string infor_file;
	std::string output_folder;
	std::string database_config_file;

	//nparts: size of partitioning, should > 2
	int nparts;

	//0: fast
	//1: middle
	//2: slow
	int execute_speed;

	//the ratio of heaviest load over average load, should  > 1
	double load_imbalance;

	bool contiguous;

	std::string node_sql;
	std::string section_sql;

	std::string network_node_file;
	std::string network_section_file;

	std::string partition_file;
};
}
