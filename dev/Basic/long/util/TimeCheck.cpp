//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

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
	start = boost::chrono::system_clock::now();
}

double TimeCheck::getClockTime_sec()
{
	time_t end_clock = time(0);
	double time_clock = difftime( end_clock, start_clock );

	return time_clock;
}

double TimeCheck::getClockTime()
{
	boost::chrono::duration<double> durationStart = start.time_since_epoch();
	boost::chrono::duration<double> durationEnd = boost::chrono::system_clock::now().time_since_epoch();

	boost::chrono::microseconds d = boost::chrono::duration_cast<boost::chrono::microseconds>(durationEnd - durationStart);

	return d.count();
}

double TimeCheck::getProcessTime()
{

	clock_t end_process = clock();
	double time_process = (double) ( end_process - start_process ) / CLOCKS_PER_SEC;

	return time_process;
}

TimeCheck::~TimeCheck(){}

