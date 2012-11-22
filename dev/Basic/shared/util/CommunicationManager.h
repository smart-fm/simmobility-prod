/*
 * CommunicationManager.h
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#ifndef COMMUNICATIONMANAGER_H_
#define COMMUNICATIONMANAGER_H_

#include <queue>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace sim_mob {

using boost::asio::ip::tcp;

class CommunicationManager {

public:
	static CommunicationManager* GetInstance();
	~CommunicationManager();
	void start();
	std::queue<std::string> *getQueue() { return &dataQueue;}
	void sendData(std::string &s) {
		boost::mutex::scoped_lock lock(guard);
		dataQueue.push(s);}
	bool getData(std::string &s) {
		std::cout<<"queue size: "<<dataQueue.size()<<std::endl;
		if(!dataQueue.empty())
		{
			boost::mutex::scoped_lock lock(guard);
			s = dataQueue.front();
			dataQueue.pop();
			return true;
		}
		return false;
	}
	bool isAllDataOut() { return dataQueue.empty(); }
private:
	CommunicationManager();
	static CommunicationManager *instance;
	boost::asio::io_service io_service;
	int listenPort;
private:
	std::queue<std::string> dataQueue;
	boost::mutex guard;
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

  void start()
  {
//	  std::queue<std::string> *dataQueue = CommunicationManager::GetInstance()->getQueue();
	  for(;;)
	  {
//		  std::cout<<"queue size: "<<dataQueue->size()<<std::endl;
		  if(CommunicationManager::GetInstance()->getData(message_))
		  {
			if(socket_.is_open())
			{
				std::ostringstream stream;
				stream<< message_;
				std::string body = "{=" + stream.str()+"=}";

				char msg[20]="\0";
				//head size 12
				sprintf(msg,"{=%08d=}",body.size());
				std::string head=msg;

				message_=head+body;
				std::cout<<"send message: "<<message_<<std::endl;
				boost::system::error_code err;
				 try
				 {
					 size_t iBytesWrit = boost::asio::write(socket_, boost::asio::buffer(message_),boost::asio::transfer_all(),err);
					 //size_t iBytesWrit = boost::asio::write(socket_, boost::asio::buffer(message_),boost::asio::transfer_all());
					 std::cout<<"start: send byte "<<iBytesWrit<<std::endl;
					 if(err)
					 {
						 std::cerr<<"start: send error "<<err.message()<<std::endl;
						 socket_.close();
						 return;
					 }
				 }
				 catch (std::exception& e)
				 {
					 std::cerr <<"start: "<< e.what() << std::endl;
					 socket_.close();
					 return;
				 }

//				 dataQueue->pop();

			}
			else
			{
				std::cout<<"start: socket broken"<<std::endl;
				socket_.close();
				return;
			}
		  }//end of while
		  sleep(0.1);
	  }

//    for(int i=0;i<10;++i)
//    {
//    	if(socket_.is_open())
//    	{
//    		std::ostringstream stream;
//			stream<< "it is data. " ;
//			stream<<i;
//			stream<<"\n";
//			std::string body = "{=" + stream.str()+"=}";
//
//			char msg[2048]="\0";
//			//head size 12
//			sprintf(msg,"{=%08d=}",body.size());
//			std::string head=msg;
//
//			message_=head+body;
//			std::cout<<"send message: "<<message_<<std::endl;
//    		boost::system::error_code err;
//    		 try
//    		 {
//    			 size_t iBytesWrit = boost::asio::write(socket_, boost::asio::buffer(message_),boost::asio::transfer_all(),err);
//    			 //size_t iBytesWrit = boost::asio::write(socket_, boost::asio::buffer(message_),boost::asio::transfer_all());
//    			 std::cout<<"start: send byte "<<iBytesWrit<<std::endl;
//    			 if(err)
//    			 {
//    				 std::cerr<<"start: send error "<<err.message()<<std::endl;
//    				 break;
//    			 }
//    		 }
//    		 catch (std::exception& e)
//    		 {
//    			 std::cerr <<"start: "<< e.what() << std::endl;
//    			 break;
//    		 }
//
//			sleep(1);
//		}
//    	else
//    	{
//    		std::cout<<"start: socket broken"<<std::endl;
//    		break;
//    	}
//    }
  }

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
  std::string message_;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_service& io_service,int port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection =
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
      new_connection->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
};

}
#endif /* COMMUNICATIONMANAGER_H_ */
