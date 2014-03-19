//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AMODController.hpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Max
 */

#ifndef AMODController_HPP_
#define AMODController_HPP_

#include "AMODClient.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

namespace AMOD {

class AMODController : public sim_mob::Agent{
public:
	virtual ~AMODController();

	/**
	  * connect to AMOD simulator
	  * @return true if successfully .
	  */
	bool connectAmodService();

	/**
	  * register a singleton object to system
	  * @param id is used to identify this object  .
	  * @param mtxStrat is for thread safety protection
	  * @return void .
	  */
	static void registerController(int id, const MutexStrategy& mtxStrat);
	/**
	  * handle vehicle initialization message when a initialization message is received from AMOD simulator
	  * @param msg is concrete message content in JSON format
	  * @return void.
	  */
	void handleVehicleInit(const std::string& msg);
	/**
	  * retrieve a singleton object
	  * @return a pointer to AMODController .
	  */
	static AMODController* instance();

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
//	override from the class agent, provide a chance to clear up a child pointer when it will be deleted from system
//	virtual void unregisteredChild(Entity* child);

private:
	explicit AMODController(int id=-1,
			const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id),
																		connectPoint(new AMODClient(ioService)), frameTicks(0), waitingseconds(10){}
private:
	// keep all children agents to communicate with it
	std::vector<Agent*> allChildren;
	//identify whether or not communication is created
	bool isConnectAmodServer;
	AmodClientPtr connectPoint;
	std::string ipAddress;
	std::string mapFile;
	int port;
	int updateTiming;
	int frameTicks;
	int waitingseconds;
private:
	static AMODController* pInstance;
	static boost::asio::io_service ioService;
};

} /* namespace AMOD */
} /* namespace sim_mob */
#endif /* AMODController_HPP_ */
