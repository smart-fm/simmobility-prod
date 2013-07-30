/*
 * NS3ClientRegistration.cpp
 *
 *  Created on: May 20, 2013
 *      Author: vahid
 */

#include "NS3ClientRegistration.hpp"
#include "entities/commsim/event/subscribers/base/ClientHandler.hpp"
#include "entities/commsim/connection/ConnectionHandler.hpp"
#include "entities/commsim/service/base/Publisher.hpp"
#include "entities/commsim/broker/Common.hpp"
#include "entities/commsim/event/AllLocationsEventArgs.hpp"
#include "AgentsInfo.hpp"
#include <boost/foreach.hpp>

namespace sim_mob {

NS3ClientRegistration::NS3ClientRegistration(/*ConfigParams::ClientType type_) : ClientRegistrationHandler(type_*/) {
	// TODO Auto-generated constructor stub

}

bool NS3ClientRegistration::handle(sim_mob::Broker& broker, sim_mob::ClientRegistrationRequest request){
	//some checks to avoid calling this method unnecessarily
	if(
			broker.getClientWaitingList().empty()
//			|| broker.getRegisteredAgents().empty()
			)
	{
		return false;
	}

		//use it to create a client entry
		boost::shared_ptr<ClientHandler> clientEntry(new ClientHandler(broker));
		boost::shared_ptr<sim_mob::ConnectionHandler > cnnHandler(new ConnectionHandler(
				request.session_
//				,broker
//				,&Broker::messageReceiveCallback
				,broker.getMessageReceiveCallBack()
				,request.clientID
				,ConfigParams::NS3_SIMULATOR
				,(unsigned long int)(0)//not needed
				)
				);
		clientEntry->cnnHandler = cnnHandler;

//		Print()<< "Connection handler for agent "

		clientEntry->AgentCommUtility_ = 0;//not needed
		//todo: some of there information are already available in the connectionHandler! omit redundancies  -vahid
		clientEntry->agent = 0;//not needed
		clientEntry->clientID = request.clientID;
		clientEntry->client_type = ConfigParams::NS3_SIMULATOR;
		clientEntry->requiredServices = request.requiredServices; //will come handy
		SIM_MOB_SERVICE srv;
		int size_i = request.requiredServices.size();
		BOOST_FOREACH(srv, request.requiredServices)
		{
			switch(srv)
			{
			case SIMMOB_SRV_TIME:{
				 boost::shared_ptr<sim_mob::Publisher> p = broker.getPublishers()[SIMMOB_SRV_TIME];
				p->Subscribe(COMMEID_TIME, clientEntry.get(), CALLBACK_HANDLER(sim_mob::TimeEventArgs, ClientHandler::OnTime) );
				Print() << "NS3 registered with SIMMOB_SRV_TIME" << std::endl;
				break;
			}
			case SIMMOB_SRV_ALL_LOCATIONS:{
				 boost::shared_ptr<sim_mob::Publisher> p = broker.getPublishers()[SIMMOB_SRV_ALL_LOCATIONS];
				p->Subscribe(COMMEID_LOCATION,(void *) COMMCID_ALL_LOCATIONS, clientEntry.get()  ,CONTEXT_CALLBACK_HANDLER(AllLocationsEventArgs, ClientHandler::OnAllLocations) );
				Print() << "NS3 registered with SIMMOB_SRV_ALL_LOCATIONS" << std::endl;
				break;
			}
			}
		}

		//also, add the client entry to broker(for message handler purposes)
		broker.insertClientList(clientEntry->clientID, ConfigParams::NS3_SIMULATOR,clientEntry);

		//send some initial configuration information to NS3

		AgentsMap<std::string>::type & agents = broker.getRegisteredAgents();
		std::set<sim_mob::Entity *> keys;
		AgentsMap<std::string>::pair kv;
		BOOST_FOREACH(kv, agents)
		{
			keys.insert(const_cast<sim_mob::Agent *>(kv.first));
		}
		AgentsInfo info;
		info.insertInfo(AgentsInfo::ADD_AGENT, keys);
		clientEntry->cnnHandler->send(info.toJson());//send synchronously

		//start listening to the handler
		clientEntry->cnnHandler->start();
		return true;
}
NS3ClientRegistration::~NS3ClientRegistration() {
	// TODO Auto-generated destructor stub
}

} /* namespace sim_mob */
