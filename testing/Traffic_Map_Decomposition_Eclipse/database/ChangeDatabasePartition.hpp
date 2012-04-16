/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"
#include "../algorithms/Configurations.hpp"
#include "DatabaseConfiguration.hpp"

namespace sim_mob_partitioning {
class ChangeDatabasePartition
{
public:
	void push_partitions_to_database(Configurations& config);

private:
	void load_in_database_configuration();
	void generate_requried_data();
	void push_to_DB();
};
}
