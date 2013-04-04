#pragma once
//#include "buffering/Shared.hpp"
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
template<class ARGS, class RET>
class Communication
{
public:
	virtual RET send(ARGS) = 0;
	virtual RET receive(ARGS) = 0;
};
}//namespace sim_mob

