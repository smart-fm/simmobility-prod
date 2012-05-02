/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "../all_includes.hpp"
#include "../algorithms/Configurations.hpp"
#include "DatabaseConfiguration.hpp"
#include "../database/DBConnection.hpp"
#include "../utils/Scenario.hpp"

namespace sim_mob_partitioning {
class ChangeDatabasePartition
{
public:
	void push_partitions_to_database(DBConnection& connection, Configurations& config);

private:
	void load_in_database_configuration(Configurations& config);
	void push_to_DB(DBConnection& connection, Configurations& config, int partition_solution_id);

public:
	Scenario the_scenario;
};
}
