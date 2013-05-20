#include "entities/roles/driver/Driver.hpp"
#include "entities/commsim/comm_support/JCommunicationSupport.hpp"
namespace sim_mob
{
class Broker;

class DriverComm : public Driver, public JCommunicationSupport
{
	static int totalSendCnt;
	static int totalReceiveCnt;
	int sendCnt,receiveCnt;
public:

	DriverComm(Person* parent, Broker* managingBroker, sim_mob::MutexStrategy mtxStrat);
	virtual ~DriverComm();

	virtual sim_mob::Role* clone(sim_mob::Person* parent) const;
	//Virtual implementations
	virtual void frame_init(UpdateParams& p);
	virtual void frame_tick(UpdateParams& p);
	virtual void frame_tick_output(const UpdateParams& p);
	virtual void frame_tick_output_mpi(timeslice now);

	void receiveModule(timeslice now);
	void sendModule(timeslice now);
};

}//namspace sim_mob
