#include "STK.hpp"

#include <sstream>
#include <boost/assign/list_of.hpp>
#include <json/json.h>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "workers/Worker.hpp"

#include "entities/commsim/broker/base/Common.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/event/RegionsAndPathEventArgs.hpp"

#include "entities/commsim/client/derived/android/AndroidClientRegistration.hpp"
#include "entities/commsim/client/derived/ns3/NS3ClientRegistration.hpp"

#include "geospatial/RoadRunnerRegion.hpp"

#include "entities/commsim/wait/WaitForAndroidConnection.hpp"
#include "entities/commsim/wait/WaitForNS3Connection.hpp"
#include "entities/commsim/wait/WaitForAgentRegistration.hpp"

sim_mob::STK_Broker::STK_Broker(const MutexStrategy& mtxStrat, int id ,std::string commElement_, std::string commMode_) :
Broker(mtxStrat, id, commElement_, commMode_)
{
}


//configure the brokers behavior
//publishers, blockers(synchronizers) and message handlers... to name a few
void sim_mob::STK_Broker::configure()
{
	//Only configure once.
	if (!configured_.check()) {
		return;
	}

	const std::string & client_mode = ConfigManager::GetInstance().FullConfig().getCommSimMode(commElement);
	sim_mob::Worker::GetUpdatePublisher().subscribe((event::EventId)sim_mob::event::EVT_CORE_AGENT_UPDATED, this, &STK_Broker::onAgentUpdate, (event::EventId)sim_mob::event::CXT_CORE_AGENT_UPDATE);

	//	client registration handlers
	//	Also listen to publishers who announce registration of new clients...
	ClientRegistrationHandlerMap[comm::ANDROID_EMULATOR].reset(new sim_mob::AndroidClientRegistration());
	registrationPublisher.registerEvent(comm::ANDROID_EMULATOR);
	registrationPublisher.subscribe((event::EventId)comm::ANDROID_EMULATOR, this, &STK_Broker::onClientRegister);

	if(client_mode == "android-ns3") {
		ClientRegistrationHandlerMap[comm::NS3_SIMULATOR].reset(new sim_mob::NS3ClientRegistration());
		registrationPublisher.registerEvent(comm::NS3_SIMULATOR);
		//listen to publishers who announce registration of new clients...
		registrationPublisher.subscribe((event::EventId)comm::NS3_SIMULATOR, this, &STK_Broker::onClientRegister);
	}

	//service publishers (those services which will be requested by clients at the time of registration)
	publisher.registerEvent(COMMEID_LOCATION);
	publisher.registerEvent(COMMEID_TIME);
	serviceList.push_back(sim_mob::Services::SIMMOB_SRV_LOCATION);
	serviceList.push_back(sim_mob::Services::SIMMOB_SRV_TIME);

	//NS-3 has its own publishers
	if (client_mode == "android-ns3") {
		publisher.registerEvent(COMMEID_ALL_LOCATIONS);
		serviceList.push_back(sim_mob::Services::SIMMOB_SRV_ALL_LOCATIONS);
	}


	bool useNs3 = false;
	if (client_mode == "android-ns3") {
		useNs3 = true;
	} else if (client_mode == "android-only") {
		useNs3 = false;
	} else { throw std::runtime_error("Unknown clientType in Broker."); }


	//Register handlers with "useNs3" flag. OpaqueReceive will throw an exception if it attempts to process a message and useNs3 is not set.
	//NOTE: As far as I can tell, the OpaqueSend/Receive are sufficient for STK as well. In other words, all three "Brokers" use the
	//      same Handlers, and the only behavioral difference occurs due to "useNs3" being true/false.
	handleLookup.addHandlerOverride("OPAQUE_SEND", new sim_mob::OpaqueSendHandler(useNs3));
	handleLookup.addHandlerOverride("OPAQUE_RECEIVE", new sim_mob::OpaqueReceiveHandler(useNs3));


	// wait for connection criteria for this broker
	clientBlockers.insert(std::make_pair(comm::ANDROID_EMULATOR, boost::shared_ptr<WaitForAndroidConnection>(new WaitForAndroidConnection(*this,MIN_CLIENTS))));


	if(client_mode == "android-ns3") {
		clientBlockers.insert(std::make_pair(comm::NS3_SIMULATOR,
				boost::shared_ptr<WaitForNS3Connection>(new WaitForNS3Connection(*this))));

	}

	// wait for connection criteria for this broker
	agentBlockers.insert(std::make_pair(0,
			boost::shared_ptr<WaitForAgentRegistration>(new WaitForAgentRegistration(*this,MIN_AGENTS))));

}



void sim_mob::STK_Broker::onAgentUpdate(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const UpdateEventArgs& argums)
{
	Broker::onAgentUpdate(id,context,sender,argums);
}

void sim_mob::STK_Broker::onClientRegister(sim_mob::event::EventId id, sim_mob::event::Context context, sim_mob::event::EventPublisher* sender, const ClientRegistrationEventArgs& argums)
{
	Broker::onClientRegister(id,context,sender,argums);
}

void sim_mob::STK_Broker::processClientRegistrationRequests()
{
	/*
	 * ns3 client registration will proceed if enough number of
	 * android emulators are registered.
	 * this is because we need to give
	 * enough number of agent add info
	 * to ns3 at the time of registration.
	 * therefore we may have to process android
	 * requests first and then start ns3 client
	 * registration request processing .
	 */

	int ns3HandleResult = 0;
	boost::shared_ptr<ClientRegistrationHandler > handler;
	ClientWaitList::iterator it;
	for (it = clientRegistrationWaitingList.begin(); it != clientRegistrationWaitingList.end();) {
		comm::ClientType clientType = sim_mob::Services::ClientTypeMap[it->first];
		if(clientType == comm::NS3_SIMULATOR){
			//first handle the android emulators
			it++;
			continue;
		}
		handler = ClientRegistrationHandlerMap[clientType];
		if (!handler) {
			std::ostringstream out("");
			out << "No Handler for [" << it->first << "] type of client" << std::endl;
			throw std::runtime_error(out.str());
		}
		if(handler->handle(*this,it->second.request, it->second.existingConn))
		{
			//success: handle() just added to the client to the main client list and started its connectionHandler
			//	next, see if the waiting state of waiting-for-client-connection changes after this process
			bool wait = clientBlockers[clientType]->calculateWaitStatus();
			Print() << "delete from clientRegistrationWaitingList[" << it->first << "]" << std::endl;
			clientRegistrationWaitingList.erase(it++);
		} else {
			it++;
		}
	}

	/*
	 * now take care of the ns3 simulator
	 */

	if((clientRegistrationWaitingList.find("NS3_SIMULATOR")) != clientRegistrationWaitingList.end()){
		//no ns3 has registered yet, let's not waste time
		return;
	}

	//let's waste some time by repeating some code
	it = clientRegistrationWaitingList.equal_range("NS3_SIMULATOR").first;


	handler = ClientRegistrationHandlerMap[comm::NS3_SIMULATOR];
	//if no handler was found for this type of client, throw an exception
	if (!handler) {
		std::ostringstream out("");
		out << "No Handler for [" << it->first << "] type of client" << std::endl;
		throw std::runtime_error(out.str());
	}

	if(handler->handle(*this,it->second.request, it->second.existingConn))
	{
		//success: handle() just added to the client to the main client list and started its connectionHandler
		//	next, see if the waiting state of waiting-for-client-connection changes after this process
		bool wait = clientBlockers[comm::NS3_SIMULATOR]->calculateWaitStatus();
		Print() << "delete from clientRegistrationWaitingList[" << it->first << "]" << std::endl;
		clientRegistrationWaitingList.erase(it);
	}
}
