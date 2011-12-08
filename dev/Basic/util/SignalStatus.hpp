#pragma once

#include <iostream>
#include <string>
#include <sstream>

namespace sim_mob {
class SignalStatus
{
public:
	int TC_for_Driver[4][3];
	int TC_for_Pedestrian[4];
};
}
