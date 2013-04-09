#include "NS3_Communication.hpp"
#include "FileBasedImpl.hpp"
#include "ASIO_Impl.hpp"
//#include "../test/NS3_Test_1.hpp"
namespace sim_mob {

/*
 * *************************************************************
 * 						NS3_Communication
 * * ***********************************************************
 */

NS3_Communication::NS3_Communication(DataContainer *sedBuffer_, DataContainer* receiveBuffer_) :
	sendBuffer(*sedBuffer_), receiveBuffer(*receiveBuffer_)
{
	init();
	};
void NS3_Communication::init()
{	//todo: configuration based
	//	SR_Impl = new FileBasedImpl();

//	std::cout << " NS3_Communication's SendBuffer address [" << &sendBuffer <<  ":" << &(sendBuffer.buffer) << "]" << std::endl;
		SR_Impl = new ASIO_Impl(receiveBuffer);
//		boost::thread FakeNS3(fakeNS3);

}
commResult NS3_Communication::send(DataContainer *value) {
	WriteLock(*myLock);
	return SR_Impl->send(value);
}
commResult NS3_Communication::receive(DataContainer* value) {
	return SR_Impl->receive(value);
}
//void NS3_Communication::shortCircuit(std::string sendFile_, std::string receiveFile_){
//	SR_Impl->shortCircuit(sendFile_, receiveFile_);
//}
}
;
//namespace sim_mob
