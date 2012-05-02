/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "ChangeDatabasePartition.hpp"

using namespace std;
using namespace sim_mob_partitioning;

void ChangeDatabasePartition::push_partitions_to_database(DBConnection& connection, Configurations& config)
{
	load_in_database_configuration(config);

	//start to insert scenario and partition solution id
	int scenario_id = connection.insertAndGetScenarioID(the_scenario, config);
	int partition_solution_id = connection.getMaximumPartitionSolutionId() + 1;
	connection.insertNewPartitionSolutionId(scenario_id, partition_solution_id);

//	std::cout << "scenario_id:" << scenario_id << std::endl;
//	std::cout << "partition_solution_id:" << partition_solution_id << std::endl;

	//start to insert all sections
	push_to_DB(connection, config, partition_solution_id);
}

void ChangeDatabasePartition::load_in_database_configuration(Configurations& config)
{
	std::string buffer_line;
	ifstream infile;
	infile.open(config.database_config_file.c_str());

	while (!infile.eof())
	{
		getline(infile, buffer_line); // Saves the line in STRING.

		if (boost::starts_with(buffer_line, "#"))
			continue;

		std::vector<std::string> strs;
		boost::split(strs, buffer_line, boost::is_any_of(":"));

		if (strs[0].compare("ROAD_NETWORK") == 0)
			the_scenario.road_network_name = strs[1];
		else if (strs[0].compare("DAY_IN_WEEK") == 0)
			the_scenario.day_in_week = strs[1];
		else if (strs[0].compare("FROM_TIME") == 0)
			the_scenario.from_time = strs[1];
		else if (strs[0].compare("TO_TIME") == 0)
			the_scenario.to_time = strs[1];
		else if (strs[0].compare("HOLIDAY") == 0)
			the_scenario.is_holiday = strs[1];
		else if (strs[0].compare("INCIDENT") == 0)
			the_scenario.has_incident = strs[1];
		else if (strs[0].compare("WEATHER") == 0)
			the_scenario.weather = strs[1];
	}

	infile.close();
}

/**
 * Source:
 * (1) node.weight
 * (2) hmetis.input.part.2
 * (3) sections.txt
 */
void ChangeDatabasePartition::push_to_DB(DBConnection& connection, Configurations& config, int partition_solution_id)
{
	//load in partition file
	map<std::string, int> node_partition;

	std::string buffer_line;
	std::string buffer_line_2;

	ifstream nodes_file, partition_file;
	nodes_file.open(config.node_file.c_str());
	partition_file.open(config.partition_file.c_str());

	while (!nodes_file.eof() && !partition_file.eof())
	{
		getline(nodes_file, buffer_line); // Saves the line in STRING.
		getline(partition_file, buffer_line_2); // Saves the line in STRING.

		std::vector<std::string> strs;
		boost::split(strs, buffer_line, boost::is_any_of(","));

		if(strs.size() < 2)
			continue;

		int partition_id = boost::lexical_cast<int>(buffer_line_2);
		node_partition[strs[0]] = partition_id;
	}

	nodes_file.close();
	partition_file.close();

	// load in section file
	ifstream section_file;
	section_file.open(config.network_section_file.c_str());

	while (!section_file.eof())
	{
		getline(section_file, buffer_line); // Saves the line in STRING.

		std::vector<std::string> strs;
		boost::split(strs, buffer_line, boost::is_any_of(","));

		if (strs.size() < 7)
			continue;

		map<std::string, int>::iterator itr_l = node_partition.find(strs[5]);
		if (itr_l == node_partition.end())
			continue;

		map<std::string, int>::iterator itr_r = node_partition.find(strs[6]);
		if (itr_r == node_partition.end())
			continue;

//		int from_node = boost::lexical_cast<int>(strs[5]);
//		int to_node = boost::lexical_cast<int>(strs[6]);

		//partition results counts from 0;
		//but, database counts from 1
		int from_node_partition = node_partition[strs[5]] + 1;
		int to_node_partition = node_partition[strs[6]] + 1;

		//one section in one partition
		if (from_node_partition == to_node_partition)
		{
			std::string sql_command = "INSERT INTO \"Partition_Section\"(\"Partition_Solution_ID\", \"Partition_ID\", \"Section_ID\") VALUES (";
			sql_command += boost::lexical_cast<std::string>(partition_solution_id) + ",";
			sql_command += boost::lexical_cast<std::string>(from_node_partition) + ",";
			sql_command += strs[0] + ")";

			connection.insertSQL(sql_command);
		}
		//one section is in boundary
		else
		{
			//currently, cut in the middle, in future, it will check whether there are road elements nearby
			double offset = boost::lexical_cast<double>(strs[7]) / 2.0;

			//upper boundary
			std::string
					sql_command =
							"INSERT INTO \"Partition_Boundary_Section\"(\"Partition_Solution_ID\", \"Partition_ID\", \"Boundary_Section_ID\",\"Section_Cut_Point_Offset\", \"Connect_Partition_ID\", \"In_Partition_Side\") VALUES ('";

			sql_command += boost::lexical_cast<std::string>(partition_solution_id) + "','";
			sql_command += boost::lexical_cast<std::string>(from_node_partition) + "','";
			sql_command += strs[0] + "','";
			sql_command += boost::lexical_cast<std::string>(offset) + "','";
			sql_command += boost::lexical_cast<std::string>(to_node_partition) + "',";
			sql_command += "'1' )";

			connection.insertSQL(sql_command);

			//down boundary
			sql_command
					= "INSERT INTO \"Partition_Boundary_Section\"(\"Partition_Solution_ID\", \"Partition_ID\", \"Boundary_Section_ID\",\"Section_Cut_Point_Offset\", \"Connect_Partition_ID\", \"In_Partition_Side\") VALUES ('";

			sql_command += boost::lexical_cast<std::string>(partition_solution_id) + "','";
			sql_command += boost::lexical_cast<std::string>(to_node_partition) + "','";
			sql_command += strs[0] + "','";
			sql_command += boost::lexical_cast<std::string>(offset) + "','";
			sql_command += boost::lexical_cast<std::string>(from_node_partition) + "',";
			sql_command += "'-1' )";

			connection.insertSQL(sql_command);
		}
	}
}
