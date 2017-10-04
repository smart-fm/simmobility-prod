//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#pragma once

#include <stdexcept>

using namespace std;

namespace sim_mob
{
namespace medium
{

class no_path_error : public runtime_error
{
public:

	explicit no_path_error(const string &arg) : runtime_error(arg)
	{
	}
};

}
}