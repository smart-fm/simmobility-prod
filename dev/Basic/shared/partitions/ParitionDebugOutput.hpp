//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <iostream>

#include "conf/settings/DisableMPI.h"

#include "PartitionManager.hpp"

namespace sim_mob {
/**
 * \author Xu Yan
 */
class ParitionDebugOutput
{
public:
	//testing
	template<class DATA_TYPE>
	void static outputToConsole(DATA_TYPE value)
	{
#ifndef SIMMOB_DISABLE_MPI
		std::cout << PartitionManager::instance().partition_config->partition_id << ":" << value << std::endl;
#endif
	}

	template<class DATA_TYPE>
	void static outputToConsoleWithoutNewLine(DATA_TYPE value)
	{
#ifndef SIMMOB_DISABLE_MPI
		std::cout << PartitionManager::instance().partition_config->partition_id << ":(" << value << ")";
#endif
	}
};
}
