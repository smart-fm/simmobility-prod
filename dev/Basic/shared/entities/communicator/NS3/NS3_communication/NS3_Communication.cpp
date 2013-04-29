#include "NS3_Communication.hpp"
#include "file_based/FileBasedImpl.hpp"
#include "boost_serialized_asio_client/Boost_Serialized_ASIO_Client.hpp"
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
{
	work_in_progress = false;
	//todo: configuration based
	//	SR_Impl = new FileBasedImpl();

//	std::cout << " NS3_Communication's SendBuffer address [" << &sendBuffer <<  ":" << &(sendBuffer.buffer) << "]" << std::endl;
		SR_Impl.reset(new BoostSerialized_Client_ASIO("localhost","2013",receiveBuffer));

//		boost::thread FakeNS3(fakeNS3);

}
commResult NS3_Communication::send(DataContainer &value) {
	return SR_Impl->send(value);
}
commResult NS3_Communication::receive(DataContainer& value) {
	return SR_Impl->receive(value);
}
//void NS3_Communication::shortCircuit(std::string sendFile_, std::string receiveFile_){
//	SR_Impl->shortCircuit(sendFile_, receiveFile_);
//}
}
;
//namespace sim_mob
