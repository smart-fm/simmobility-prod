/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "DBConnection.hpp"
#include <string>
#include <sstream>

namespace sim_mob_partitioning {
DBConnection::DBConnection() :
	sql_(soci::postgresql, "host=172.18.127.157 port=5432 dbname=SimMobility_DB user=postgres password=5M_S1mM0bility")
{
}

void DBConnection::loadInRoadNode(Configurations& config)
{
	std::string sql_str = "select * from ";
	sql_str += config.node_sql;

	std::string file_name = config.network_node_file;
	FileOutputHelper file_helper;
	file_helper.openFile(file_name);

	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);

	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); it++)
	{
		soci::row const& row = *it;

		stringstream ss;
		int node_id = row.get<int> (0);
		ss << node_id << ",";

		double x_pos = row.get<double> (1);
		int int_x_pos = (int) x_pos;
		ss << int_x_pos << ",";

		double y_pos = row.get<double> (2);
		int int_y_pos = (int) y_pos;
		ss << int_y_pos;

		file_helper.output_to_file(ss.str());
	}

	file_helper.closeFile();
}

void DBConnection::loadInRoadSection(Configurations& config)
{
	std::string sql_str = "select * from ";
	sql_str += config.section_sql;

	std::string file_name = config.network_section_file;
	FileOutputHelper file_helper;
	file_helper.openFile(file_name);

	soci::rowset<soci::row> rs = (sql_.prepare << sql_str);

	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); it++)
	{
		soci::row const& row = *it;

		stringstream ss;

		int section_id = row.get<int> (0);
		ss << section_id << ",";

		string name = row.get<string> (1);
		ss << name << ",";

		int lane_size = row.get<int> (2);
		ss << lane_size << ",";

		double speed = row.get<double> (3);
		ss << speed << ",";

		double capacity = row.get<double> (4);
		ss << capacity << ",";

		int from_node_id = row.get<int> (5);
		ss << from_node_id << ",";

		int to_node_id = row.get<int> (6);
		ss << to_node_id << ",";

		double section_length = row.get<double> (7);
		ss << section_length;

		file_helper.output_to_file(ss.str());
	}

	file_helper.closeFile();
}

void DBConnection::loadInRoadWeight(Configurations& config)
{
	bool generate_node_weight = false;

	//if the user set the node weights to be NULL, it means to calculate the weight automatically from database
	if (config.node_file.compare("NULL") == 0 || config.node_file.compare("null") == 0)
	{
		config.node_file = "data/node.weight";
		generate_node_weight = true;
	}

	if (generate_node_weight)
	{
		//		std::cout << "YESY2" << std::endl;
		Automate_Weight_Calculator::generate_node_weight(config.node_file, config.network_node_file, config.network_section_file);
	}
}

void DBConnection::loadInRoadSectionWeight(Configurations& config)
{
	bool generate_section_weight = false;

	//if the user set the section weights to be NULL, it means to calculate the weight automatically from database
	if (config.flow_file.compare("NULL") == 0 || config.flow_file.compare("null") == 0)
	{
		config.flow_file = "data/traffic.flow.data";
		generate_section_weight = true;
	}

	if (generate_section_weight)
	{
		Automate_Weight_Calculator::generate_section_weight(config.flow_file, config.network_node_file, config.network_section_file);
	}
}

int DBConnection::insertAndGetScenarioID(Scenario setting, Configurations& config)
{
	//search the scenario.
	std::string sql_command = "SELECT \"Simulation_Scenario\".\"Scenario_ID\" FROM public.\"Simulation_Scenario\"";
	sql_command += " WHERE";
	sql_command += " \"Simulation_Scenario\".\"Road_Network\" = '" + setting.road_network_name + "' AND";
	sql_command += " \"Simulation_Scenario\".\"Partition_Size\" = '" + boost::lexical_cast<std::string>(config.nparts) + "' AND";
	sql_command += " \"Simulation_Scenario\".\"Days_in_Week\" = '" + setting.day_in_week + "' AND";
	sql_command += " \"Simulation_Scenario\".\"From_Time\" = '" + setting.from_time + "' AND";
	sql_command += " \"Simulation_Scenario\".\"To_Time\" = '" + setting.to_time + "' AND";
	sql_command += " \"Simulation_Scenario\".\"Holiday\" = '" + setting.is_holiday + "' AND";
	sql_command += " \"Simulation_Scenario\".\"Incident\" = '" + setting.has_incident + "' AND";
	sql_command += " \"Simulation_Scenario\".\"Weather\" = '" + setting.weather + "'";

//	std::cout << sql_command << std::endl;

	soci::rowset<soci::row> rs_2 = (sql_.prepare << sql_command);

	for (soci::rowset<soci::row>::const_iterator it = rs_2.begin(); it != rs_2.end(); it++)
	{
		soci::row const& row = *it;
		int id = row.get<int> (0);
		return id;
	}

	//insert the scenario.
	int scenario_id = getMaximumScenarioId() + 1;

	sql_command
			= "INSERT INTO \"Simulation_Scenario\"(\"Scenario_ID\", \"Road_Network\", \"Partition_Size\", \"Days_in_Week\",\"From_Time\", \"To_Time\", \"Holiday\", \"Incident\", \"Weather\") VALUES ('";
	sql_command += boost::lexical_cast<std::string>(scenario_id) + "','";
	sql_command += setting.road_network_name + "','";
	sql_command += boost::lexical_cast<std::string>(config.nparts) + "','";
	sql_command += setting.day_in_week + "','";
	sql_command += setting.from_time + "','";
	sql_command += setting.to_time + "','";
	sql_command += setting.is_holiday + "','";
	sql_command += setting.has_incident + "','";
	sql_command += setting.weather + "')";

	soci::statement st = (sql_.prepare << sql_command);
	st.execute(true);

	//redo search.
//	sql_command = "SELECT \"Simulation_Scenario\".\"Scenario_ID\" FROM public.\"Simulation_Scenario\"";
//	sql_command += " WHERE";
//	sql_command += " \"Simulation_Scenario\".\"Road_Network\" = '" + setting.road_network_name + "' AND";
//	sql_command += " \"Simulation_Scenario\".\"Partition_Size\" = '" + boost::lexical_cast<std::string>(config.nparts) + "' AND";
//	sql_command += " \"Simulation_Scenario\".\"Days_in_Week\" = '" + setting.day_in_week + "' AND";
//	sql_command += " \"Simulation_Scenario\".\"From_Time\" = '" + setting.from_time + "' AND";
//	sql_command += " \"Simulation_Scenario\".\"To_Time\" = '" + setting.to_time + "' AND";
//	sql_command += " \"Simulation_Scenario\".\"Holiday\" = '" + setting.is_holiday + "' AND";
//	sql_command += " \"Simulation_Scenario\".\"Incident\" = '" + setting.has_incident + "' AND";
//	sql_command += " \"Simulation_Scenario\".\"Weather\" = '" + setting.weather + "'";
//
//	soci::rowset<soci::row> rs_1 = (sql_.prepare << sql_command);
//
//	for (soci::rowset<soci::row>::const_iterator it = rs_1.begin(); it != rs_1.end(); it++)
//	{
//		soci::row const& row = *it;
//		int id = row.get<int> (0);
//		return id;
//	}
//
//	std::cout << "Error" << std::endl;
	return scenario_id;
}

int DBConnection::getMaximumScenarioId()
{
	std::string sql_command = "SELECT MAX(\"Scenario_ID\") FROM \"Simulation_Scenario\"";

	soci::rowset<soci::row> rs = (sql_.prepare << sql_command);

	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); it++)
	{
		soci::row const& row = *it;
		int id = row.get<int> (0);
		return id;
	}

	return -1;
}

int DBConnection::getMaximumPartitionSolutionId()
{
	std::string sql_command = "SELECT MAX(\"partition_solution_ID\") FROM \"Simulation_Scenario_Partition\"";

	soci::rowset<soci::row> rs = (sql_.prepare << sql_command);

	for (soci::rowset<soci::row>::const_iterator it = rs.begin(); it != rs.end(); it++)
	{
		soci::row const& row = *it;
		int id = row.get<int> (0);
		return id;
	}

	return -1;
}

void DBConnection::insertNewPartitionSolutionId(int scenario_id, int partition_solution_id)
{
	std::string sql_command = "INSERT INTO \"Simulation_Scenario_Partition\"(\"scenario_ID\", \"partition_solution_ID\", note) VALUES (";
	sql_command += boost::lexical_cast<std::string>(scenario_id) + ",";
	sql_command += boost::lexical_cast<std::string>(partition_solution_id) + ",";
	sql_command += "'For testing')";

	soci::statement st = (sql_.prepare << sql_command);
	st.execute(true);
}

void DBConnection::insertSQL(std::string insert_sql_command)
{
	soci::statement st = (sql_.prepare << insert_sql_command);
	st.execute(true);
}

}
