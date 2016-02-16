/*
 * TrainController.hpp
 *
 *  Created on: Feb 11, 2016
 *      Author: zhang huai peng
 */
#include <string>
#include <map>
#include "geospatial/network/Block.hpp"
#include "geospatial/network/Platform.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {
/**
 * the structure to store the train route
 */
struct TrainRoute{
	TrainRoute():sequenceNo(0){};
	std::string lineId;
	std::string blockId;
	int sequenceNo;
};
/**
 * the structure to store train schedule
 */
struct TrainSchedule{
	TrainSchedule():scheduleId(0),headwaySec(0){};
	int scheduleId;
	std::string lineId;
	std::string startTime;
	std::string endTime;
	int headwaySec;
};
/**
 * the structure to store transfered time between platforms
 */
struct TransferTimeInPlatform{
	TransferTimeInPlatform():transferedTimeSec(0){};
	std::string stationNo;
	std::string platformFirst;
	std::string platformSecond;
	int transferedTimeSec;
};

class TrainController : public sim_mob::Agent{
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
	void loadRoutes();
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
	std::map<std::string, std::vector<TrainRoute>> mapOfIdvsRoutes;
	/**the map from id to the schedule table*/
	std::map<std::string, std::vector<TrainSchedule>> mapOfIdvsSchedules;
	/**the map from id to polyline object*/
	std::map<unsigned int, PolyLine*> mapOfIdvsPolylines;
private:
	static TrainController* pInstance;
};

} /* namespace sim_mob */


