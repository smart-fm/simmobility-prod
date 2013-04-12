
#include "DriverComm.hpp"
#include "entities/Person.hpp"
#include "entities/communicator/Communicator.hpp"

namespace sim_mob
{

sim_mob::DriverComm::DriverComm(Person* parent, sim_mob::MutexStrategy mtxStrat):Driver(parent,mtxStrat), CommunicationSupport(*parent)
{ }
sim_mob::DriverComm::~DriverComm(){
}

Role* sim_mob::DriverComm::clone(Person* parent) const
{
	Role* role = 0;
	role = new DriverComm(parent, parent->getMutexStrategy());

	return role;
}
void sim_mob::DriverComm::receiveModule(timeslice now)
{
	//check if you have received anything in the incoming buffer
	if(!isIncomingDirty())
		{
			std::cout << "DriverComm::receiveModule=>Nothing to receive( " << getIncoming().get().size() << ")" << std::endl;
			return;
		}
	std::cout << "tick " << now.frame() << " [" << this->parent << "] incoming is dirty" << std::endl;
	if(getIncoming().get().size() < 0) { std::cout << " But there are no data" << std::endl; return; }
	for(std::vector<DATA_MSG_PTR>::iterator it = getIncoming().get().begin(); it != getIncoming().get().end(); it++)
	{
//		std::ostringstream out("");
		std::cout  << "tick " << now.frame() << " [" ;
		std::cout << this->parent << "]" ;
		{
			WriteLock(Communicator_Mutex);
			std::cout << " Received[" << (*it)->str << "]" << std::endl;
		}
	}
	getIncoming().reset();


}
void sim_mob::DriverComm::sendModule(timeslice now)
{
	//		WriteLock(Communicator_Mutex);
	std::vector<Entity*> &agents = sim_mob::Agent::all_agents;
	std::vector<Entity*>::iterator  it , it_end(agents.end());
	for(it = agents.begin(); it != it_end; it++)
	{
		sim_mob::dataMessage *data = new sim_mob::dataMessage();
		data->str = "Hi man how are you\n\0";
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
		addOutgoing(data);
	}
	//sorry for the hack.
	//Seriously! you don't wnat to put this in the person/agent 's update method, you dont want to put it in the perform_main function
	//then where to put this?
	setAgentUpdateDone(true);
}
//Virtual implementations
void DriverComm::frame_init(UpdateParams& p) {
	Driver::frame_init(p);
	subscribe(this->parent, sim_mob::NS3_Communicator::GetInstance());
}
;
void DriverComm::frame_tick(UpdateParams& p) {
	std::cout << "[" << this->parent << "]:DriverComm::frame_tick(" << p.now.frame() << ")" << std::endl;
	Driver::frame_tick(p);
	if(p.now.frame() == 4)
	{
		sendModule(p.now);
	}
//	else if((p.now.frame() >= 4) && (p.now.frame() < 10) )//todo, just to test, just put else without if
//	{
		receiveModule(p.now);
//	}
	setAgentUpdateDone(true);
	std::cout << "[" << this->parent << "]: AdentUpdate Done" << std::endl;

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

