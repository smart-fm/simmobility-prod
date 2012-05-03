/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "soci.h"
#include "soci-postgresql.h"
#include "../algorithms/Configurations.hpp"
#include "../utils/FileOutputHelper.hpp"
#include "../utils/Automate_Weight_Calculator.hpp"
#include "../utils/Scenario.hpp"

#include <iostream>

using namespace std;
using namespace sim_mob_partitioning;

namespace sim_mob_partitioning {
class DBConnection
{
public:
	explicit DBConnection();

	/**
	 * Select RoadNetwork
	 */
	void loadInRoadNode(Configurations& config);
	void loadInRoadSection(Configurations& config);
	void loadInRoadWeight(Configurations& config);
	void loadInRoadSectionWeight(Configurations& config);

	/**
	 * Insert Partitioning
	 */
	int insertAndGetScenarioID(Scenario setting, Configurations& config);
	int getMaximumScenarioId();
	int getMaximumPartitionSolutionId();
	void insertNewPartitionSolutionId(int scenario_id, int partition_solution_id);

	void insertSQL(std::string insert_sql_command);
private:
	soci::session sql_;
};
}
