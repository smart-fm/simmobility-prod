/*
 * PartitionManager.cpp
 *
 */

#include "PartitionManager.hpp"
#ifndef SIMMOB_DISABLE_MPI

#include <iostream>

//#include "mpi.h"
#include <boost/mpi.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/thread/mutex.hpp>

#include "BoundarySegment.hpp"
#include "util/MathUtil.hpp"
#include "conf/simpleconf.hpp"
#include "workers/Worker.hpp"
#include "workers/WorkGroup.hpp"

#include "entities/roles/driver/Driver.hpp"
#include "entities/roles/passenger/Passenger.hpp"
#include "entities/roles/pedestrian/Pedestrian.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"

namespace mpi = boost::mpi;

using std::string;
using namespace sim_mob;


//Use an anonymous namespace for private, helper functions.
namespace {
void initMPIConfigurationParameters(PartitionConfigure* partition_config, SimulationScenario* scenario)
{
	partition_config->adaptive_load_balance = false;
	partition_config->boundary_length = 60 * 100; //feet
	partition_config->boundary_width = 20 * 100; //feet
	partition_config->measurem_performance = false;
	partition_config->maximum_agent_id = 10000;
	partition_config->measure_output_file = "";
	partition_config->partition_solution_id = 1; //default value, will be overloaded by the configured value

	//should be used later to find the best partition solution
	scenario->day_of_week = "Mon";
	scenario->from_time = "6:00:00:00";
	scenario->to_time = "7:00:00:00";
	scenario->holiday = "NO_HOLIDAY";
	scenario->incident = "NO_INCIDENT";
	scenario->road_network = "BUGIS";
	scenario->weather = "SUNNY";
}

void changeInputOutputFile(int argc, char* argv[], int partition_id)
{
	string input = argv[1];
	string id = MathUtil::getStringFromNumber(partition_id + 1);
	input += "_";
	input += id;
	input += ".xml";
	argv[1] = (char*) input.c_str();

	string output = argv[2];
	output += id;
	output += ".txt";
	argv[2] = (char*) output.c_str();
}
} //End anon namespace



PartitionManager sim_mob::PartitionManager::instance_;
int sim_mob::PartitionManager::count = 0;

PartitionManager& sim_mob::PartitionManager::instance()
{
	return instance_;
}


string sim_mob::PartitionManager::startMPIEnvironment(int argc, char* argv[], bool config_adaptive_load_balance,
		bool config_measure_cost)
{
	//Let is try on MPI_Init firstly. ~xuyan
	MPI_Init(&argc, &argv);

	mpi::communicator world;
	int computer_size = world.size();
	if (computer_size <= 0)
	{
		MPI_Finalize();

		return "configuration error, computer size must > 1";
	}
	else if (computer_size == 1)
	{
		string input = argv[1];
		input += ".xml";
		argv[1] = (char*) input.c_str();

		MPI_Finalize();
		return "";
	}

	std::cout << "MPI is started: " << world.size() << std::endl;
	/*
	 * 1. Init MPI configuration
	 * 2. User/Modeler can overload the MPI configuration in config.xml file
	 */

	//	is_on_many_computers = true;
	partition_config = new PartitionConfigure();
	scenario = new SimulationScenario();

	partition_config->partition_size = world.size();
	partition_config->partition_id = world.rank();

	initMPIConfigurationParameters(partition_config, scenario);
	changeInputOutputFile(argc, argv, partition_config->partition_id);

	processor.setConfigure(partition_config, scenario);
	return "";
}

void sim_mob::PartitionManager::setEntityWorkGroup(WorkGroup* entity_group,
		WorkGroup* singal_group)
{
	processor.setEntityWorkGroup(entity_group, singal_group);
}

void sim_mob::PartitionManager::initBoundaryTrafficItems()
{
	processor.initBoundaryTrafficItems();
}

void sim_mob::PartitionManager::loadInBoundarySegment(string boundary_segment_id, BoundarySegment* boundary)
{
	processor.loadInBoundarySegment(boundary_segment_id, boundary);
}

void sim_mob::PartitionManager::updateRandomSeed()
{
	std::vector<Entity*> all_agents = Agent::all_agents;
	//
	std::vector<Entity*>::iterator itr = all_agents.begin();
	for (; itr != all_agents.end(); itr++)
	{
		Entity* one_agent = (*itr);
		if (one_agent->id >= 10000)
		{
			one_agent->id = one_agent->id - 10000 + 3;
		}
		else if (one_agent->id >= 3)
		{
			one_agent->id = one_agent->id + 4;
		}

		const Person *person = dynamic_cast<const Person *> (one_agent);
		if (person)
		{
			Person* p = const_cast<Person*> (person);
			//init random seed
			p->dynamic_seed = one_agent->getId();

			//update pedestrain speed
			Pedestrian* pedestrian = dynamic_cast<Pedestrian*> (p->getRole());
			if (pedestrian)
			{
				pedestrian->speed = 1.2 + (double(one_agent->getId() % 5)) / 10;
			}
		}
	}
}

string sim_mob::PartitionManager::crossPCboundaryProcess(int time_step)
{
	return processor.boundaryProcessing(time_step);
}

string sim_mob::PartitionManager::crossPCBarrier()
{
	mpi::communicator world;
	world.barrier();

	return "";
}

string sim_mob::PartitionManager::outputAllEntities(timeslice now)
{
	return processor.outputAllEntities(now);
}

string sim_mob::PartitionManager::stopMPIEnvironment()
{
	MPI_Finalize();
	std::cout << "Finished" << std::endl;
	return ""; //processor.releaseMPIEnvironment();
}

string sim_mob::PartitionManager::adaptiveLoadBalance()
{
	//do nothing now
	return "";
}

#endif

