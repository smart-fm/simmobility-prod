/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <sys/time.h>

#define PARALLEL_MEASURE

namespace sim_mob {
class PerformanceProfile
{
private:
	timeval start_update_time;
	timeval end_update_time;

#ifdef PARALLEL_MEASURE
	timeval* start_query_time;
	timeval* end_query_time;
	double* measured_query_cost_array;
//	double total_query_time;
	double buffer_sum; //ms
#else
	timeval start_query_time;
	timeval end_query_time;
#endif

	timeval start_simulation_time;
	timeval end_simulation_time;

	timeval anything_start_time;
	timeval anything_end_time;

	timeval anything_start_time2;
	timeval anything_end_time2;

	double total_update_cost; //ms
	double total_query_cost; //ms
	double total_simulation_cost; //ms

//	double* measured_query_cost_array;
//	double total_query_time;

	bool start_measure;

	int thread_size;

public:

	PerformanceProfile()
	{
	}
	~PerformanceProfile()
	{
#ifdef PARALLEL_MEASURE
		if (start_query_time) {
			delete[] start_query_time;
			start_query_time = NULL;
		}

		if (end_query_time) {
			delete[] end_query_time;
			end_query_time = NULL;
		}

		if (measured_query_cost_array) {
			delete[] measured_query_cost_array;
			measured_query_cost_array = NULL;
		}
#endif
	}

	static PerformanceProfile &
	instance()
	{
		return instance_;
	}

	void init(int threads_)
	{
		start_measure = false;
		total_update_cost = 0;
		total_query_cost = 0;
		total_simulation_cost = 0;

#ifdef PARALLEL_MEASURE
		buffer_sum = 0;
//		total_query_time = 0;
		thread_size = threads_;
		start_query_time = new timeval[thread_size];
		end_query_time = new timeval[thread_size];
		measured_query_cost_array = new double[thread_size];

		for (int i = 0; i < thread_size; i++) {
			measured_query_cost_array[i] = 0;
		}
#endif
	}

	void startMeasure()
	{
		start_measure = true;
		std::cout << "start_measure is true" << std::endl;
	}

	void endMeasure()
	{
		start_measure = false;
		std::cout << "start_measure is false" << std::endl;
	}

	void markStartUpdate()
	{
		if (start_measure)
			gettimeofday(&start_update_time, NULL);
	}

	void markEndUpdate()
	{
		if (start_measure) {
			gettimeofday(&end_update_time, NULL);
			total_update_cost += diff_ms(end_update_time, start_update_time);
		}
	}

	void markStartQuery(int thread_id)
	{
//		std::cout << "thread_id:" << thread_id << std::endl;
#ifdef PARALLEL_MEASURE
		if (start_measure && thread_id >= 0)
			gettimeofday(&(start_query_time[thread_id]), NULL);
#else
		if (start_measure && thread_id == 0)
		gettimeofday(&start_query_time, NULL);
#endif
	}

	//xuyan: might have multi-thread problem
	void markEndQuery(int thread_id)
	{
#ifdef PARALLEL_MEASURE
		if (start_measure && thread_id >= 0) {
			gettimeofday(&(end_query_time[thread_id]), NULL);
			measured_query_cost_array[thread_id] += diff_ms(end_query_time[thread_id], start_query_time[thread_id]);
		}
#else
		if (start_measure && thread_id == 0) {
			gettimeofday(&(end_query_time), NULL);
			gettimeofday(&end_query_time, NULL);
			total_query_cost += diff_ms(end_query_time, start_query_time);
		}
#endif
	}

	void update()
	{
#ifdef PARALLEL_MEASURE
		double max_cost = 0;

		for (int i = 0; i < thread_size; i++) {
			if (max_cost < measured_query_cost_array[i]) {
				max_cost = measured_query_cost_array[i];
			}

			buffer_sum += measured_query_cost_array[i];
			measured_query_cost_array[i] = 0;
		}

		total_query_cost += max_cost;
#endif
	}

	void markStartSimulation()
	{
		if (start_measure)
			gettimeofday(&start_simulation_time, NULL);
	}

	void markEndSimulation()
	{
		if (start_measure) {
			gettimeofday(&end_simulation_time, NULL);
			total_simulation_cost += diff_ms(end_simulation_time, start_simulation_time);

			//			std::cout << "total_simulation_cost (ms)" << total_simulation_cost << std::endl;
		}
	}

	void markAnthingStart()
	{
		if (start_measure) {
			gettimeofday(&anything_start_time, NULL);
		}
	}

	double markAnthingEnd()
	{
		if (start_measure) {
			gettimeofday(&anything_end_time, NULL);
			double time = diff_ms(anything_end_time, anything_start_time);
			std::cout << time << std::endl;
			return time;
		}
		return 0;
	}

	void markAnthingStart2()
	{
		if (start_measure) {
			gettimeofday(&anything_start_time2, NULL);
		}
	}

	double markAnthingEnd2()
	{
		if (start_measure) {
			gettimeofday(&anything_end_time2, NULL);
			double time = diff_ms(anything_end_time2, anything_start_time2);
			return time;
		}
		return 0;
	}

	void showPerformanceProfile()
	{
		std::cout << "======PerformanceProfile========" << std::endl;
		std::cout << "total_update_cost (ms)" << total_update_cost << std::endl;
		std::cout << "total_query_cost (ms)" << total_query_cost << std::endl;
#ifdef PARALLEL_MEASURE
		std::cout << "all threads total_query_cost:" << buffer_sum << std::endl;
#endif
		std::cout << "total_simulation_cost (ms)" << total_simulation_cost << std::endl;
	}

private:

	//return ms
	double diff_ms(timeval t1, timeval t2)
	{
		//		std::cout << "======XXX========" << std::endl;
		//		std::cout << (t1.tv_sec - t2.tv_sec) * 1000.0 << std::endl;
		//		std::cout << (t1.tv_usec - t2.tv_usec) / 1000.0 << std::endl;

		return (((t1.tv_sec - t2.tv_sec) * 1000.0) + (t1.tv_usec - t2.tv_usec) / 1000.0);
	}

private:
	static PerformanceProfile instance_;
};

}
