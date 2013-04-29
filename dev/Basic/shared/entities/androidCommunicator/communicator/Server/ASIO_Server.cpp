#include "ASIO_Server.hpp"
namespace sim_mob
{
void clientRegistration_(session_ptr sess, server *server_)
{
//	boost::shared_ptr<WhoAreYouProtocol> registration(new WhoAreYouProtocol(sess,server_));
//	WhoAreYouProtocol registration(sess,server_);
	WhoAreYouProtocol *registration = new WhoAreYouProtocol(sess,server_);
	registration->start();
}
}
