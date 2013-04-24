#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <time.h>
#include "session.hpp"
boost::asio::io_service io_service;
boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 2013);
boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
//boost::asio::ip::tcp::socket sock(io_service);
std::string data = "Hello, world!";
namespace sim_mob
{
class Observer;
std::vector<Observer*> observers;

class Observer
{
public:
    Observer(boost::asio::ip::tcp::socket *socket_):socket_obs(socket_){}
    void notify(std::string data)
    {
        std::cout << "notify called data[" << data << "]" << std::endl;
        boost::asio::async_write(*socket_obs, boost::asio::buffer(data) , boost::bind(&Observer::write_handler, this,boost::asio::placeholders::error));
    }
    void write_handler(const boost::system::error_code &ec)
    {
         if (!ec) //no error: done, just wait for the next notification
             return;
         socket_obs->close(); //client will get error and exit its read_handler
         observers.erase(std::find(observers.begin(), observers.end(),this));
         std::cout << "Observer::write_handler  returns as nothing was written" << std::endl;
    }
private:
        boost::asio::ip::tcp::socket *socket_obs;
};


class server
{
	std::map<unsigned int, sim_mob::session> sessions;
public:
     void CreatSocketAndAccept()
     {
    	    // Start an accept operation for a new connection.
    	    connection_ptr new_conn(new connection(acceptor_.get_io_service()));
//    	    new_conn->socket().remote_endpoint().
    	    acceptor_.async_accept(new_conn->socket(),
    	        boost::bind(&server::handle_accept, this,
    	          boost::asio::placeholders::error, new_conn));
     }
      server(boost::asio::io_service& io_service)
      {
          acceptor.listen();
          CreatSocketAndAccept();
      }

      void handle_accept(const boost::system::error_code& e, connection_ptr conn)
      {
    	    if (!e)
    	    {

    	    }
          CreatSocketAndAccept();
      }
private:
  boost::asio::ip::tcp::acceptor acceptor_;
};

class Agent
{
public:
    void update(std::string data)
    {
        if(!observers.empty())
        {
//          std::cout << "calling notify data[" << data << "]" << std::endl;
            observers[0]->notify(data);
        }
    }

};
Agent agent;
void AgentSim()
{
    int i = 0;
    sleep(10);//wait for me to start client
    while(i++ < 10)
    {
        std::ostringstream out("");
        out << data << i ;
//      std::cout << "calling update data[" << out.str() << "]" << std::endl;
        agent.update(out.str());
        sleep(1);
    }
}
void run()
{
    io_service.run();
    std::cout << "io_service returned" << std::endl;
}
}//namespace sim_mob
