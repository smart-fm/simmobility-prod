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
