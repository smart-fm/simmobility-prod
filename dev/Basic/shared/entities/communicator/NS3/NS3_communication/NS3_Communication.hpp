#pragma once
//#include "buffering/Shared.hpp"
#include "entities/communicator/Communication_Base.hpp"

#include<fstream>
////this file provides some of the communication
////1-tools required for agent "communication subsystem"
////2-buffers used by common agents for their incoming and outgoing data
//
//
///*********************************************************************************
// communication subsystem:
// the basic idea is to provide send and receive functionality.
// This involves :
// 1-defining send and receive arguments and return value types
// 2-Implementing send and receive based on the current requirement, situation and scenario etc.
// 3-used by communicator agent only
// **********************************************************************************/
//
//
namespace sim_mob
{
class FileBasedImpl;
class BoostSerialized_Client_ASIO;

class NS3_Communication: public sim_mob::Communication<DataContainer &, commResult> {
	//ASIO implementation needs these variables for its operations
	 DataContainer& sendBuffer;
	 DataContainer& receiveBuffer;
	 bool work_in_progress;
public:
	NS3_Communication(DataContainer *sedBuffer_ = 0, DataContainer* receiveBuffer_ = 0);
	void init();
	commResult send(DataContainer &value);
	commResult receive(DataContainer& value);
	bool get_work_in_progress();
	void shortCircuit(std::string sendFile_ = "./send.txt", std::string receiveFile_ = "./receive.txt");

private:
	//Send & Receive implementations
//	FileBasedImpl *SR_Impl;
//	sim_mob::Communication<DataContainer&, commResult> *SR_Impl;//testing
	boost::shared_ptr<sim_mob::BoostSerialized_Client_ASIO> SR_Impl;//testing
};



}//namespace sim_mob

