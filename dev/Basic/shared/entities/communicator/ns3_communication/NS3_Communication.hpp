#pragma once
//#include "buffering/Shared.hpp"
#include "../Communication_Base.hpp"
//#include "../CommunicationData.hpp"

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
class ASIO_Impl;

class NS3_Communication: public sim_mob::Communication<DataContainer*, commResult> {
	//ASIO implementation needs these variables for its operations
	 DataContainer& sendBuffer;
	 DataContainer& receiveBuffer;

public:
	NS3_Communication(DataContainer *sedBuffer_ = 0, DataContainer* receiveBuffer_ = 0);
	void init();
	commResult send(DataContainer *value);
	commResult receive(DataContainer* value);
	void shortCircuit(std::string sendFile_ = "./send.txt", std::string receiveFile_ = "./receive.txt");

private:
	//Send & Receive implementations
//	FileBasedImpl *SR_Impl;
//	sim_mob::Communication<DataContainer&, commResult> *SR_Impl;//testing
	sim_mob::ASIO_Impl *SR_Impl;//testing
};



}//namespace sim_mob

