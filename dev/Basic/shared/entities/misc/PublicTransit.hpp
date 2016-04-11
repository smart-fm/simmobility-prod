//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "util/DailyTime.hpp"
#include "boost/lexical_cast.hpp"
#include "path/Common.hpp"

namespace sim_mob {

class PT_BusDispatchFreq {
public:
	std::string frequencyId;
	std::string routeId;
	sim_mob::DailyTime startTime;
	sim_mob::DailyTime endTime;
	int headwaySec;
};

class PT_BusRoutes {
public:
	std::string routeId;
	std::string linkId;
	int sequenceNo;
};

class PT_BusStops {
public:
	std::string routeId;
	std::string stopNo;
	int sequenceNo;
};

class OD_Trip {
public:
	  std::string startStop;
	  std::string endStop;
	  int sType;
	  int eType;
	  sim_mob::PT_EdgeType tType;
	  std::string tTypeStr;
	  std::string serviceLines;
	  std::string pathset;
	  int id;
	  std::string originNode;
	  std::string destNode;
	  std::string scenario;
	  double travelTime;
	  double walkTime;
};

class MatchesOD_Trip {

std::string originId;
std::string destId;

public:
	std::vector<const OD_Trip*> result;
	MatchesOD_Trip(const int original,const int dest){
		originId=boost::lexical_cast<std::string>(original);
		destId=boost::lexical_cast<std::string>(dest);
	}

	bool operator()(const OD_Trip& item){
		if(item.originNode==originId && item.destNode==destId){
			result.push_back(&item);
		}
		return false;
	}

};

}
