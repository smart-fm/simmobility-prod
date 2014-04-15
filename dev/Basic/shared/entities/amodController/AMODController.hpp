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

#include "entities/Agent.hpp"

namespace sim_mob {

namespace AMOD {

class AMODController : public sim_mob::Agent{
public:
	virtual ~AMODController();

	/**
	  * retrieve a singleton object
	  * @return a pointer to AMODController .
	  */
	static AMODController* instance();
	/**
	      * check whether or not the instance already existed
	      * @return true if existed .
	      */
	static bool instanceExists();
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
																		 frameTicks(0){}
private:
	// keep all children agents to communicate with it
	std::vector<Agent*> allChildren;
	//identify whether or not communication is created
//	bool isConnectAmodServer;
//	AmodClientPtr connectPoint;
//	std::string ipAddress;
//	std::string mapFile;
//	int port;
//	int updateTiming;
	int frameTicks;
//	int waitingseconds;
private:
	static AMODController* pInstance;
//	static boost::asio::io_service ioService;

	//when vehicle initialize and pack, it will store to this structure
//	FMOD::ParkingCoordinator parkingCoord;
};

} /* namespace AMOD */
} /* namespace sim_mob */
#endif /* AMODController_HPP_ */
