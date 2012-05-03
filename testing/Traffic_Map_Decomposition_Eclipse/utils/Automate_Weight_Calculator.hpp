/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"
#include "FileOutputHelper.hpp"

using namespace std;

namespace sim_mob_partitioning {
	class Automate_Weight_Calculator
	{
	public:
		void static generate_node_weight(std::string node_weight_file, std::string node_file, std::string section_file);
		void static generate_section_weight(std::string section_weight_file, std::string node_file, std::string section_file);
	};
}
