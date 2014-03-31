#include "RoadRunner.hpp"

#include <sstream>
#include <boost/assign/list_of.hpp>
#include <json/json.h>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "workers/Worker.hpp"

#include "entities/commsim/broker/base/Common.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/connection/ConnectionServer.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/event/RegionsAndPathEventArgs.hpp"

#include "entities/commsim/client/derived/android/AndroidClientRegistration.hpp"
#include "entities/commsim/client/derived/ns3/NS3ClientRegistration.hpp"

#include "geospatial/RoadRunnerRegion.hpp"
#include "entities/profile/ProfileBuilder.hpp"


#include "entities/commsim/wait/WaitForAndroidConnection.hpp"
#include "entities/commsim/wait/WaitForNS3Connection.hpp"
#include "entities/commsim/wait/WaitForAgentRegistration.hpp"

sim_mob::Roadrunner_Broker::Roadrunner_Broker(const MutexStrategy& mtxStrat, int id ,std::string commElement_, std::string commMode_) :
Broker(mtxStrat, id, commElement_, commMode_)
{
}



sim_mob::Entity::UpdateStatus sim_mob::Roadrunner_Broker::update(timeslice now) {
	if (EnableDebugOutput) {
		Print() << "Broker tick:" << now.frame() << std::endl;
	}

	//step-1 : Create/start the thread if this is the first frame.
	//TODO: transfer this to frame_init
	if (now.frame() == 0) {
		connection->start(); //Not profiled; this only happens once.
	}

	if (EnableDebugOutput) {
		Print() << "=====================ConnectionStarted =======================================\n";
	}

	PROFILE_LOG_COMMSIM_UPDATE_BEGIN(currWorkerProvider, this, now);

	//Step-2: Ensure that we have enough clients to process
	//(in terms of client type (like ns3, android emulator, etc) and quantity(like enough number of android clients) ).
	//Block the simulation here(if you have to)
	waitAndAcceptConnections();

	if (EnableDebugOutput) {
		Print() << "===================== wait Done =======================================\n";
	}

	//step-3: Process what has been received in your receive container(message queue perhaps)
	size_t numAgents = getNumConnectedAgents();
	processIncomingData(now);
	if (EnableDebugOutput) {
		Print() << "===================== processIncomingData Done =======================================\n";
	}

	//step-4: if need be, wait for all agents(or others)
	//to complete their tick so that you are the last one ticking)
	PROFILE_LOG_COMMSIM_LOCAL_COMPUTE_BEGIN(currWorkerProvider, this, now, numAgents);
	waitForAgentsUpdates();
	PROFILE_LOG_COMMSIM_LOCAL_COMPUTE_END(currWorkerProvider, this, now, numAgents);

	if (EnableDebugOutput) {
		Print() << "===================== waitForAgentsUpdates Done =======================================\n";
	}

	//step-5: signal the publishers to publish their data
	processPublishers(now);
	if (EnableDebugOutput) {
		Print() << "===================== processPublishers Done =======================================\n";
	}

//	step-5.5:for each client, append a message at the end of all messages saying Broker is ready to receive your messages
	sendReadyToReceive();

	//This may have changed (or we should at least log if it did).
	numAgents = getNumConnectedAgents();

	//step-6: Now send all what has been prepared, by different sources, to their corresponding destications(clients)
	PROFILE_LOG_COMMSIM_MIXED_COMPUTE_BEGIN(currWorkerProvider, this, now, numAgents);
	processOutgoingData(now);
	PROFILE_LOG_COMMSIM_MIXED_COMPUTE_END(currWorkerProvider, this, now, numAgents);
	if (EnableDebugOutput) {
		Print() << "===================== processOutgoingData Done =======================================\n";
	}

	//step-7:
	//the clients will now send whatever they want to send(into the incoming messagequeue)
	//followed by a Done! message.That is when Broker can go forwardClientList::pair clientByType;
	PROFILE_LOG_COMMSIM_ANDROID_COMPUTE_BEGIN(currWorkerProvider, this, now);
	waitForClientsDone();
	PROFILE_LOG_COMMSIM_ANDROID_COMPUTE_END(currWorkerProvider, this, now);
	if (EnableDebugOutput) {
		Print() << "===================== waitForClientsDone Done =======================================\n";
	}

	//step-8:
	//Now that all clients are done, set any properties on new clients.
	if (!newClientsWaitingOnRegionEnabling.empty()) {
		setNewClientProps();
		if (EnableDebugOutput) {
			Print() << "===================== setNewClientProps Done =======================================\n";
		}
	}

	//step-9: final steps that should be taken before leaving the tick
	//prepare for next tick.
	cleanup();

	PROFILE_LOG_COMMSIM_UPDATE_END(currWorkerProvider, this, now);

	return UpdateStatus(UpdateStatus::RS_CONTINUE);
}

/*void sim_mob::Roadrunner_Broker::onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums)
{
	Broker::onClientRegister(id,context,sender,argums);
}*/


void sim_mob::Roadrunner_Broker::setNewClientProps()
{
	//NOTE: I am locking this anyway, just to be safe. (In case a new Broker request arrives in the meantime). ~Seth
	boost::unique_lock<boost::mutex> lock(mutex_clientList);

	//Now, loop through each client and send it a message to inform that the Broker has registered it.
	for (std::set< boost::weak_ptr<sim_mob::ClientHandler> >::iterator it=newClientsWaitingOnRegionEnabling.begin(); it!=newClientsWaitingOnRegionEnabling.end(); it++) {
		//Attempt to resolve the weak pointer.
		boost::shared_ptr<sim_mob::ClientHandler> cHand = it->lock();
		if (cHand) {
			//NOTE: Be CAREFUL here using the Agent pointer as a Context (void*). If you have a class with multiple inheritance,
			//      the void* for different realizations of the same object may have DIFFERENT pointer values. ~Seth
			messaging::MessageBus::PublishEvent(sim_mob::event::EVT_CORE_COMMSIM_ENABLED_FOR_AGENT,
				cHand->agent,
				messaging::MessageBus::EventArgsPtr(new event::EventArgs())
			);
		} else {
			Warn() <<"Broker::setNewClientProps() -- Client was destroyed before its weak_ptr() could be resolved.\n";
		}
	}

	//These clients have been processed.
	newClientsWaitingOnRegionEnabling.clear();
}

