#include "entities/commsim/Broker.hpp"

namespace sim_mob {

class Roadrunner_Broker :public Broker{
	///Set of clients that need their enableRegionSupport() function called. This can only be done once their time tick is over,
	///  so we pend them on this list. The extra weak_ptr shouldn't be a problem; if the object is destroyed before its
	///  call to enableRegionSupport(), it will just be silently dropped.
	std::set< boost::weak_ptr<sim_mob::ClientHandler> > newClientsWaitingOnRegionEnabling;
public:

	/**
	 * constructor and destructor
	 */
	explicit Roadrunner_Broker(const MutexStrategy& mtxStrat, int id=-1,std::string commElement_ = "roadrunner", std::string commMode_ = "android-only");
	virtual void configure();
	virtual Entity::UpdateStatus update(timeslice now);
	void onAgentUpdate(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const UpdateEventArgs& argums);
	void onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums);
	/**
	 * Set any properties on "new" clients for this time tick. (Currently we do this with a specialized data-structure per client).
	 */
	void setNewClientProps();

	/**
	 * Call a client's "enableRegionSupport()" method later, after that client is definitely done with its frame_tick() method.
	 */
	void pendClientToEnableRegions(boost::shared_ptr<sim_mob::ClientHandler> &clientHandler);
};

}
