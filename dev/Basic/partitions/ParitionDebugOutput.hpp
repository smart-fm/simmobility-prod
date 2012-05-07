#pragma once

#include "PartitionManager.hpp"
#include <string>
#include <iostream>

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
		std::cout << PartitionManager::instance().partition_config->partition_id << ":" << value << std::endl;
	}

	template<class DATA_TYPE>
	void static outputToConsoleWithoutNewLine(DATA_TYPE value)
	{
		std::cout << PartitionManager::instance().partition_config->partition_id << ":(" << value << ")";
	}
};
}
