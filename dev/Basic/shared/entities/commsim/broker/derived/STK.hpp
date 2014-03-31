#include "entities/commsim/Broker.hpp"

namespace sim_mob {
class STK_Broker :public Broker{
public:
	explicit STK_Broker(const MutexStrategy& mtxStrat, int id=-1,std::string commElement_ = "stk", std::string commMode_ = "android-only");
	~STK_Broker() {}

	/**
		basic configuration script
	*/
	//virtual void configure();

	/**
		what happens when a client is registered
	*/
	//void onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums);
	/**
	 * processes clients requests to be registered with the broker
	 */
//	void processClientRegistrationRequests();
};
}
