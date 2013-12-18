#include "entities/commsim/Broker.hpp"

namespace sim_mob {

class Roadrunner_Broker :public Broker{
public:

	/**
	 * constructor and destructor
	 */
	explicit Roadrunner_Broker(const MutexStrategy& mtxStrat, int id=-1,std::string commElement_ = "roadrunner", std::string commMode_ = "android-only");
	virtual void configure();
	void onAgentUpdate(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const UpdateEventArgs& argums);
	void onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums);

};

}
