#pragma once
#include "buffering/Shared.hpp"
#include "CommunicationData.hpp"

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
////todo this is rather useless for boost serialization, change it-vahid
//class dataMessage
//{
//public:
//	std::string data;
//};
//



template<class ARGS, class RET>
class Communication
{
public:
	virtual RET send(ARGS) = 0;
	virtual RET receive(ARGS) = 0;
};

class NS3_Communication: public Communication<std::set<DATA_MSG_PTR>&, commResult> {

	//nested class
	class FileBasedImpl: public Communication<std::set<DATA_MSG_PTR>&, commResult> {
	    std::string sendFile;
	    std::string receiveFile;
	public:
	    FileBasedImpl(std::string sendFile_ = "./send.txt", std::string receiveFile_ = "send.txt");//for test purpose, send & receive are same
		commResult send(std::set<DATA_MSG_PTR>& value);
		commResult receive(std::set<DATA_MSG_PTR>&value);
	};
	//end of nested class

public:
	NS3_Communication() ;
	commResult send(std::set<DATA_MSG_PTR> &value) {

		return SR_Impl->send(value);
	}
	commResult receive(std::set<DATA_MSG_PTR>& value) {
		return SR_Impl->receive(value);
	}

private:
	FileBasedImpl *SR_Impl; //Send & Receive implementation
};



}//namespace sim_mob

