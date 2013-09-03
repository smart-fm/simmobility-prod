#pragma once
#include "entities/roles/driver/Driver.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
namespace sim_mob
{
class Broker;
class DriverCommMovement;
class DriverCommBehavior;

class DriverComm : public Driver, public AgentCommUtilityBase
{
	static int totalSendCnt;
	static int totalReceiveCnt;
	int sendCnt,receiveCnt;
public:

	DriverComm(Person* parent/*, Broker* managingBroker*/, sim_mob::MutexStrategy mtxStrat, sim_mob::DriverCommBehavior* behavior = nullptr, sim_mob::DriverCommMovement* movement = nullptr);
	virtual ~DriverComm();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;

	void receiveModule(timeslice now);
	void sendModule(timeslice now);
	sim_mob::Agent * getParentAgent();
};

}//namspace sim_mob
