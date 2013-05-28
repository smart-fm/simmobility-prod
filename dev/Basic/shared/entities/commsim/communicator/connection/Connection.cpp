#include "Connection.hpp"
#include "boost/tuple/tuple.hpp"
namespace sim_mob
{
void ConnectionServer::handleNewClient(session_ptr sess)
{
	//using boost_shared_ptr won't let the protocol to release(i guess).
	//Therefore I used raw pointer. the protocol will delete itself(delete this;)
	WhoAreYouProtocol *registration = new WhoAreYouProtocol(sess,this);
	registration->start();
}


void ConnectionServer::CreatSocketAndAccept() {
	// Start an accept operation for a new connection.
	std::cout << "Accepting..." << std::endl;
	sim_mob::session_ptr new_sess(new sim_mob::Session(io_service_));
	acceptor_.async_accept(new_sess->socket(),
			boost::bind(&ConnectionServer::handle_accept, this,
					boost::asio::placeholders::error, new_sess));
}

ConnectionServer::ConnectionServer(	std::queue<boost::tuple<unsigned int,ClientRegistrationRequest > > &clientRegistrationWaitingList_,
		boost::shared_ptr<boost::mutex> Broker_Client_Mutex_,
		boost::shared_ptr<boost::condition_variable> COND_VAR_CLIENT_REQUEST_,
		unsigned short port)
:
		Broker_Client_Mutex(Broker_Client_Mutex_),
		COND_VAR_CLIENT_REQUEST(COND_VAR_CLIENT_REQUEST_),
		acceptor_(io_service_,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
		clientRegistrationWaitingList(clientRegistrationWaitingList_)
{

}

void ConnectionServer::start()
{
	io_service_thread = boost::thread(&ConnectionServer::io_service_run,this);
}

void ConnectionServer::io_service_run()
{
	acceptor_.listen();
	CreatSocketAndAccept();
	io_service_.run();
}
void ConnectionServer::handle_accept(const boost::system::error_code& e, session_ptr sess) {
	if (!e) {
		std::cout << "Connection Accepted" << std::endl;
		handleNewClient(sess);
	}
	else
	{
		std::cout << "Connection Refused" << std::endl;
	}
	CreatSocketAndAccept();
}
//void ConnectionServer::RequestClientRegistration(unsigned int ID, unsigned int type, session_ptr session_)
void ConnectionServer::RequestClientRegistration(sim_mob::ClientRegistrationRequest request)
{
	unsigned int ID;
	unsigned int type;
	session_ptr session_;
	boost::unique_lock< boost::mutex > lock(*Broker_Client_Mutex);//todo remove comment
	clientRegistrationWaitingList.insert(std::make_pair(request.client_type, request));
	COND_VAR_CLIENT_REQUEST->notify_one();
	std::cout << " RequestClientRegistration Done, returning" << std::endl;
}

void ConnectionServer::read_handler(const boost::system::error_code& e, std::string &data, session_ptr sess) {
	if (!e) {
		std::cout << "read Successful" << std::endl;
	} else {
		std::cout << "read Failed" << std::endl;
	}

}

void ConnectionServer::general_send_handler(const boost::system::error_code& e, session_ptr sess) {
	if (!e) {
		std::cout << "write Successful" << std::endl;
	} else {
		std::cout << "write Failed:" << e.message() <<  std::endl;
	}

}

ConnectionServer::~ConnectionServer()
{
	io_service_.stop();
	io_service_thread.join();
}


/*
 * **********************************************
 * *************  WhoAreYouProtocol *************
 * **********************************************
 */

WhoAreYouProtocol::WhoAreYouProtocol(session_ptr sess_, ConnectionServer *server__):sess(sess_),server_(server__),registerSuccess(false){
	}
	void WhoAreYouProtocol::start()
	{
		startClientRegistration(sess);
	}
	bool WhoAreYouProtocol::isDone()
	{
		return registerSuccess;
	}

	void WhoAreYouProtocol::getTypeAndID(std::string& input, unsigned int & type, unsigned int & ID)
	{
		 JsonParser::getTypeAndID(input, type, ID);
	}
	sim_mob::ClientRegistrationRequest WhoAreYouProtocol::getSubscriptionRequest(std::string input, session_ptr sess)
	{

		sim_mob::ClientRegistrationRequest candidate;
		 JsonParser::getTypeAndID(input, candidate.client_type, candidate.clientID);
		candidate.session_ = sess;
		JsonParser::getServices(input,candidate.requiredServices);

	}

	void WhoAreYouProtocol::startClientRegistration(session_ptr sess) {
		std::string str = JsonParser::makeWhoAreYou();
		std::cout<< " WhoAreYou protocol Sending [" << str << "]" <<  std::endl;
		sess->async_write(str,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_handler, this,
						boost::asio::placeholders::error, sess));
	}
	void WhoAreYouProtocol::WhoAreYou_handler(const boost::system::error_code& e,session_ptr sess) {
		std::cout<< " WhoAreYou_handler readring" << std::endl;
		sess->async_read(response,
				boost::bind(&WhoAreYouProtocol::WhoAreYou_response_handler, this,
						boost::asio::placeholders::error, sess));
	}

	void WhoAreYouProtocol::WhoAreYou_response_handler(const boost::system::error_code& e, session_ptr sess) {
		if(e)
		{
			std::cerr << "WhoAreYou_response_handler Fail [" << e.message() << "]" << std::endl;
		}
		else
		{
			std::cout<< " WhoAreYou_handler read Done [" << response << "]"  << std::endl;
			//response string is successfully populated
			unsigned int id = -1 , type = -1;
			sim_mob::ClientRegistrationRequest request = getSubscriptionRequest(response, sess);
	        getTypeAndID(response, type, id);
	        std::cout << "response = " << id << " " << type << std::endl;
			server_->RequestClientRegistration(request);
			std::cout << "registering the client Done" << std::endl;
			delete this; //first time in my life! people say it is ok.


		}
	}


	/***************************************************************************
	 *************************        ConnectionHandler       *****************************
	 ****************************************************************************/
	class Broker;
		//NOTE: Passing "callback" by value and then saving it by reference is a bad idea!
		//      For now I've made both work by value; you may need to modify this. ~Seth
		ConnectionHandler::ConnectionHandler(
				session_ptr session_ ,
				Broker& broker,
				messageReceiveCallback callback,
				unsigned int clientID_,
				unsigned int ClienType_ = 0,
				unsigned long agentPtr_
				):theBroker(broker), receiveCallBack(callback)

		{
			mySession = session_;
			clientID = clientID_;
			clientType = ClienType_;
			agentPtr = agentPtr_;
		}

		void ConnectionHandler::start()
		{

			Json::Value ready;
			ready["MessageType"] = "Ready";
			Json::FastWriter writer;

			std::string str = writer.write(ready);

			mySession->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,str));
		}

		void ConnectionHandler::readyHandler(const boost::system::error_code &e, std::string str)
		{
			if(e)
			{
				std::cerr << "Connection Not Ready [" << e.message() << "] Trying Again" << std::endl;
				mySession->async_write(str,boost::bind(&ConnectionHandler::readyHandler, this, boost::asio::placeholders::error,str));
			}
			else
			{
				//will not pass 'message' variable as argument coz it
				//is global between functions. some function read into it, another function read from it
				mySession->async_read(message,
					boost::bind(&ConnectionHandler::readHandler, this,
							boost::asio::placeholders::error));
			}
		}

		void ConnectionHandler::readHandler(const boost::system::error_code& e) {

			if(e)
			{
				std::cerr << "Read Fail [" << e.message() << "]" << std::endl;
			}
			else
			{
				//call the receive handler in the broker
				CALL_MEMBER_FN(theBroker, receiveCallBack)(*this,message);

				mySession->async_read(message,
								boost::bind(&ConnectionHandler::readHandler, this,
										boost::asio::placeholders::error));
			}
		}

		void ConnectionHandler::send(std::string str)
		{
			mySession->async_write(str,boost::bind(&ConnectionHandler::sendHandler, this, boost::asio::placeholders::error));
		}
		void ConnectionHandler::sendHandler(const boost::system::error_code& e) {
			std::cout << "Write to agent[" << agentPtr << "]  client["  << clientID << "] " <<(e?"Failed":"Success") << std::endl;
		}

};
