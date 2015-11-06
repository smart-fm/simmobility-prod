//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <map>
#include <string>
#include <vector>

#include "entities/models/Constants.h"

using namespace std;

namespace sim_mob
{

enum StatusValue
{
	STATUS_NO = -1,
	STATUS_UNKNOWN = 0,
	STATUS_YES = 1
};

struct SMStatus
{
	/**Indicates the type of status*/
	string statusName;

	/**Indicates the status value*/
	StatusValue status;

	/**Indicates information such as the method that set the status*/
	string recordInfo;

	SMStatus();
	virtual ~SMStatus();
};

class SMStatusManager
{
private:
	/**Stores the the type of status as key and the status value as the value*/
	map<string, SMStatus> statusMap;
public:
	SMStatusManager()
	{
	}

	virtual ~SMStatusManager()
	{
	}

	void setStatus(string name, StatusValue value, string whoSet);
	StatusValue getStatus(string name);
};

}