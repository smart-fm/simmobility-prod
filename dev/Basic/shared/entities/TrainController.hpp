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
#include "entities/roles/Role.hpp"
#include "conf/RawConfigParams.hpp"


using namespace sim_mob::messaging;
namespace sim_mob {

const Message::MessageType MSG_TRAIN_BACK_DEPOT = 7300001;
const Message::MessageType MSG_REGISTER_BLOCKS_CHANGE_SPEEDLIMIT = 84099002;
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

struct ResetBlockSpeeds
{
std::string startStation;
std::string endStation;
double speedLimit;;
double defaultSpeed;
std::string startTime;
std::string endTime;
std::string line;
bool speedReset=false;
};

struct DisruptionEntity
{
	std::string startStation="";
	std::string endStation="";
	std::string disruptionTime="";
};

class ResetSpeedMessage: public messaging::Message
{
public:
	ResetSpeedMessage(ResetBlockSpeeds resetSpeedBlock)
	{
		ResetSpeedBlock = resetSpeedBlock;
	}
	virtual ~ResetSpeedMessage()
	{
	}
	ResetBlockSpeeds ResetSpeedBlock;
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

	/* Assigns the reset block speed entities,to store the speed reset information
	 * And the timing
	 */
	void AssignResetBlocks(ResetBlockSpeeds resetSpeedBlocks);

	/* adds to list of Active trains in Line when new train is created*/
	void AddToListOfActiveTrainsInLine(std::string lineId,Role<PERSON> *driver);
	/* Removes from list of Active Trains when train is sent to depot */
	void RemoveFromListOfActiveTrainsInLine(std::string lineId,Role<PERSON> *driver);

	/* returns the train route of blocks for particular line */
	bool getTrainRoute(const std::string& lineId, std::vector<Block*>& route);

	/* gives train platforms of particular line */
	bool getTrainPlatforms(const std::string& lineId, std::vector<Platform*>& platforms);
	std::vector<std::string> GetLinesBetweenTwoStations(std::string src,std::string dest);
	typename std::vector <Role<PERSON>*> GetActiveTrainsForALine(std::string lineID);

	/*This gives the next platform from current platform of particular line */
	TrainPlatform  GetNextPlatform(std::string platformNo,std::string lineID);


	static Platform* getPlatform(const std::string& lineId, const std::string& stationName);

	/*
	 * This the pointer to platform from pltaform name
	 */
	Platform* GetPlatformFromId(std::string platformNo);
	/**
	 * check whether platform is existed or not
	 * @param stationAgent is a pointer to station agent
	 * @param platformName is the name of platform you want to check
	 * @return true if find corresponding platform
	 */
	static bool checkPlatformIsExisted(const Agent* stationAgent, const std::string& platformName);
	/**
	 * get previous platform for a particular line
	 * @param lineId is a train line
	 * @param curPlatform is current platform name
	 * @return null if do't have previous platform
	 */
	static Platform* getPrePlatform(const std::string& lineId, const std::string& curPlatform);
	/* get vector of blocks from lineId*/
	std::vector<Block*> GetBlocks(std::string lineId);
	/* get station entity from ID*/
	Station * GetStationFromId(std::string stationId);
	/*
	 * This gives the opposite lineId of a particular line
	 *
	 */
	std::string GetOppositeLineId(std::string lineId);
	Block * GetBlock(int blockId);

	/* Pull out the train from InActive pool
	 * To add to the Active pool
	 *The Train is deleted from InActive pool
	 * And Added to Active pool
	 */

	int pullOutTrainFromInActivePool(std::string lineID);

	/*
	 * This deletes the train from  active pool
	 */
	int DeleteTrainFromActivePool(std::string lineID);

	/* adds the train to active pool */
	void AddTrainToActivePool(std::string lineId,int trainId);

	/* adds the train to inactive pool */
	void AddTrainToInActivePool(std::string lineId,int trainId);

	/* This deletes the train from  inactive pool
	 *
	 */
	int DeleteTrainFromInActivePool(std::string lineID);

	/*
	 * This pushes the train to Active Pool from  InActive pool
	 *The train is deleted from Active pool and added to InActive Pool
	 */
	void pushTrainIntoInActivePool(int trainId,std::string lineID);

	/* Returns the minimum dwell time of a train depending on type of station
	 * If its normal station then dwell time is 20 secs minimum
	 * If its interchange then 40 secs minimum
	 * If its terminal station(start and end station of line) then 60 secs minimum
	 */
	double GetMinDwellTime(std::string stationNo,std::string lineId);

	/* just checks if the station is the first station for a given line */
	bool IsFirstStation(std::string lineId,Platform *platform);
	/*
	 * This terminated the train service for entire train line
	 * Stops the future dispatch of trains
	 * The trains reach the nearest next platform where the they alight all passengers
	 * and  then the train returns to depot
	 */
	void TerminateTrainService( std::string lineId);
	std::vector<Platform*> GetPlatforms(std::string lineId,std::string startStation);
	/* composes unscheduled train trip at a particular time stamp ,for a particular line
	 * The trip can be starting somewhere at between platform not necessary at start station
	 * The distance traveled is fast forwarded till that platform
	 * Corressponding block and polyline of those platforms are adjusted
	 */
	void composeTrainTripUnScheduled(std::string lineId,std::string startTime,std::string startStation);
	/* gets the list of disrupted platforms for service controller caused by service controller */
	std::map<std::string,std::vector<std::string>> GetDisruptedPlatforms_ServiceController();
	/*Gets all the train ids for active trains */
    std::vector<int> GetActiveTrainIds();
    /* performs disruptiopn.sets the disrupted platform list */
    void PerformDisruption(std::string startStation,std::string endStation,timeslice now,std::string disruptionTime);
    void SetDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID);
    void SetDisruptionParams(std::string startStation,std::string endStation,std::string time);
    /* gets the list of platforms between two stations for a particular line*/
    std::vector<std::string> GetPlatformsBetweenStations(std::string lineId,std::string startStation,std::string endStation);
    /* checks if train service is terminated for a particular line or not */
    bool IsServiceTerminated(std::string lineId);

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
	//bool getTrainRoute(const std::string& lineId, std::vector<Block*>& route);
	/**
	 * get platform list for a particular line
	 * @param lineId is line id
	 * @param platforms are the list of platforms
	 * @return true if successfully get the list of platforms
	 */
	//bool getTrainPlatforms(const std::string& lineId, std::vector<Platform*>& platforms);
	/**
	 * handle messages
	 */
	virtual void HandleMessage(messaging::Message::MessageType type, const messaging::Message& message);
	/**
	 * Inherited from EventListener.
	 * @param eventId
	 * @param ctxId
	 * @param sender
	 * @param args
	 */
	virtual void onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args);
	/**
	 * change train trip when disruption happen
	 * @param trip is a pointer to the train trip
	 * @param params is a pointer to disruption structure
	 */
	void changeTrainTrip(sim_mob::TrainTrip* trip, sim_mob::DisruptionParams* params);

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

	void composeTrainTrip();
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





	void resetBlockSpeeds(timeslice now);






    /** gives opposite line Id */

private:
	/**recording disruption information*/
	boost::shared_ptr<DisruptionParams> disruptionParam;
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
	std::map<std::string,std::vector<TrainTrip*>> mapOfIdvsUnsheduledTrips;
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
    std::map<std::string,std::vector<std::string>> disruptedPlatformsNamesMap_ServiceController;
	std::vector<ResetBlockSpeeds> resetSpeedBlocks;
	std::map<int, double> blockIdSpeed;
	std::map<std::string, std::vector <Role<PERSON>*>> mapOfLineAndTrainDrivers;
	std::map<std::string,std::vector<int>> mapOfInActivePoolInLine;
	std::vector<int> trainsToBePushedToInactivePoolAfterTripCompletion;
	mutable boost::mutex activeTrainsListLock;
	mutable boost::mutex terminatedTrainServiceLock;




	//std::map<std::string, std::vector <int>> mapOfLineAndTrainDrivers;
	//sim_mob::Role<PERSON>*
    //std::map< std::vector<Block*> blockVectorOfRessetedSpeeds>;


	int lastTrainId;


	/**reused train Ids*/
	std::map<std::string, std::vector<int>> recycleTrainId;
private:
	static TrainController* pInstance;
	mutable boost::mutex activePoolLock;
	mutable boost::mutex inActivePoolLock;
	bool disruptionPerformed=false;
	 int maxTripId;
	DisruptionEntity disruptionEntity;

};

}
/* namespace sim_mob */

#define _CLASS_TRAIN_CONTROLLER_FUNCTIONS
#include "TrainController.cpp"
#endif
