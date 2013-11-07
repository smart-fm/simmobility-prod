//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * FMODController.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef FMODCONTROLLER_HPP_
#define FMODCONTROLLER_HPP_
#include "FMOD_Client.hpp"
#include "ParkingCoordinator.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

namespace FMOD
{

struct Request;

/**
  * provide collaboration with FMOD simulator
  */
class FMOD_Controller : public sim_mob::Agent {
public:

	virtual ~FMOD_Controller();

    /**
      * insert a new trip chain with FMOD mode into collection
      * @param personID is key to identify a person .
      * @param item is a trip chain
      * @return true if inserting successfully .
      */
	bool insertFmodItems(const std::string& personID, TripChainItem* item);

    /**
      * assign settings to controller
      * @param ipadress is IP address  .
      * @param port is IP port
      * @param updateTiming is the frequence of updating status to FMOD simulator
      * @param mapFile is the road network filename in simulation
      * @param blockingTime is waiting time in blocking mode
      * @return void .
      */
	void settings(std::string ipadress, int port, int updateTiming, std::string mapFile, int blockingTime) { this->ipAddress=ipadress; this->port=port; this->updateTiming=updateTiming; this->mapFile=mapFile; this->waitingseconds=blockingTime;}

    /**
      * register a singleton object to system
      * @param id is used to identify this object  .
      * @param mtxStrat is for thread safety protection
      * @return void .
      */
	static void registerController(int id, const MutexStrategy& mtxStrat);

    /**
      * retrieve a singleton object
      * @return a pointer to FMOD_Controller .
      */
	static FMOD_Controller* instance();

    /**
      * check whether or not the instance already existed
      * @return true if existed .
      */
	static bool instanceExists();

    /**
      * connect to FMOD simulator
      * @return true if successfully .
      */
	bool connectFmodService();

    /**
      * break down communication to FMOD simulator
      * @return void .
      */
	void stopClientService();

    /**
      * initialize states of FMOD controller
      * @return void .
      */
	void initialize();

private:
	//record links travel times
	std::map<sim_mob::Link*, double> linkTravelTimes;
	//record all trip chain items with FMOD mode
	std::map<std::string, TripChainItem*> allItems;
	//record all FMOD requests
	std::map<Request*, TripChainItem*> allRequests;
	//record all passengers information
	std::vector<Agent*> allPersons;
	//record dispatching drivers
	std::vector<Agent*> allDrivers;
	// keep all children agents to communicate with it
	std::vector<Agent*> allChildren;
	//identify whether or not communication is created
	bool isConnectFmodServer;

protected:
	//override from the class agent, provide initilization chance to sub class
	virtual bool frame_init(timeslice now);
	//override from the class agent, do frame tick calculation
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	//override from the class agent, print output information
	virtual void frame_output(timeslice now);

	//May implement later
	virtual void load(const std::map<std::string, std::string>& configProps){}
	//Signals are non-spatial in nature.
	virtual bool isNonspatial() { return true; }
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList){}
	//override from the class agent, provide a chance to clear up a child pointer when it will be deleted from system
	virtual void unregisteredChild(Entity* child);
	explicit FMOD_Controller(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id), connectPoint(new FMOD_Client(ioService)), frameTicks(0), waitingseconds(10){}

private:
    /**
      * process messages received from FMOD simulator in non-blocking mode
      * @param now is current frame tick
      * @return void .
      */
	void processMessages(timeslice now);

    /**
      * process messages received from FMOD simulator in blocking mode
      * @param now is current frame tick
      * @return void .
      */
	void processMessagesInBlocking(timeslice now);

    /**
      * generate request messages on current frame tick
      * @param now is current frame tick
      * @return message list including messages which will be sent to FMOD simulator .
      */
	MessageList generateRequest(timeslice now);

    /**
      * handle offer message when a offer is received from FMOD simulator
      * @param msg is message content in json format
      * @return message list including confirmation messages which will be sent to FMOD simulator .
      */
	MessageList handleOfferMessage(const std::string& msg);

    /**
      * handle confirmation message when a confirmation message is received from FMOD simulator
      * @param msg is message content in json format
      * @return message list including confirmation messages which will be sent to FMOD simulator .
      */
	MessageList handleConfirmMessage(const std::string& msg);

    /**
      * handle schedule message when a schedule message is received from FMOD simulator
      * @param msg is concrete message content in json format
      * @return void.
      */
	void handleScheduleMessage(const std::string& msg);

    /**
      * handle vehicle initialization message when a initialization message is received from FMOD simulator
      * @param msg is concrete message content in JSON format
      * @return void.
      */
	void handleVehicleInit(const std::string& msg);

    /**
      * update simulation status including link travel time and vehicles position to FMOD simulator in non blocking mode.
      * @param now is current frame tick
      * @return void.
      */
	void updateMessages(timeslice now);

    /**
      * update simulation status including link travel time and vehicles position to FMOD simulator in blocking mode.
      * @param now is current frame tick
      * @return void.
      */
	void updateMessagesInBlocking(timeslice now);

    /**
      * collect vehicles stops information and generate messages to FMOD simulator.
      * @return void.
      */
	MessageList collectVehStops();

    /**
      * collect current vehicle position information and generate message to FMOD simulator.
      * @return void.
      */
	MessageList collectVehPos();

    /**
      * collect link travel information and generate message to FMOD simulator.
      * @return void.
      */
	MessageList collectLinkTravelTime();

    /**
      * dispatch a pending agent to system by its start time.
      * @param now is current frame tick
      * @return void.
      */
	void dispatchPendingAgents(timeslice now);

private:
    /**
      * collect person information from trip chain.
       * @return void.
      */
	void collectPerson();

    /**
      * collect request information which will send to FMOD simulator.
      * @return void.
      */
	void collectRequest();

private:
	FmodClientPtr connectPoint;
	std::string ipAddress;
	std::string mapFile;
	int port;
	int updateTiming;
	int frameTicks;
	int waitingseconds;

private:
	struct travelTimes
	{
	public:
		unsigned int linkTravelTime;
		unsigned int agentCount;

		travelTimes(unsigned int linkTravelTime, unsigned int agentCount)
		: linkTravelTime(linkTravelTime), agentCount(agentCount) {}
	};

	std::map<const sim_mob::Link*, travelTimes> LinkTravelTimesMap;

	//when vehicle initialize and pack, it will store to this structure
	ParkingCoordinator parkingCoord;

private:
	static FMOD_Controller* pInstance;
	static boost::asio::io_service ioService;

};

}

} /* namespace sim_mob */
#endif /* FMOD_Controller_HPP_ */
