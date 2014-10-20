//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include "util/DailyTime.hpp"
#include "boost/lexical_cast.hpp"

namespace sim_mob {


class PT_trip {
public:
	std::string trip_id;
	std::string service_id;
	std::string route_id;
	sim_mob::DailyTime start_time;
	sim_mob::DailyTime end_time;

};

class PT_bus_dispatch_freq {
public:
	std::string frequency_id;
	std::string route_id;
	sim_mob::DailyTime start_time;
	sim_mob::DailyTime end_time;
	int headway_sec;
};

class PT_bus_routes {
public:
	std::string route_id;
	std::string link_id;
	int link_sequence_no;
};

class PT_bus_stops {
public:
	std::string route_id;
	std::string busstop_no;
	int busstop_sequence_no;
};

class OD_Trip {
public:
	  std::string startStop;
	  std::string endStop;
	  std::string type;
	  std::string serviceLines;
	  int OD_Id;
	  int legId;
	  std::string originNode;
	  std::string destNode;
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
		Print()<<"original Id:"<<item.OD_Id
				<<" destination Id:"<<item.destNode <<std::endl;
		if(item.originNode==originId && item.destNode==destId){
			result.push_back(&item);
		}
		return false;
	}

};

}
