/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"
#include "Configurations.hpp"

namespace sim_mob_partitioning
{
class MapDecompositionWithoutInformationFlow
{
public:
	/**
	 *the interface between main function
	 */
	void do_map_partitioning(Configurations& config);

private:
	/**
	 * Do map decomposition step by step
	 */
	void generate_required_parameters_without_infor(Configurations& config);
	void do_map_decompose_without_infor(Configurations& config);
	void result_analysis_without_infor(Configurations& config);

};
}
