/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "ChangeDatabasePartition.hpp"

using namespace std;
using namespace sim_mob_partitioning;


void ChangeDatabasePartition::push_partitions_to_database(Configurations& config)
{
	load_in_database_configuration();
	generate_requried_data();
	push_to_DB();
}

void ChangeDatabasePartition::load_in_database_configuration()
{

}

void ChangeDatabasePartition::generate_requried_data()
{

}

void ChangeDatabasePartition::push_to_DB()
{
	std::cout << "Push partitioning to Database is not finished yet" << std::endl;
}
