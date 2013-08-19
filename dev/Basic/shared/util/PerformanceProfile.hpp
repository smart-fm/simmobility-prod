//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <sys/time.h>

#include "util/LangHelpers.hpp"

namespace sim_mob {
class PerformanceProfile
{
private:
	timeval start_update_time;
	timeval end_update_time;

	std::vector<double> measured_query_cost_array;
	double buffer_sum; //ms

	//If "measureParallel" is on, these vectors contain <numthread> elements. Else, they
	//  contain one element.
	std::vector<timeval> start_query_times;
	std::vector<timeval> end_query_times;

	timeval start_simulation_time;
	timeval end_simulation_time;

	timeval anything_start_time;
	timeval anything_end_time;

	timeval anything_start_time2;
	timeval anything_end_time2;

	double total_update_cost; //ms
	double total_query_cost; //ms
	double total_simulation_cost; //ms

	bool measureParallel;
	bool start_measure;
	int thread_size;

public:

	PerformanceProfile() : buffer_sum(0), total_update_cost(0),
						   total_query_cost(0), total_simulation_cost(0),
						   measureParallel(false), start_measure(0), thread_size(0)
	{}

	void init(int threads_, bool measureParallel);

	void startMeasure();
	void endMeasure();

	void markStartUpdate();
	void markEndUpdate();

	//xuyan: might have multi-thread problem
	void markEndQuery(int thread_id);
	void markStartQuery(int thread_id);

	void update();

	void markStartSimulation();
	void markEndSimulation();
	void markAnthingStart();
	double markAnthingEnd();
	void markAnthingStart2();
	double markAnthingEnd2();
	void showPerformanceProfile();
};

}
