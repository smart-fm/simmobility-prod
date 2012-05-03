/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"
#include "Configurations.hpp"

namespace sim_mob_partitioning
{
class ContiguousProcessor
{
public:
	/**
	 *the interface between main function
	 */
	void do_contiguous(std::string node_file, std::string section_file, std::string partition_file);
};
}
