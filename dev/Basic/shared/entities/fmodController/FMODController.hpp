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
#include "TCPClient.hpp"
#include "ParkingCoordinator.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

namespace FMOD
{

struct Request;
class FMODController : public sim_mob::Agent {
public:

	virtual ~FMODController();

    /**
      * insert a new trip chain with FMOD mode into collection
      * @param personID is key to identify a person .
      * @param item is a trip chain
      * @return true if inserting successfully .
      */
	bool InsertFMODItems(const std::string& personID, TripChainItem* item);

    /**
      * assign settings to controller
      * @param ipadress is IP address  .
      * @param port is IP port
      * @param updateTiming is the frequence of updating status to FMOD simulator
      * @param mapFile is the road network filename in simulation
      * @param blockingTime is waiting time in blocking mode
      * @return void .
      */
	void Settings(std::string ipadress, int port, int updateTiming, std::string mapFile, int blockingTime) { this->ipAddress=ipadress; this->port=port; this->updateTiming=updateTiming; this->mapFile=mapFile; this->waitingseconds=blockingTime;}

    /**
      * register a singleton object to system
      * @param id is used to identify this object  .
      * @param mtxStrat is for thread safety protection
      * @return void .
      */
	static void RegisterController(int id, const MutexStrategy& mtxStrat);

    /**
      * retrieve a singleton object
      * @return a pointer to FMODController .
      */
	static FMODController* Instance();

    /**
      * check whether or not the instance already existed
      * @return true if existed .
      */
	static bool InstanceExists();

    /**
      * connect to FMOD simulator
      * @return true if successfully .
      */
	bool ConnectFMODService();

    /**
      * break down communication to FMOD simulator
      * @return void .
      */
	void StopClientService();

    /**
      * initialize states of FMOD controller
      * @return void .
      */
	void Initialize();

private:
	//record links travel times
	std::map<sim_mob::Link*, double> linkTravelTimes;
	//record all trip chain items with FMOD mode
	std::map<std::string, TripChainItem*> all_items;
	//record all FMOD requests
	std::map<Request*, TripChainItem*> all_requests;
	//record all passengers information
	std::vector<Agent*> all_persons;
	//record dispatching drivers
	std::vector<Agent*> all_drivers;
	// keep all children agents to communicate with it
	std::vector<Agent*> all_children;
	//identify whether or not communication is created
	bool isConnectFMODServer;

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
	explicit FMODController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id), connectPoint(new TCPClient(io_service)), frameTicks(0), waitingseconds(10){}

private:
    /**
      * process messages received from FMOD simulator in non-blocking mode
      * @param now is current frame tick
      * @return void .
      */
	void ProcessMessages(timeslice now);

    /**
      * process messages received from FMOD simulator in blocking mode
      * @param now is current frame tick
      * @return void .
      */
	void ProcessMessagesInBlocking(timeslice now);

    /**
      * generate request messages on current frame tick
      * @param now is current frame tick
      * @return message list including messages which will be sent to FMOD simulator .
      */
	MessageList GenerateRequest(timeslice now);

    /**
      * handle offer message when a offer is received from FMOD simulator
      * @param msg is message content in json format
      * @return message list including confirmation messages which will be sent to FMOD simulator .
      */
	MessageList HandleOfferMessage(std::string& msg);

    /**
      * handle confirmation message when a confirmation message is received from FMOD simulator
      * @param msg is message content in json format
      * @return message list including confirmation messages which will be sent to FMOD simulator .
      */
	MessageList HandleConfirmMessage(std::string& msg);

    /**
      * handle schedule message when a schedule message is received from FMOD simulator
      * @param msg is concrete message content in json format
      * @return void.
      */
	void HandleScheduleMessage(std::string& msg);

    /**
      * handle vehicle initialization message when a initialization message is received from FMOD simulator
      * @param msg is concrete message content in JSON format
      * @return void.
      */
	void HandleVehicleInit(std::string& msg);

    /**
      * update simulation status including link travel time and vehicles position to FMOD simulator in non blocking mode.
      * @param now is current frame tick
      * @return void.
      */
	void UpdateMessages(timeslice now);

    /**
      * update simulation status including link travel time and vehicles position to FMOD simulator in blocking mode.
      * @param now is current frame tick
      * @return void.
      */
	void UpdateMessagesInBlocking(timeslice now);

    /**
      * collect vehicles stops information and generate messages to FMOD simulator.
      * @return void.
      */
	MessageList CollectVehStops();

    /**
      * collect current vehicle position information and generate message to FMOD simulator.
      * @return void.
      */
	MessageList CollectVehPos();

    /**
      * collect link travel information and generate message to FMOD simulator.
      * @return void.
      */
	MessageList CollectLinkTravelTime();

    /**
      * dispatch a pending agent to system by its start time.
      * @param now is current frame tick
      * @return void.
      */
	void DispatchPendingAgents(timeslice now);

private:
    /**
      * collect person information from trip chain.
       * @return void.
      */
	void CollectPerson();

    /**
      * collect request information which will send to FMOD simulator.
      * @return void.
      */
	void CollectRequest();

private:
	TCPSessionPtr connectPoint;
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
	static FMODController* pInstance;
	static boost::asio::io_service io_service;

};

}

} /* namespace sim_mob */
#endif /* FMODCONTROLLER_HPP_ */
