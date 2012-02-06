/*
 * Configuration.hpp
 *
 *  Created on: 06-Feb-2012
 *      Author: xuyan
 */

#pragma once

#include <string>

using namespace std;

namespace partitioning {
class Configuration
 {
 public:
	/**
	 *Only one object can be created
	 */
	static Configuration &
	instance()
	{
		return instance_;
	}

 public:
	string getNodeSQL;
	string getSectionSQL;
	int partition_size;
	double cut_lane_minimum_size;
	string cut_lane_can_curve;
	int output_format;

 private:
 	static Configuration instance_;
 };
}
