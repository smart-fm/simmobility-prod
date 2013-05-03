
#include "DriverComm.hpp"
#include "entities/Person.hpp"
//#include "entities/communicator/NS3/NS3_Communicator/NS3_Communicator.hpp"
#include "entities/androidCommunicator/communicator/Broker.hpp"

namespace sim_mob
{

int DriverComm::totalSendCnt = 0;
int DriverComm::totalReceiveCnt = 0;
sim_mob::DriverComm::DriverComm(Person* parent, sim_mob::MutexStrategy mtxStrat):Driver(parent,mtxStrat), JCommunicationSupport(*parent)
{ }
sim_mob::DriverComm::~DriverComm(){
}

Role* sim_mob::DriverComm::clone(Person* parent) const
{
	Role* role = 0;
	role = new DriverComm(parent, parent->getMutexStrategy());

	return role;
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
	std::vector<Entity*> &agents = sim_mob::Agent::all_agents;
	std::vector<Entity*>::iterator  it , it_end(agents.end());
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
//Virtual implementations
void DriverComm::frame_init(UpdateParams& p) {
	Driver::frame_init(p);
//	subscribed = subscribe(this->parent, sim_mob::NS3_Communicator::GetInstance());
	subscribed = subscribe(this->parent, sim_mob::Broker::GetInstance());
}
;
void DriverComm::frame_tick(UpdateParams& p) {
	std::cout << "[" << this->parent << "]:DriverComm::frame_tick(" << p.now.frame() << ")" << std::endl;
	Driver::frame_tick(p);
//	if((p.now.frame() > 4)&&(p.now.frame() <= 400))
//	{
//		sendModule(p.now);
//	}
////	else if((p.now.frame() >= 4) && (p.now.frame() < 10) )//todo, just to test, just put else without if
////	{
//		receiveModule(p.now);
////	}
	setAgentUpdateDone(true);
	std::cout << "[" << this->parent << "]: AgentUpdate Done" << std::endl;
	std::cerr  << std::dec << "tick " << p.now.frame() << " [" << this->parent << "] send:[" << sendCnt << "] totalsend:[" << sim_mob::DriverComm::totalSendCnt << "]    receive:[" << receiveCnt << "]   totalreceive:[" << sim_mob::DriverComm::totalReceiveCnt << "]" << std::endl;


}
;
void DriverComm::frame_tick_output(const UpdateParams& p) {
	Driver::frame_tick_output(p);
}
;
void DriverComm::frame_tick_output_mpi(timeslice now) {
	Driver::frame_tick_output_mpi(now);
}
;
}//namespace sim_mob

