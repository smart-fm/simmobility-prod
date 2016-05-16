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
#include <boost/unordered_map.hpp>
#include "geospatial/network/Block.hpp"
#include "geospatial/network/Platform.hpp"
#include "entities/Agent.hpp"
#include "entities/misc/TrainTrip.hpp"
//#include "entities/roles/Role.hpp"
//#include "entities/Person_MT.hpp"
//#include "medium/entities/roles/driver/TrainDriver.hpp"

using namespace sim_mob::messaging;
namespace sim_mob {

const Message::MessageType MSG_TRAIN_BACK_DEPOT = 7300001;
/**
 * Message holding a pointer to trainDriver
 */


class TrainMessage: public messaging::Message
{
public:
	TrainMessage(Agent* agent):
		trainAgent(agent)
	{
	}
	virtual ~TrainMessage()
	{
	}
	Agent* trainAgent;
};
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
			headwaySec(0) {
	}
	;
	std::string scheduleId;
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
struct cmp_trip_start : public std::less<TrainTrip*>
{
	bool operator()(const TrainTrip* x, const TrainTrip* y) const
	{
		if ((!x) || (!y)) {
			return false;
		}

		return x->getStartTime() > y->getStartTime();
	}
};

/**C++ static constructors*/
class TripStartTimePriorityQueue : public std::priority_queue<TrainTrip*, std::vector<TrainTrip*>, cmp_trip_start>
{
};
template<typename PERSON>
class TrainController: public sim_mob::Agent
{
	BOOST_STATIC_ASSERT_MSG(
			(boost::is_base_of<sim_mob::Person, PERSON>::value),
			"PERSON must be a descendant of sim_mob::Person"
	);
public:
	explicit TrainController(int id = -1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered);
	virtual ~TrainController();
	typedef boost::unordered_map<const Station*, Agent*> StationAgentsMap;

public:
	/**
	 * initialize the train controller
	 */
	void initTrainController();
	/**
	 * update nearby previous and next train location
	 */

	void InitializeTrainIds(std::string lineId);
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
	/**
	 * finds the station Agent from station name.
	 * @param nameStation is the name of station
	 * @returns pointer to station agent corresponding to station
	 */
	static Agent* getAgentFromStation(const std::string& nameStation);
	/**
	 * adds station agent to the static allStationAgents
	 */
	static void registerStationAgent(const std::string& nameStation, Agent* stationAgent);
	/**
	 * Prints the road network to the output file
	 * @param network the road network
	 */
	void printTrainNetwork(const std::string& outFileName) const;
	/**
	 * get platform from line id and station name
	 * @param lineId is the line id
	 * @param stationName is the station name
	 */
	static Platform* getPlatform(const std::string& lineId, const std::string& stationName);
	/* get vector of blocks from lineId*/
	std::vector<Block*> GetBlocks(std::string lineId);
	/* get station entity from ID*/
	Station * GetStationFromId(std::string stationId);
	std::string GetOppositeLineId(std::string lineId);


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
	 * unregister child item from children list
	 */
	virtual void unregisterChild(Entity* child);
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
	/**
	 * handle messages
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);



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



	/** function to get blocks for particular train route*/

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
	/**
	 * compose the train blocks with poly-line
	 */
	void composeBlocksAndPolyline();
	/**
	 * compose trips from schedules
	 */
	void composeTrainTrips();
	/**
	 * print out blocks information
	 * @param out is output stream
	 */
	void printBlocks(std::ofstream& out) const;
	/**
	 * print out platforms information
	 * @param out is output stream
	 */
	void printPlatforms(std::ofstream& out) const;
	/**
	 * get Train Id
	 * @param lineId is refer to train line
	 */
	int getTrainId(const std::string& lineId);

	/** get the station entity from its Id */
	void TerminateTrainService( std::string lineId);

	bool IsServiceTerminated(std::string lineId);

    /** gives opposite line Id */

private:
	/** global static bus stop agents lookup table*/
	static StationAgentsMap allStationAgents;
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
	/**the map from id to trip*/
	std::map<std::string, TripStartTimePriorityQueue> mapOfIdvsTrip;
	/**the map from name to the station*/
	std::map<std::string, Station*> mapOfIdvsStations;
	/**the map from id to polyline object*/
	std::map<unsigned int, PolyLine*> mapOfIdvsPolylines;
	/**	buses waiting to be added to the simulation, prioritized by start time.*/
	StartTimePriorityQueue pendingChildren;
	/**last train id*/
	std::map< std::string,unsigned int>mapOfNoAvailableTrains;
    /* holds the status of train service ...true if terminated false if running*/
	std::map<std::string,bool>mapOfTrainServiceTerminated;

	int lastTrainId;


	/**reused train Ids*/
	std::map<std::string, std::vector<int>> recycleTrainId;
private:
	static TrainController* pInstance;
};

}
/* namespace sim_mob */

#define _CLASS_TRAIN_CONTROLLER_FUNCTIONS
#include "TrainController.cpp"
#endif
