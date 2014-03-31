#include "entities/commsim/Broker.hpp"

namespace sim_mob {

class Roadrunner_Broker : public Broker{
public:
	explicit Roadrunner_Broker(const MutexStrategy& mtxStrat, int id=-1,std::string commElement_ = "roadrunner", std::string commMode_ = "android-only");
	~Roadrunner_Broker() {}

	//virtual void configure();
	virtual Entity::UpdateStatus update(timeslice now);
	//void onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums);
	/**
	 * Set any properties on "new" clients for this time tick. (Currently we do this with a specialized data-structure per client).
	 */
	void setNewClientProps();

	/**
	 * Call a client's "enableRegionSupport()" method later, after that client is definitely done with its frame_tick() method.
	 */
	//void pendClientToEnableRegions(boost::shared_ptr<sim_mob::ClientHandler> &clientHandler);

};

}
