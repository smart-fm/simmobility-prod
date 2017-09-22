//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include <sstream>
#include <stdexcept>
#include "SMStatus.hpp"

using namespace std;
using namespace sim_mob;

SMStatus::SMStatus()
: status(STATUS_UNKNOWN), statusName("No Name"), recordInfo("Initial Status")
{
}

SMStatus::~SMStatus()
{
}

void SMStatusManager::setStatus(string name, StatusValue value, string whoSet)
{
	map<string, SMStatus>::iterator it = statusMap.find(name);
	
	if (it != statusMap.end())
	{
		it->second.status = value;
		it->second.recordInfo = whoSet;
	}
	else
	{
		//Add status
		SMStatus status;
		status.statusName = name;
		status.status = value;
		status.recordInfo = whoSet;
		statusMap.insert(std::make_pair(name, status));
	}
}

StatusValue SMStatusManager::getStatus(string name)
{
	map<string, SMStatus>::iterator it = statusMap.find(name);
	if (it != statusMap.end())
	{
		StatusValue v = it->second.status;
		return v;
	}
	else
	{
		std::stringstream msg;
		msg << __func__ << ": Unknown status: " << name;
		throw std::runtime_error(msg.str());
	}
}
