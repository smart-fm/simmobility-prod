#pragma once
#include "buffering/Shared.hpp"
#include "../Communication_Base.hpp"
#include "../CommunicationData.hpp"

//#include "serialized_asio/ASIO_Impl.hpp"
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

class NS3_Communication: public Communication<DataContainer&, commResult> {

public:
	NS3_Communication() ;
	commResult send(DataContainer &value);
	commResult receive(DataContainer& value);
	void shortCircuit(std::string sendFile_ = "./send.txt", std::string receiveFile_ = "./receive.txt");

private:
	//Send & Receive implementations
	FileBasedImpl *SR_Impl;
	ASIO_Impl *SR_Impl_1;//testing
};



}//namespace sim_mob

