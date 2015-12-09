/*
 * TimeCheck.cpp
 *
 *  Created on: 9 Dec 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <util/TimeCheck.hpp>

using namespace sim_mob::long_term;

TimeCheck::TimeCheck()
{
	start_clock   = time(0);
	start_process = clock();
}

double TimeCheck::getClockTime()
{
	time_t end_clock = time(0);
	double time_clock = difftime( end_clock, start_clock );

	return time_clock;

}

double TimeCheck::getProcessTime()
{

	clock_t end_process = clock();
	double time_process = (double) ( end_process - start_process ) / CLOCKS_PER_SEC;

	return time_process;
}

TimeCheck::~TimeCheck(){}

