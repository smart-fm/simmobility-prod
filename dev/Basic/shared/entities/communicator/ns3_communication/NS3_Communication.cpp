#include "NS3_Communication.hpp"
#include "FileBasedImpl.hpp"
#include "ASIO_Impl.hpp"
namespace sim_mob {

/*
 * *************************************************************
 * 						NS3_Communication
 * * ***********************************************************
 */
NS3_Communication::NS3_Communication() {
	//todo: configuration based
	SR_Impl = new FileBasedImpl();
}
commResult NS3_Communication::send(DataContainer &value) {

	return SR_Impl->send(value);
}
commResult NS3_Communication::receive(DataContainer& value) {
	return SR_Impl->receive(value);
}
void NS3_Communication::shortCircuit(std::string sendFile_, std::string receiveFile_){
	SR_Impl->shortCircuit(sendFile_, receiveFile_);
}
}
;
//namespace sim_mob
