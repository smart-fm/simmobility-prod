//Copyright (c) <2016> Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once
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

/*
 * This structure is an entity which stores the attributes of a reset block speed action
 */
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

struct ResetBlockAccelerations
{
	std::string startStation;
	std::string endStation;
	double accLimit;;
	double defaultAcceleration;
	std::string startTime;
	std::string endTime;
	std::string line;
	bool accelerationReset=false;
};

/**
 * This structure holds the attributes of disruption to be performed by service controller Api call
 */
struct DisruptionEntity
{
	std::string startStation="";
	std::string endStation="";
	std::string disruptionTime="";
};

/**
 * the structure to store the train route
 */
struct TrainRoute
{
	TrainRoute() :
			sequenceNo(0), blockId(0)
	{
	}
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
struct TransferTimeInPlatform
{
	TransferTimeInPlatform() :
			transferedTimeSec(0)
	{
	}
	std::string stationNo;
	std::string platformFirst;
	std::string platformSecond;
	int transferedTimeSec;
};





struct CompTripStart : public std::less<TrainTrip*>
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
class TripStartTimePriorityQueue : public std::priority_queue<TrainTrip*, std::vector<TrainTrip*>, CompTripStart>
{
};

/*
 * This class loads train schedules,train routes,platforms,stations
 * It also receives to do actions from service controller
 * This class is also an interface to access non changing entities like platforms,train routes
 * blocks,stations.
 * Author: zhang huai peng
 */
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
	 * Interface to initialize train ids for a particular line
	 * @param lineId is the id of the line
	 */
	void InitializeTrainIds(std::string lineId);

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
	 * @param nameStation is the name of the station
	 * @stationAgent is the pointer to stationAgent
	 */
	static void registerStationAgent(const std::string& nameStation, Agent* stationAgent);

	/**
	 * Prints the road network to the output file
	 * @param network the road network
	 */
	void printTrainNetwork(const std::string& outFileName) const;

	/**
	 * Assigns the reset block speed entities,to store the speed reset information
	 * And their timing
	 * @param ResetBlockSpeeds is the structure for a particular reset
	 */
	void assignResetBlockSpeeds(ResetBlockSpeeds resetSpeedBlocks);

	/**
	 * Assigns the reset block acceleration entities,to store the speed reset information
	 * And their timing
	 * @param ResetAccelerations is the structure for a particular reset
	 */
	void assignResetBlockAccelerations(ResetBlockAccelerations resetSpeedBlocks);

	/**
	 * adds to list of Active trains in Line when new train is created and is running on the route
	 * @param lineId is the id of the line where the train is to be added
	 * @param driver is the pointer to the train driver
	 */
	void addToListOfActiveTrainsInLine(std::string lineId,Role<PERSON> *driver);

	/**
	 * Removes from list of Active Trains when train is sent to depot
	 * @param lineId is the id of the line where the train is to be added
	 * @param driver is the pointer to the train driver
	 */
	void removeFromListOfActiveTrainsInLine(std::string lineId,Role<PERSON> *driver);

	/**
	 * returns the train route of blocks for particular line
	 * @param lineId is the id of the line
	 * @route is the reference to the route which is to be populated with the blocks
	 */
	bool getTrainRoute(const std::string& lineId, std::vector<Block*>& route) const;

	/**
	 * returns train platforms of particular line
	 * @param lineId is the id of the line
	 * @param platforms is the reference to the platforms vector where the platforms are to be populated
	 * @returns true if the line is found
	 */
	bool getTrainPlatforms(const std::string& lineId, std::vector<Platform*>& platforms) const;

	/**
	 * returns the train lines connecting the two stations directly
	 * @param src is the name of the start staion
	 * @param dest is the name of end station
	 * @return the lines present the stations
	 */
	std::vector<std::string> getLinesBetweenTwoStations(std::string src,std::string dest);

	/**
	 * returns the active trains in a particular line
	 * @param lineId is the id of the line
	 * @return the vector of active trains on that line
	 */
	typename std::vector <Role<PERSON>*> getActiveTrainsForALine(std::string lineID);

	/**
	 * This gives the next platform from current platform of particular line
	 * @param lineId is the id of the line
	 * @param platformNo is the name of the platform
	 * @return the next platform after the one specified
	 */
	TrainPlatform  getNextPlatform(std::string platformNo,std::string lineId);

	/**
	 * Returns the platform of a particular station of a particular line
	 * @param lineId is the id of the line
	 * @param stationName is the name of the station
	 * @return the platform of a particular station of a particular line
	 */
	static Platform* getPlatform(const std::string& lineId, const std::string& stationName);

	/**
	 * This returns the pointer to platform from platform name
	 * @param platfromNo is the name of the platform
	 * @return the required platform
	 */
	Platform* getPlatformFromId(std::string platformNo);

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

	/**
	 * gets vector of blocks from lineId
	 * @param lineId is the id of the line
	 * @return the vector of blocks
	 */
	std::vector<Block*> getBlocks(std::string lineId);

	/**
	 * get station entity from stationId
	 * @param stationId is the id of the station
	 * @return the pointer to the station
	 */
	Station * getStationFromId(std::string stationId);

	/**
	 * This gives the opposite lineId of a particular line
	 * @param lineId is the id of the line specified
	 * @param the opposite line id required
	 */
	std::string getOppositeLineId(std::string lineId);

	/**
	 * This returns the block from block id
	 * @param blockId is the id of the block
	 * @return is the pointer to the block
	 */
	Block * getBlock(int blockId);

	/**
	 * This pulls out the train from InActive pool
	 * to add to the Active pool
	 * The Train is deleted from InActive pool
	 * And Added to Active pool
	 * @param lineId is the id of the line
	 * @return the id of the train pulled out from inactive pool and added to active pool
	 */
	int pullOutTrainFromInActivePool(std::string lineId);

	/*
	 * This deletes the train from  active pool
	 * @param lineId is the id of the line
	 * @return the id of the train deleted
	 */
	int deleteTrainFromActivePool(std::string lineId);

	/**
	 * adds the train to active pool
	 * @param lineId is the id of the line
	 * @param trainId is the id of the train
	 */
	void addTrainToActivePool(std::string lineId,int trainId);

	/**
	 * adds the train to active pool
	 * @param lineId is the id of the line
	 * @param trainId is the id of the train
	 */
	void addTrainToInActivePool(std::string lineId,int trainId);

	/**
	 * This deletes the train from  active pool
	 * @param lineId is the id of the line
	 * @return the id of the train deleted
	 */
	int deleteTrainFromInActivePool(std::string lineID);

	/**
	 * This pushes the train to Active Pool from  InActive pool
	 * The train is deleted from Active pool and added to InActive Pool
	 * @param trainId is the id of the train
	 * @param lineId is the id of the line
	 */
	void pushTrainIntoInActivePool(int trainId,std::string lineID);

	/* Returns the minimum dwell time of a train depending on type of station
	 * If its normal station then dwell time is 20 secs minimum
	 * If its interchange then 40 secs minimum
	 * If its terminal station(start and end station of line) then 60 secs minimum
	 * @param stationNo is the name of the station
	 * @param lineId is the id of the line
	 * @return the minimum dwell time
	 */
	double getMinDwellTime(std::string stationNo,std::string lineId) const;

	/* Returns the maximum dwell time of a train irrespective of type of station
	 * For any station it is 120 secs
	 * @return the maximum dwell time
	 */
	double getMaximumDwellTime(std::string lineId) const;

	/**
	 * just checks if the station is the first station for a given line
	 * @param lineId is the id of the line
	 * @param platform is the pointer to the platform of the station
	 * @return true if it is the first platform
	 */
	bool isFirstStation(std::string lineId,Platform *platform) const;

	/*
	 * This terminated the train service for entire train line
	 * Stops the future dispatch of trains
	 * The trains reach the nearest next platform where the they alight all passengers
	 * and then the train returns to depot
	 * @param lineId is the id of the line
	 */
	void terminateTrainService( std::string lineId);

	/**
	 * This returns the vector of platforms from starting station on a particular line
	 * @param lineId is the id of the line
	 * @param startStaion is the name of the station from where the platforms along the line are needed
	 * @return the vector of platforms from the start station
	 */
	std::vector<Platform*> getPlatforms(std::string lineId,std::string startStation);

	/**
	 * composes unscheduled train trip at a particular time stamp ,for a particular line
	 * The trip can be starting somewhere at between platform not necessarily at start station
	 * @param lineId is the id of the line
	 * @param startTie is the start time of the trip
	 * @param startStation is the start station of the trip
	 */
	void composeTrainTripUnScheduled(std::string lineId,std::string startTime,std::string startStation);

	/**
	 * gets the list of disrupted platforms for service controller caused by service controller
	 * @return the map of line and disrupted platforms
	 */
	std::map<std::string,std::vector<std::string>> getDisruptedPlatforms_ServiceController();

	/**
	 * gets the list of uturn platforms for all the train lines
	 * @return map  of line and vector of uturn platforms
	 */
	std::map<std::string,std::vector<std::string>>& getUturnPlatforms();

    /**
     * performs disruptiopn.sets the disrupted platform list
     * @param startStation is the name of start station
     * @param end station is the name of end station
     * @param disruptionTime is the time specified for disruption
     */
    void performDisruption(std::string startStation,std::string endStation,timeslice now,std::string disruptionTime);

    /**
	 * sets the disrupted platform list (done by service controller)
	 * @param startStation is the name of start station
	 * @param end station is the name of end station
	 * @param disruptionTime is the time specified for disruption
	 */
    void setDisruptedPlatforms(std::string startStation,std::string endStation,std::string lineID);

    /**
     * Sets the disruption params
     * @param startStation is the name of start station
     * @param endStaion is the name of endStation
     * @param time is the time for disruption
     */
    void setDisruptionParams(std::string startStation,std::string endStation,std::string time);

    /**
     * gets the list of platforms between two stations for a particular line
     * @param startStation is the name of start station
     * @param endStaion is the name of endStation
     * @return the platforms between the two stationss
     */
    std::vector<std::string> getPlatformsBetweenStations(std::string lineId,std::string startStation,std::string endStation);

    /**
     * checks if one platform is another
     * @param firstplatform is the name of first platform
     * @param secondplatform is the name of second platform
     * @lineId is the id of the line
     * @return true if it is the first platform
     */
    bool isPlatformBeforeAnother(std::string firstPlatfrom ,std::string secondPlatform,std::string lineId);

    /**
     * clears the disruption on the platforms and deletes the disrupted platforms list
     * @lineId where the disruption is to be cleared
     */
    void clearDisruption(std::string lineId);

    /**
     * checks if the train service is terminated or not
     * @param lineid is the id of the line
     * @return whether the train service is terminated
     */
    bool isServiceTerminated(std::string lineId);

    /**
     * loads the opposite lines of the train lines
     */
    void loadOppositeLines();

    /**
     * loads the availabilities of the train from database
     * Assigns the id range of the trains
     */
    void loadTrainAvailabilities();

    /**
     * loads the uturn platforms from database
     */
    void loadUTurnPlatforms();

    /**
     * checks if a particular platform is a disrupted platform
     * @param platformName is the name of the platform
     * @param lineId is the id of the line
     * @return bool true if it is the disrupted platform
     */
    bool isDisruptedPlatform(std::string platformName,std::string lineId);

    /**
     * adds the train into a list of trains which are to be pushed into inactive pool after completion
     * @param trainId is the id of the train to be pushed to inactive pool
     * @lineId is the id of the line in whose pool it has to be pushed
     */
    void pushToInactivePoolAfterTripCompletion(int trainId,std::string lineId);

    /**
     * checks if a particular platform is a uturn platform
     * @param platformName is the name of the platform
     * @lineId is the id of the line
     * @return true if it is uturn platform
     */
    bool isUturnPlatform(std::string platformName,std::string lineId);

    /**
     * This gets the uturn platform available starting from the platform specified
     * @param platformName is the name of the platform to start from
     * @param lineId is the id of the line
     * @return the next uturn platform
     */
    std::string getNextUturnPlatform(std::string platformName,std::string lineId);

    /**
     * resets the speed of the blocks
     * @param now is the current relative time in simulation
     */
    void resetBlockSpeeds(DailyTime now);

    /**
	 * resets the acceleration of the blocks
	 * @param now is the current relative time in simulation
	 */
    void resetBlockAccelerations(DailyTime now);

    /**
     * checks if the given platform is the last platform of a given line
     * @param platformNo is the name of the platform
     * @param lineId is the id of the line
     * @return bool true if it is the last platform
     */
    void handleTrainReturnAfterTripCompletition(PERSON *person);
    bool isTerminalPlatform(std::string platformNo,std::string lineId);
    const std::vector<double> getNumberOfPersonsCoefficients(const Station *station,const Platform *platformName) const;
	void changeNumberOfPersonsCoefficients(std::string stationName,std::string platformName,double coefficientA,double coefficientB,double coefficientC);

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
	 * function to get blocks for particular train route
	 */
	void loadTrainRoutes();

	/**
	 * the function to load train platforms from DB
	 */
	void loadTrainPlatform();

	/**
	 * loads the properties of train lines
	 */
	void loadTrainLineProperties();

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

	/**
	 * resets the speed limits of the stretch of blocks
	 * @param now is the current time slice
	 */

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
    /** holds the status of train service ...true if terminated false if running*/
	std::map<std::string,bool>mapOfTrainServiceTerminated;
	/** holds the disrupted platforms per line */
    std::map<std::string,std::vector<std::string>> disruptedPlatformsNamesMap_ServiceController;
    /** entities which hold information for resetting the block speeds*/
	std::vector<ResetBlockSpeeds> resetSpeedBlocks;
	/** entities which hold information for resetting the block accelerations*/
	std::vector<ResetBlockAccelerations> resetAccelerationBlocks;
	/** map which saves the default accelerations when the accelerations are reset by service controller */
	std::map<std::string,std::map<int, double>> blockIdAcceleration;
	/** map which saves the default block speeds when the accelerations are reset by service controller */
	std::map<std::string,std::map<int, double>> blockIdSpeed;
	/** map that holds the active train lines in the system */
	std::map<std::string, std::vector <Role<PERSON>*>> mapOfLineAndTrainDrivers;
	/** holds the train ids in inactive pool per train line*/
	std::map<std::string,std::vector<int>> mapOfInActivePoolInLine;
	/** holds the train ids to be pushed to inactive pool after the completion of their trip*/
	std::map<std::string,std::vector<int>> trainsToBePushedToInactivePoolAfterTripCompletion;
	/** lock of "mapOfLineAndTrainDrivers" */
	mutable boost::mutex activeTrainsListLock;
	/** lock of "mapOfTrainServiceTerminated" */
	mutable boost::mutex terminatedTrainServiceLock;
	/** map which holds the ids range for train lines*/
	std::map<std::string,std::list<int>> mapOfTrainMaxMinIds;
	/** map which holds the opposite pairs of train lines eg "NE_1,NE_2","EW_1,EW_2"*/
	std::map<std::string,std::string> mapOfOppositeLines;
	/** map which holds the uturn platforms for evry train line */
	std::map<std::string,std::vector<std::string>> mapOfUturnPlatformsLines;
	std::map<const Station*,std::map<const Platform*,std::vector<double>>> mapOfCoefficientsOfNumberOfPersons;

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

