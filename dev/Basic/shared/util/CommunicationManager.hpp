/*
 * CommunicationManager.h
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#pragma once

#include <queue>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
namespace sim_mob {

class ConfigParams;

using boost::asio::ip::tcp;
enum COMM_COMMAND{
	RECEIVED=1,
	SHUTDOWN=2
};
class CommunicationDataManager
{
public:
	//static CommunicationDataManager* GetInstance();
	void sendTrafficData(std::string &s);
	void sendRoadNetworkData(std::string &s);
	bool getTrafficData(std::string &s);
	bool getCmdData(std::string &s);
	bool getRoadNetworkData(std::string &s);
	bool isAllTrafficDataOut() { return trafficDataQueue.empty(); }
private:
	CommunicationDataManager();
	//static CommunicationDataManager *instance;
	std::queue<std::string> trafficDataQueue;
	std::queue<std::string> cmdDataQueue;
	std::queue<std::string> roadNetworkDataQueue;
	boost::mutex trafficDataGuard;
	boost::mutex cmdDataGuard;
	boost::mutex roadNetworkDataGuard;

	//The CommunicationDataManager should be shared for *all* communication, but we want to avoid
	// making it a singleton. For now, that means we put it in ConfigParams, so ConfigParams needs
	// friend access.
	friend class sim_mob::ConfigParams;
};
class CommunicationManager {

public:
//	static CommunicationManager* GetInstance();
	CommunicationManager(int port, CommunicationDataManager& comDataMgr);
	~CommunicationManager();
	void start();
//	void sendTrafficData(std::string &s);
//	bool getTrafficData(std::string &s);
//	bool getCmdData(std::string &s);
//	bool getRoadNetworkData(std::string &s);
//	bool isAllTrafficDataOut() { return trafficDataQueue.empty(); }
	bool isCommDone() { return CommDone; }
	void setCommDone(bool b) { CommDone = b; }
	void setSimulationDone(bool b) { simulationDone = b; }
	bool isSimulationDone() { return simulationDone; }
private:
//	CommunicationManager();
//	static CommunicationManager *instance;
	CommunicationDataManager* comDataMgr;
	boost::asio::io_service io_service;
	int listenPort;
private:
//	std::queue<std::string> trafficDataQueue;
//	std::queue<std::string> cmdDataQueue;
//	std::queue<std::string> roadNetworkDataQueue;
//	boost::mutex trafficDataGuard;
//	boost::mutex cmdDataGuard;
//	boost::mutex roadNetworkDataGuard;
	bool simulationDone;
	bool CommDone;
};

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new tcp_connection(io_service));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void handle_write(const boost::system::error_code& /*error*/,
        size_t /*bytes_transferred*/)
    {
//	  std::cout<<"write: "<<std::endl;
    }
  std::string make_daytime_string()
  {
    using namespace std; // For time_t, time and ctime;
    time_t now = std::time(0);
    return std::ctime(&now);
  }
  void commDone();
//  {
//	  socket_.close();
//	  CommunicationManager::GetInstance()->setCommDone(true);
//  }
  bool sendData(std::string &cmd,std::string data)
  {
//	  std::ostringstream stream;
//		stream<< data;
	  std::string message_;
		std::string body = "{=" + cmd+"=}{@="+data+"=@}";


		char msg[20]="\0";
		//head size 12

		//TODO: Need to test that this works:
		//sprintf(msg,"{=%08d=}",body.size());
		sprintf(msg,"{=%08zu=}",body.size());

		std::string head=msg;

		message_=head+body;
	//				std::cout<<"send message: "<<message_<<std::endl;
//		file_output<<"send begin: "<<make_daytime_string();
//		file_output<<message_;
//		file_output<<"\n";
		boost::system::error_code err;
		 try
		 {
	//					 size_t iBytesWrit = boost::asio::write(socket_, boost::asio::buffer(message_),boost::asio::transfer_all(),err);
			 boost::asio::async_write(socket_, boost::asio::buffer(message_),
									  boost::bind(&tcp_connection::handle_write,shared_from_this(),
									  boost::asio::placeholders::error,
									  boost::asio::placeholders::bytes_transferred));
	//					 std::cout<<"start: send byte "<<iBytesWrit<<std::endl;
//			 file_output<<"send over: "<<make_daytime_string();
			 if(err)
			 {
				 std::cerr<<"start: send error "<<err.message()<<std::endl;
	//						 socket_.close();
//				 commDone();
				 return false;
			 }
		 }
		 catch (std::exception& e)
		 {
			 std::cerr <<"start: "<< e.what() << std::endl;
	//					 socket_.close();
//			 commDone();
			 return false;
		 }

		 return true;
  }
  bool receiveData(std::string &cmd,std::string &data);
//  {
//	  // after send data , lets expect the response from visualizer
//		int head_len=12;
//		boost::array<char, 12> buf;
//		socket_.set_option(boost::asio::socket_base::receive_buffer_size(head_len));
//		size_t len = boost::asio::read(socket_,boost::asio::buffer(buf,head_len),boost::asio::transfer_at_least(head_len));
//		std::string head_data(buf.begin(), buf.end());
////		file_output<<data<<"\n";
//		boost::regex head_regex("^\\{\\=(\\d+)\\=\\}$",boost::regex::perl);
//		boost::smatch what;
//		int body_len=0;
//		if( regex_match( head_data, what,head_regex ) )
//		{
//			  std::string s=what[1];
//			  body_len= atoi(s.c_str());
////			if (RECEIVED != atoi(s.c_str()))
////			{
//////						std::cout<<"unknown res "<<std::endl;
//////				file_output<<"unknown res "<<"\n";
//////						socket_.close();
////				commDone();
////				return false;
////			}
//		}
//		else
//		{
//			std::cout<<"bad head: "<<head_data<<std::endl;
////			file_output<<"bad res "<<"\n";
////					socket_.close();
////			commDone();
//			return false;
//		}
//		// read body
//		  if (body_len == 0)
//		  {
//			  std::cout<< " body len zero "<<std::endl;
//	//		  break;
////			  commDone();
//			  return false;
//		   }
//		  char buf_body[2048]="\0";
//		  socket_.set_option(boost::asio::socket_base::receive_buffer_size(body_len));
//		//    	  len = socket.read_some(boost::asio::buffer(buf_body), error);
//		  len = boost::asio::read(socket_,boost::asio::buffer(buf_body,body_len),boost::asio::transfer_at_least(body_len));
//		//    	  std::cout<<" read body len: "<<len<<std::endl;
//		  std::string data_body_str(buf_body,body_len);
////		  boost::regex body_regex("^\\{\\=(.*)\\=\\}$",boost::regex::perl);
//		  boost::regex body_regex("^\\{\\=(.*)\\=\\}\\{\\@\\=(.*)\\=\\@\\}$",boost::regex::perl);
//	//	  file_output<<data_body_str<<"\n";
//		  if( regex_match( data_body_str, what,body_regex ) )
//		  {
//			  cmd = what[1];
//			  data =what[2];
//		//		  file_output<<s<<"\n";
//		  }
//		  else
//		  {
//			  std::cout<<"not good body: "<<data_body_str<<std::endl;
//	//		  break;
//			  return false;
//		  }
//		return true;
//  }
  void trafficDataStart(CommunicationDataManager& comDataMgr);
  void cmdDataStart(CommunicationDataManager& comDataMgr);
  void roadNetworkDataStart(CommunicationDataManager& comDataMgr);

private:
  tcp_connection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

//  void handle_write(const boost::system::error_code& error,
//      size_t bytes_transferred)
//  {
//	  if (error)
//		  std::cout<<"handle_write: "<<error.message()<<" "<<error.value()<<" "<<" send bytes: "<<bytes_transferred<<std::endl;
//	  else
//	  {
//		  std::cout<<"handle_write: "<<error.message()<<" "<<" send bytes: "<<bytes_transferred<<std::endl;
//		  boost::system::error_code ec;
//		  socket_.close(ec);
//		  if (ec)
//		  {
//		    std::cout<<"socket close error: "<<ec.message()<<std::endl;
//		  }
//	  }
//  }

  tcp::socket socket_;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_service& io_service,int port, CommunicationDataManager& comDataMgr)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), comDataMgr(&comDataMgr)
  {
	 myPort=port;
    start_accept();
  }
  tcp_connection::pointer new_connection;
  bool isClientConnect() { return new_connection->socket().is_open();}
private:
  CommunicationDataManager* comDataMgr;
  int myPort;
  void start_accept()
  {
    new_connection =
      tcp_connection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error)
  {
    if (!error)
    {
    	if(myPort==13333)
    	{
    		new_connection->trafficDataStart(*comDataMgr);
    	}
    	else if(myPort==13334)
		{
			new_connection->cmdDataStart(*comDataMgr);
		}
    	else if(myPort==13335)
		{
			new_connection->roadNetworkDataStart(*comDataMgr);
		}
    	else
    	{
    		std::cout<<"handle_accept: what port it is? "<<myPort<<std::endl;
    	}

    }

    start_accept();
  }

  tcp::acceptor acceptor_;
};

}
