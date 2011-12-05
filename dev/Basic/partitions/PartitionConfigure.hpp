/*
 * PartitionConfigure.hpp
 * Target:
 * 1. Have some attributes for configuration
 */

#pragma once

#include <string>

namespace sim_mob
{
class PartitionConfigure
{
public:
	int partition_size; 		//from the MPI ENV
	int partition_id; 			//from the MPI ENV
	int partition_solution_id; 	//To load in the partition solution in database

	double boundary_length;
	double boundary_width; 		//define how large is the boundary area

	int maximum_agent_id;		//each partition has the same range size
	bool adaptive_load_balance; //whether to start adaptive load balance
	bool measurem_performance; //whether to measure load balance
	std::string measure_output_file; //measurement file

};
}
