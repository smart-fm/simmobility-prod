//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace sim_mob
{

/**
 * \author Xu Yan
 */
class MathUtil
{
public:

	template<typename T>
	static std::string getStringFromNumber(const T number)
	{
		std::stringstream convert;
		convert << number;
		return convert.str();
	}

};
}
