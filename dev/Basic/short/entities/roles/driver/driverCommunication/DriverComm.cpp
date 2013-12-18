//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "DriverComm.hpp"
#include "DriverCommFacets.hpp"
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
//#include "entities/communicator/NS3/NS3_Communicator/NS3_Communicator.hpp"
#include "entities/commsim/Broker.hpp"

namespace sim_mob
{

int DriverComm::totalSendCnt = 0;
int DriverComm::totalReceiveCnt = 0;
sim_mob::DriverComm::DriverComm(Person* parent/*, */, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverCommBehavior* behavior, sim_mob::DriverCommMovement* movement):
		Driver(parent,mtxStrat,behavior, movement), AgentCommUtility(parent)
{	}

sim_mob::DriverComm::~DriverComm()
{}

Role* sim_mob::DriverComm::clone(Person* parent) const
{
	DriverCommBehavior* behavior = new DriverCommBehavior(parent);
	DriverCommMovement* movement = new DriverCommMovement(parent);
	DriverComm* driver = new DriverComm(parent, /*&this->communicator, */parent->getMutexStrategy(), behavior, movement);
	behavior->setParentDriver(driver);
	movement->setParentDriver(driver);
	behavior->setParentDriverComm(driver);
	movement->setParentDriverComm(driver);
	//broker, (external)communicator :( ... setting

//	const ConfigParams &cfg = ConfigManager::GetInstance().FullConfig();
//	const std::string &type = cfg.getAndroidClientType();
//	Broker* managingBroker = Broker::getExternalCommunicator(type);
//	Print() << "Setting Broker["  << managingBroker << "] to drivercomm " << std::endl;
//	driver->setBroker(managingBroker);
	//todo this is a hardcoding, to be solved later
	Broker* rr = Broker::getExternalCommunicator("roadrunner");
	Broker* stk = Broker::getExternalCommunicator("stk");
	driver->setBroker("roadrunner", rr);
	driver->setBroker("stk", stk);

	return driver;
}
sim_mob::Agent * DriverComm::getParentAgent()
{
	return parent;
}


#if 0
void sim_mob::DriverComm::receiveModule(timeslice now)
{
	//check if you have received anything in the incoming buffer
	std::cout << "checking isIncomingDirty" << std::endl;
	{
	if(!isIncomingDirty())
		{
			std::cout << "DriverComm::receiveModule=>Nothing to receive( " << getIncoming().get().size() << ")" << std::endl;
			return;
		}
	std::cout << "checking-isIncomingDirty-Done" << std::endl;
	}
	//handover the incoming data to a temporary container and clear
	DataContainer receiveBatch;
	getAndClearIncoming(receiveBatch);
	std::vector<DATA_MSG_PTR> buffer = receiveBatch.get();//easy reading only
	DATA_MSG_PTR it;
	BOOST_FOREACH(it, buffer)
		{
			//		std::ostringstream out("");
			std::cout  << "tick " << now.frame() << " [" ;
			std::cout << this->parent << "]" ;
			{
				std::cout << " Received[ " << it->serial << " : " << it->str << "]" << std::endl;
			}
			receiveCnt  += 1;
			totalReceiveCnt += 1;
		}//for
	//haha, no need of mutex here
	receiveBatch.reset();
}
void sim_mob::DriverComm::sendModule(timeslice now)
{
	std::set<Entity*> &agents = sim_mob::Agent::all_agents;
	std::set<Entity*>::iterator  it , it_end(agents.end());
	for(it = agents.begin(); it != it_end; it++)
	{
		sim_mob::dataMessage *data = new sim_mob::dataMessage();
		data->str = "Hi man how are you\0";
		data->serial = totalSendCnt;
		//small filter
		//1.send only to drivers
		sim_mob::Person * person = dynamic_cast<sim_mob::Person *>(*it);
		sim_mob::DriverComm* role = 0;
		if(person)
			role = dynamic_cast<sim_mob::DriverComm*>((person)->getRole());
		if(!role) {
			continue;
		}
		//2.dont send to yourseld(for some reason, this is bigger than normal pointers
		if(person == this->parent) {
			continue;
		}
		data->receiver = (unsigned long)(*it);
		{
//			boost::unique_lock< boost::shared_mutex > lock(*(Communicator_Mutexes[1]));
			data->serial = totalSendCnt ++;
			addOutgoing(data);
		}
		sendCnt += 1;
//		totalSendCnt += 1;
	}
	//sorry for the hack.
	//Seriously! you don't wnat to put this in the person/agent 's update method, you dont want to put it in the perform_main function
	//then where to put this?
	setAgentUpdateDone(true);
}
#endif
;
}//namespace sim_mob

