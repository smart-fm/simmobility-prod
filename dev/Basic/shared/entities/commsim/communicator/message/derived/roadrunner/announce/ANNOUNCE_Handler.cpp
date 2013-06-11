/*
 * ANNOUNCE_Handler.cpp
 *
 *  Created on: May 9, 2013
 *      Author: vahid
 */
#include "ANNOUNCE_Handler.hpp"
#include "entities/commsim/communicator/service/services.hpp"
namespace sim_mob {
namespace roadrunner
{

void HDL_ANNOUNCE::handle(msg_ptr message_,boost::shared_ptr<Broker> broker){
//will require a dynamic_cast but no matter this design saved
//alot of space and time for creating new handlers over and over

	Json::Value &data = message_->getData();

	Print() << "HDL_ANNOUNCE::handle is handling a mesage" << std::endl;
	if(data.isMember("SENDER_TYPE") && data.isMember("SENDER"))
	{
		Print() << "Sender : " << data["SENDER"] << "  sender_type : " << data["SENDER_TYPE"] << std::endl;
	}
//	find the agent from the client
	ClientType clientType = ClientTypeMap[data["SENDER_TYPE"].asString()];
//	broker->getClientList()[clientType];


}

}/* namespace roadrunner */
} /* namespace sim_mob */



