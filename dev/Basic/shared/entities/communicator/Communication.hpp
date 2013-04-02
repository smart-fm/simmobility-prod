#pragma once
#include "buffering/Shared.hpp"
#include "CommunicationData.hpp"
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

#define sendFile_temp "./send.txt"
#define receiveFile_temp "./receive.txt"

class NS3_Communication: public Communication<DataContainer&, commResult> {

	//begin nested class FileBasedImpl
	class FileBasedImpl: public Communication<DataContainer&, commResult> {
	    std::string sendFile;
	    std::string receiveFile;
	    std::ofstream ofs;
	    std::ifstream ifs;
	public:
	    FileBasedImpl(std::string sendFile_ = sendFile_temp, std::string receiveFile_ = sendFile_temp);//for test purpose, send & receive are same
		commResult send(DataContainer& value);
		commResult receive(DataContainer&value);
		void shortCircuit(std::string sendFile_ = sendFile_temp, std::string receiveFile_ = sendFile_temp);
	};
	//end of nested class

	//begin nested class ASIOImpl
	class ASIO_Impl: public Communication<DataContainer&, commResult>
	{

	public:
		ASIO_Impl();
		bool init();
		commResult send(DataContainer& value);
		commResult receive(DataContainer&value);
	};

public:
	NS3_Communication() ;
	commResult send(DataContainer &value) {

		return SR_Impl->send(value);
	}
	commResult receive(DataContainer& value) {
		return SR_Impl->receive(value);
	}
	void shortCircuit(std::string sendFile_ = "./send.txt", std::string receiveFile_ = "./receive.txt"){
		SR_Impl->shortCircuit(sendFile_, receiveFile_);
	}

private:
	FileBasedImpl *SR_Impl; //Send & Receive implementation
};



}//namespace sim_mob

