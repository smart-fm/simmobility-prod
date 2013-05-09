#include "ASIO_Server.hpp"
//testing to see them error free:
//#include "../message/derived/ANNOUNCE_Handler.hpp"
//#include "../message/derived/ANNOUNCE_Message.hpp"
//#include "../message/base/Message.hpp"
//#include "../message/base/Handler.hpp"
//#include "../message/base/HandlerFactory.hpp"
namespace sim_mob
{
void clientRegistration_(session_ptr sess, server *server_)
{
//	boost::shared_ptr<WhoAreYouProtocol> registration(new WhoAreYouProtocol(sess,server_));
//	WhoAreYouProtocol registration(sess,server_);
	WhoAreYouProtocol *registration = new WhoAreYouProtocol(sess,server_);
	registration->start();
}
};
