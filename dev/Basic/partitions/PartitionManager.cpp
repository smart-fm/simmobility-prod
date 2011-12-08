/*
 * PartitionManager.cpp
 *
 */

#include "constants.h"
#ifndef SIMMOB_DISABLE_MPI

#include "PartitionManager.hpp"
#include <iostream>

#include "mpi.h"
#include <boost/mpi.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/thread/mutex.hpp>

#include "BoundarySegment.hpp"
#include "util/MathUtil.hpp"
#include "conf/simpleconf.hpp"
#include "workers/EntityWorker.hpp"
#include "workers/Worker.hpp"
#include "WorkGroup.hpp"
#include "AgentPackageManager.hpp"

namespace mpi = boost::mpi;

namespace sim_mob {

PartitionManager PartitionManager::instance_;
int PartitionManager::count = 0;

void initMPIConfigurationParameters(PartitionConfigure* partition_config, SimulationScenario* scenario)
{
	partition_config->adaptive_load_balance = false;
	partition_config->boundary_length = 100 * 100; //mm
	partition_config->boundary_width = 20 * 100; //mm
	partition_config->measurem_performance = false;
	partition_config->maximum_agent_id = 10000;
	partition_config->measure_output_file = "";
	partition_config->partition_solution_id = 1;

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
	std::string input = argv[1];
	std::string id = MathUtil::getStringFromNumber(partition_id + 1);
	input += "_";
	input += id;
	input += ".xml";
	argv[1] = (char*) input.c_str();

	std::string output = argv[2];
	output += id;
	output += ".txt";
	argv[2] = (char*) output.c_str();
}

std::string PartitionManager::startMPIEnvironment(int argc, char* argv[], bool config_adaptive_load_balance,
		bool config_measure_cost)
{
	//start MPI
	//	MPI_Init(&argc, &argv);

	int pmode;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &pmode);
	if (pmode != MPI_THREAD_MULTIPLE)
	{
		std::cout << "Thread Multiple not supported by the MPI implementation" << std::endl;
		MPI_Abort(MPI_COMM_WORLD, -1);
		return "MPI start failed";
	}

	mpi::communicator world;
	int computer_size = world.size();
	if (computer_size <= 0)
	{
		MPI_Finalize();

		return "configuration error, computer size must > 1";
	}
	else if (computer_size == 1)
	{
		std::string input = argv[1];
		input += ".xml";
		argv[1] = (char*) input.c_str();

		//		ConfigParams& config = ConfigParams::GetInstance();
		//		config.is_run_on_many_computers = false;
		//for internal use
		//		is_on_many_computers = false;

		MPI_Finalize();
		return "";
	}
	//temp
	else if (computer_size > 2)
	{
		return "Sorry, currently can not support more than 2 computers, because the configure file is hard coded.";
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

void PartitionManager::setEntityWorkGroup(sim_mob::WorkGroup<Entity>* entity_group,
		sim_mob::WorkGroup<Entity>* singal_group)
{
	//	if (is_on_many_computers == false)
	//		return;

	processor.setEntityWorkGroup(entity_group, singal_group);
}

void PartitionManager::initBoundaryTrafficItems()
{
	//	if (is_on_many_computers == false)
	//		return;

	processor.initBoundaryTrafficItems();
}

void PartitionManager::loadInBoundarySegment(std::string boundary_segment_id, BoundarySegment* boundary)
{
	//	if (is_on_many_computers == false)
	//		return;

	processor.loadInBoundarySegment(boundary_segment_id, boundary);
}

void PartitionManager::updateRandomSeed()
{
	std::vector<Agent*> all_agents = Agent::all_agents;
	//
	std::vector<Agent*>::iterator itr = all_agents.begin();
	for (; itr != all_agents.end(); itr++)
	{
		Agent* one_agent = (*itr);
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
			p->getRole()->dynamic_seed = one_agent->getId();

			//update pedestrain speed
			const Pedestrian *pedestrian = dynamic_cast<const Pedestrian *> (p->getRole());
			if (pedestrian)
			{
				Pedestrian *one_pedestrian = const_cast<Pedestrian*> (pedestrian);
				one_pedestrian->speed = 1.2 + (double(one_agent->getId() % 5)) / 10;
			}
		}
	}
}

std::string PartitionManager::crossPCboundaryProcess(int time_step)
{
	//	if (is_on_many_computers == false)
	//		return "";

	return processor.boundaryProcessing(time_step);
}

std::string PartitionManager::crossPCBarrier()
{
	//	if (is_on_many_computers == false)
	//		return "";

	mpi::communicator world;
	world.barrier();

	return "";
}

std::string PartitionManager::outputAllEntities(frame_t time_step)
{
	return processor.outputAllEntities(time_step);
}

std::string PartitionManager::stopMPIEnvironment()
{
	//	if (is_on_many_computers == false)
	//		return "";

	MPI_Finalize();
	std::cout << "Finished" << std::endl;
	return ""; //processor.releaseMPIEnvironment();
}

std::string PartitionManager::adaptiveLoadBalance()
{
	//do nothing now

	return "";
}
}

#endif

