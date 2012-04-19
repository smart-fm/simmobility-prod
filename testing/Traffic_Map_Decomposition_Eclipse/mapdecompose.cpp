/* Copyright Singapore-MIT Alliance for Research and Technology */
/**
 * argv[1] : node.weight
 * argv[2] : flow.data.size
 * argv[3] : infor.data.size
 * argv[4] : output folder
 * argv[5] : nparts          (>=2)
 * argv[6] : load imbalance  (>1)
 */

#include "all_includes.hpp"
#include "algorithms/Configurations.hpp"
#include "algorithms/MapDecompositionWithInformationFlow.hpp"
#include "algorithms/MapDecompositionWithoutInformationFlow.hpp"
#include "database/ChangeDatabasePartition.hpp"

using namespace std;
using namespace sim_mob_partitioning;

/*
 * includes parameters that will change the behaviors of partitioning algorithm
 */
Configurations config;

/**
 * supporting functions
 */
void show_menu();
bool is_file_or_folder_exist(const char* filename);
bool check_user_inputs(char* argv[]);

/**
 * start location
 */
int main(int argc, char* argv[])
{
	if (argc != 9)
	{
		show_menu();
		cout << "Parameters in main function is not enough, program exit" << endl;
		return 1;
	}

	if (check_user_inputs(argv) == false)
	{
		show_menu();
		cout << "Parameters setting not correct, program exit" << endl;
		return 1;
	}

	/*
	 *If there are information inputs, then use MapDecompositionWithInformationFlow
	 *If not,use MapDecompositionWithoutInformationFlow
	 */
	if (config.infor_file.compare("NULL") == 0 || config.infor_file.compare("null") == 0)
	{
		MapDecompositionWithoutInformationFlow algorithm;
		algorithm.do_map_partitioning(config);
	}
	else
	{
		MapDecompositionWithInformationFlow algorithm;
		algorithm.do_map_partitioning(config);
	}

	if(config.database_config_file.compare("NULL") != 0 && config.database_config_file.compare("null") != 0)
	{
		ChangeDatabasePartition database_util;
		database_util.push_partitions_to_database(config);
	}

	return 0;
}

/* argv[1] : node.weight
 * argv[2] : flow.data.size
 * argv[3] : infor.data.size
 * argv[4] : output folder
 * argv[5] : nparts          (>=2)
 * argv[6] : load imbalance  (>1)
 */
void show_menu()
{
	std::cout << "-------------------------------------------" << std::endl;
	std::cout << "Required Parameters" << std::endl;
	std::cout
			<< "argv[1] : the input file describing CPU cost of each node (eg. no of vehicle * minute or length of lanes)"
			<< std::endl;
	std::cout
			<< "argv[2] : the input file describing Communication between nodes (eg. no of vehicle crossing or no of cut-lanes)"
			<< std::endl;
	std::cout << "argv[3] : the input file describing information exchange, if no such file, set NULL" << std::endl;
	std::cout << "argv[4] : the output folder, if you do not care medium results, set NULL" << std::endl;
	std::cout << "argv[5] : nparts: >= 2" << std::endl;
	std::cout << "argv[6] : load imbalance: >= 1, (suggestion: 1.05)" << std::endl;
	std::cout << "argv[7] : execute_speed: 0: fast; 1:medium; 2:slow, (suggestion: 2)" << std::endl;
	std::cout << "argv[8] : database configuration: if you want to update Git database, specify the database configuration file, if not change database, set NULL" << std::endl;
	std::cout << "-------------------------------------------" << std::endl;
}

bool check_user_inputs(char* argv[])
{
	//input processing
	config.node_file = (argv[1]);
	config.flow_file = (argv[2]);
	config.infor_file = (argv[3]);
	config.output_folder = (argv[4]);

	config.nparts = boost::lexical_cast<int>(argv[5]);
	config.load_imbalance = boost::lexical_cast<double>(argv[6]);
	config.execute_speed = boost::lexical_cast<int>(argv[7]);

	config.database_config_file = (argv[8]);

	if (config.nparts < 2)
	{
		std::cout << "nparts can not < 2" << std::endl;
		return false;
	}

	if (config.load_imbalance < 1)
	{
		std::cout << "load_imbalance can not < 1" << std::endl;
		return false;
	}

	if (config.execute_speed != 0 && config.execute_speed != 1 && config.execute_speed != 2)
	{
		std::cout << "execute_speed can only be 0,1,2" << std::endl;
		return false;
	}

	//file and folder check
	if (is_file_or_folder_exist(config.node_file.c_str()) == false)
	{
		std::cout << "node_file not existing" << std::endl;
		return false;
	}

	if (is_file_or_folder_exist(config.flow_file.c_str()) == false)
	{
		std::cout << "flow_file not existing" << std::endl;
		return false;
	}

	if (config.infor_file.compare("NULL") != 0 && config.infor_file.compare("null") != 0)
	{
		if (is_file_or_folder_exist(config.infor_file.c_str()) == false)
		{
			std::cout << "information_folder not existing" << std::endl;
			return false;
		}
	}

	if (config.output_folder.compare("NULL") == 0 || config.output_folder.compare("null") == 0)
	{
		config.output_folder = "temp_outputs";
	}

	if (is_file_or_folder_exist(config.output_folder.c_str()) == false)
	{
		std::cout << "output_folder not existing" << std::endl;
		return false;
	}

	if (config.database_config_file.compare("NULL") != 0 && config.database_config_file.compare("null") != 0)
	{
		if (is_file_or_folder_exist(config.database_config_file.c_str()) == false)
		{
			std::cout << "database configuration file not existing" << std::endl;
			return false;
		}
	}

	return true;
}

/**
 * check the file exist
 */
bool is_file_or_folder_exist(const char* filename)
{
	struct stat buffer;
	if (stat(filename, &buffer) == 0)
		return true;
	return false;
}
