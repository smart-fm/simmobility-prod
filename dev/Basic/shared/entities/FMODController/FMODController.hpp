/*
 * FMODController.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef FMODCONTROLLER_HPP_
#define FMODCONTROLLER_HPP_
#include "TCPSession.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Agent.hpp"

namespace sim_mob {

namespace FMOD
{

class FMODController : public sim_mob::Agent {
public:
	explicit FMODController(int id=-1, const MutexStrategy& mtxStrat = sim_mob::MtxStrat_Buffered) : Agent(mtxStrat, id), connectPoint(new TCPSession(io_service)){}

	virtual ~FMODController();

	bool CollectFMODAgents(std::vector<sim_mob::Entity*>& all_agents);
	void SetConnection(std::string ipadress, int port) { this->ipAddress=ipadress; this->port=port; }

	static void RegisterController(int id, const MutexStrategy& mtxStrat);
	static FMODController* Instance();

private:
	// keep all children agents to communicate with it
	std::vector<Entity*> all_persons;
	std::vector<Entity*> all_drivers;

protected:
	virtual bool frame_init(timeslice now);
	virtual Entity::UpdateStatus frame_tick(timeslice now);
	virtual void frame_output(timeslice now);

	//May implement later
	virtual void load(const std::map<std::string, std::string>& configProps){}
	//Signals are non-spatial in nature.
	virtual bool isNonspatial() { return true; }
	virtual void buildSubscriptionList(std::vector<BufferedBase*>& subsList){}
	bool StartService();
	void StopService();

private:
	void ProcessMessages();
	MessageList CollectRequest();
	MessageList HandleOfferMessage(std::string msg);
	MessageList HandleConfirmMessage(std::string msg);
	void HandleScheduleMessage(std::string msg);
	void HandleVehicleInit(std::string msg);

private:
	TCPSessionPtr connectPoint;
	std::string ipAddress;
	int port;

private:
	static FMODController* pInstance;
	boost::asio::io_service io_service;

};

}

} /* namespace sim_mob */
#endif /* FMODCONTROLLER_HPP_ */
