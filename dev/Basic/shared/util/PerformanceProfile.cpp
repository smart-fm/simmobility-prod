/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "PerformanceProfile.hpp"

#include "logging/Log.hpp"
#include "util/Utils.hpp"

using std::vector;

namespace sim_mob {



void sim_mob::PerformanceProfile::init(int threads_, bool measureParallel)
{
	this->measureParallel = measureParallel;
	start_measure = false;
	total_update_cost = 0;
	total_query_cost = 0;
	total_simulation_cost = 0;

	buffer_sum = 0;
	thread_size = threads_;

	int numElements = measureParallel ? thread_size : 1;
	start_query_times = vector<timeval>(numElements, timeval());
	end_query_times = vector<timeval>(numElements, timeval());

	measured_query_cost_array.clear();
	if (measureParallel) {
		measured_query_cost_array = vector<double>(thread_size, 0);
	}
}


void sim_mob::PerformanceProfile::startMeasure()
{
	start_measure = true;
	PrintOut("start_measure is true" <<std::endl);
}

void sim_mob::PerformanceProfile::endMeasure()
{
	start_measure = false;
	PrintOut("start_measure is false" <<std::endl);
}


void sim_mob::PerformanceProfile::markStartUpdate()
{
	if (start_measure) {
		gettimeofday(&start_update_time, nullptr);
	}
}

void sim_mob::PerformanceProfile::markEndUpdate()
{
	if (start_measure) {
		gettimeofday(&end_update_time, nullptr);
		total_update_cost += Utils::diff_ms(end_update_time, start_update_time);
	}
}

void sim_mob::PerformanceProfile::update()
{
	if (measureParallel) {
		double max_cost = 0;

		for (int i = 0; i < thread_size; i++) {
			if (max_cost < measured_query_cost_array.at(i)) {
				max_cost = measured_query_cost_array.at(i);
			}

			buffer_sum += measured_query_cost_array.at(i);
			measured_query_cost_array.at(i) = 0;
		}

		total_query_cost += max_cost;
	}
}

void sim_mob::PerformanceProfile::markStartQuery(int thread_id)
{
	if (measureParallel) {
		if (start_measure && thread_id >= 0) {
			gettimeofday(&(start_query_times.at(thread_id)), NULL);
		}
	} else {
		if (start_measure && thread_id == 0) {
			gettimeofday(&start_query_times.front(), NULL);
		}
	}
}

//xuyan: might have multi-thread problem
void sim_mob::PerformanceProfile::markEndQuery(int thread_id)
{
	if (measureParallel) {
		if (start_measure && thread_id >= 0) {
			gettimeofday(&(end_query_times.at(thread_id)), NULL);
			measured_query_cost_array.at(thread_id) += Utils::diff_ms(end_query_times.at(thread_id), start_query_times.at(thread_id));
		}
	} else {
		if (start_measure && thread_id == 0) {
			gettimeofday(&(end_query_times.front()), NULL);
			gettimeofday(&end_query_times.front(), NULL);
			total_query_cost += Utils::diff_ms(end_query_times.front(), start_query_times.front());
		}
	}
}

void sim_mob::PerformanceProfile::markStartSimulation()
{
	if (start_measure)
		gettimeofday(&start_simulation_time, NULL);
}

void sim_mob::PerformanceProfile::markEndSimulation()
{
	if (start_measure) {
		gettimeofday(&end_simulation_time, NULL);
		total_simulation_cost += Utils::diff_ms(end_simulation_time, start_simulation_time);

		//			std::cout << "total_simulation_cost (ms)" << total_simulation_cost << std::endl;
	}
}

void sim_mob::PerformanceProfile::markAnthingStart()
{
	if (start_measure) {
		gettimeofday(&anything_start_time, NULL);
	}
}

double sim_mob::PerformanceProfile::markAnthingEnd()
{
	if (start_measure) {
		gettimeofday(&anything_end_time, NULL);
		double time = Utils::diff_ms(anything_end_time, anything_start_time);
		std::cout << time << std::endl;
		return time;
	}
	return 0;
}

void sim_mob::PerformanceProfile::markAnthingStart2()
{
	if (start_measure) {
		gettimeofday(&anything_start_time2, NULL);
	}
}

double sim_mob::PerformanceProfile::markAnthingEnd2()
{
	if (start_measure) {
		gettimeofday(&anything_end_time2, NULL);
		double time = Utils::diff_ms(anything_end_time2, anything_start_time2);
		return time;
	}
	return 0;
}

void sim_mob::PerformanceProfile::showPerformanceProfile()
{
	std::cout << "======PerformanceProfile========" << std::endl;
	std::cout << "total_update_cost (ms)" << total_update_cost << std::endl;
	std::cout << "total_query_cost (ms)" << total_query_cost << std::endl;
	if (measureParallel) {
		std::cout << "all threads total_query_cost:" << buffer_sum << std::endl;
	}
	std::cout << "total_simulation_cost (ms)" << total_simulation_cost << std::endl;
}



}
