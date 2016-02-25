/*
 * TrainTrip.hpp
 *
 *  Created on: Feb 19, 2016
 *      Author: zhang huai peng
 */
#ifndef TRAINTRIP_HPP_
#define TRAINTRIP_HPP_
#include <vector>
#include <map>
#include <string>


#include "buffering/Shared.hpp"
#include "conf/settings/DisableMPI.h"
#include "entities/misc/TripChain.hpp"
#include "util/DailyTime.hpp"

namespace sim_mob {
class Block;
class Platform;

class TrainTrip : public sim_mob::Trip {
public:
	TrainTrip();
	virtual ~TrainTrip();
	void setLineId(const std::string& id){
		lineId = id;
	}
	std::string getLineId() const{
		return lineId;
	}
	void setTripId(int id){
		tripId = id;
	}
	int getTripId() const{
		return tripId;
	}
	void setTrainRoute(const std::vector<Block*>& route){
		trainRoute = route;
	}
	const std::vector<Block*>& getTrainRoute() const{
		return trainRoute;
	}
	void setTrainPlatform(const std::vector<Platform*>& platform){
		trainPlatform = platform;
	}
	const std::vector<Platform*>& getTrainPlatform() const{
		return trainPlatform;
	}
private:
	/**train line id*/
	std::string lineId;
	/**trip id of current train*/
	int tripId;
	/**train route include a list of blocks*/
	std::vector<Block*> trainRoute;
	/**train platform include a list of platform*/
	std::vector<Platform*> trainPlatform;
};

} /* namespace sim_mob */

#endif
