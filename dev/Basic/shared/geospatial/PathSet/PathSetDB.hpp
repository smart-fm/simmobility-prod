/*
 * PathSetDB.hpp
 *
 *  Created on: May 17, 2013
 *      Author: Max
 */

#ifndef PATHSETDB_H_
#define PATHSETDB_H_

#include <string>

namespace sim_mob
{

class PathSetDB {
public:
	PathSetDB();
	PathSetDB(PathSetDB& data) : id(data.id),from_node_id(data.from_node_id),
			to_node_id(data.to_node_id),singlepath_id(data.singlepath_id),
			scenario(data.scenario){}
	virtual ~PathSetDB();

public:
	std::string id;
	std::string from_node_id;
	std::string to_node_id;
//	std::string person_id;
//	std::string trip_id;
	std::string singlepath_id;
	std::string scenario;
};

class PathPoolDB {
public:
	PathPoolDB() {}
	virtual ~PathPoolDB(){}

public:
	std::string id;
	std::string singlepath_id;
	std::string scenario;
};

class SinglePathDB {
public:
	SinglePathDB(SinglePathDB& data):utility(data.utility),pathsize(data.pathsize),
	travel_distance(data.travel_distance),travel_cost(data.travel_cost),
	signal_number(data.signal_number),right_turn_number(data.right_turn_number),
	id(data.id),exclude_seg_id(data.exclude_seg_id),pathset_id(data.pathset_id),
	waypointset(data.waypointset),from_node_id(data.from_node_id),
	to_node_id(data.to_node_id),scenario(data.scenario),length(data.length),travle_time(data.travle_time){}
	SinglePathDB():utility(0.0),pathsize(0.0),travel_distance(0.0),travel_cost(0.0),
		signal_number(0.0),right_turn_number(0.0),length(0.0),travle_time(0.0){}
	virtual ~SinglePathDB(){}

public:
	std::string id;
	std::string exclude_seg_id;
	std::string pathset_id;
	std::string waypointset; //seg path aimsun id: seg1id_seg2id_seg3id
	std::string from_node_id;
	std::string to_node_id;
	double utility;
	double pathsize;
	double travel_distance;
	double travel_cost;
	int signal_number;
	int right_turn_number;
	std::string scenario;
	double length;
	double travle_time;
};
}
#endif /* PATHSETDB_H_ */
