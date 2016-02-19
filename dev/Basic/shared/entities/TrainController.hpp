/*
 * TrainController.hpp
 *
 *  Created on: Feb 11, 2016
 *      Author: zhang huai peng
 */
#ifndef _CLASS_TRAIN_CONTROLLER
#define _CLASS_TRAIN_CONTROLLER
#include <string>
#include <map>
#include <type_traits>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/static_assert.hpp>
#include "geospatial/network/Block.hpp"
#include "geospatial/network/Platform.hpp"
#include "entities/Agent.hpp"
#include "entities/misc/TrainTrip.hpp"

namespace sim_mob {
/**
 * the structure to store the train route
 */
struct TrainRoute {
	TrainRoute() :
			sequenceNo(0), blockId(0) {
	}
	;
	std::string lineId;
	int blockId;
	int sequenceNo;
};
/**
 * the structure to store the train stops
 */
struct TrainPlatform {
	TrainPlatform() :
			sequenceNo(0) {
	}
	;
	std::string lineId;
	std::string platformNo;
	int sequenceNo;
};
/**
 * the structure to store train schedule
 */
struct TrainSchedule {
	TrainSchedule() :
			scheduleId(0), headwaySec(0) {
	}
	;
	int scheduleId;
	std::string lineId;
	std::string startTime;
	std::string endTime;
	int headwaySec;
};
/**
 * the structure to store transfered time between platforms
 */
struct TransferTimeInPlatform {
	TransferTimeInPlatform() :
			transferedTimeSec(0) {
	}
	;
	std::string stationNo;
	std::string platformFirst;
	std::string platformSecond;
	int transferedTimeSec;
};

template<typename PERSON>
class TrainController: public sim_mob::Agent {
	BOOST_STATIC_ASSERT_MSG(
			(boost::is_base_of<sim_mob::Person, PERSON>::value),
			"T must be a descendant of sim_mob::Person"
	);
public:
	explicit TrainController(int id = -1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);
	virtual ~TrainController();

public:
	/**
	 * initialize the train controller
	 */
	void initTrainController();
	/**
	 * update nearby previous and next train location
	 */
	void updateTrainPosition();
	/**
	 * get global instance for train controller
	 */
	static TrainController* getInstance();
	/**
	 * checks if the train controller instance exists
	 */
	static bool HasTrainController();
	/**
	 * assign train trip to person
	 */
	void assignTrainTripToPerson(std::set<Entity*>& activeAgents);
protected:
	/**
	 * inherited from base class agent to initialize parameters for train controller
	 */
	virtual Entity::UpdateStatus frame_init(timeslice now);

	/**
	 * inherited from base class to update this agent
	 */
	virtual Entity::UpdateStatus frame_tick(timeslice now);

	/**
	 * inherited from base class to output result
	 */
	virtual void frame_output(timeslice now);
	/**
	 * Signals are non-spatial in nature.
	 */
	virtual bool isNonspatial();
	/**
	 * get block list for a particular line
	 * @param lineId is line id
	 * @param route is the list of blocks
	 * @return true if successfully get the list of blocks
	 */
	bool getTrainRoute(const std::string& lineId, std::vector<Block*>& route);
	/**
	 * get platform list for a particular line
	 * @param lineId is line id
	 * @param platforms are the list of platforms
	 * @return true if successfully get the list of platforms
	 */
	bool getTrainPlatforms(const std::string& lineId, std::vector<Platform*>& platforms);
private:
	/**
	 * the function to load platforms from DB
	 */
	void loadPlatforms();
	/**
	 * the function to load schedules from DB
	 */
	void loadSchedules();
	/**
	 * the function to load blocks from DB
	 */
	void loadBlocks();
	/**
	 * the function to load routes from DB
	 */
	void loadTrainRoutes();
	/**
	 * the function to load train platforms from DB
	 */
	void loadTrainPlatform();
	/**
	 * the function to load transfered time between platforms from DB
	 */
	void loadTransferedTimes();
	/**
	 * the function to load polylines from DB
	 */
	void loadBlockPolylines();

private:
	/**the map from id to the object of platform*/
	std::map<std::string, Platform*> mapOfIdvsPlatforms;
	/**the map from id to the object of block*/
	std::map<unsigned int, Block*> mapOfIdvsBlocks;
	/**the map from line id to the train route*/
	std::map<std::string, std::vector<TrainRoute>> mapOfIdvsTrainRoutes;
	/**the map from line id to the train platform*/
	std::map<std::string, std::vector<TrainPlatform>> mapOfIdvsTrainPlatforms;
	/**the map from id to the schedule table*/
	std::map<std::string, std::vector<TrainSchedule>> mapOfIdvsSchedules;
	/**the map from id to polyline object*/
	std::map<unsigned int, PolyLine*> mapOfIdvsPolylines;
private:
	static TrainController* pInstance;
};

}
/* namespace sim_mob */

#define _CLASS_TRAIN_CONTROLLER_FUNCTIONS
#include "TrainController.cpp"
#endif
