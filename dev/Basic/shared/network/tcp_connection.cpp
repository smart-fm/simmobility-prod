/*
 * tcp_connection.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#include "tcp_connection.hpp"

#include "CommunicationDataManager.hpp"
#include "ControlManager.hpp"

using boost::asio::ip::tcp;
using namespace sim_mob;


bool sim_mob::tcp_connection::receiveData(std::string &cmd,std::string &data)
  {
	  // after send data , lets expect the response from visualizer
		int head_len=12;
		boost::array<char, 12> buf;
		socket_.set_option(boost::asio::socket_base::receive_buffer_size(head_len));
		size_t len = boost::asio::read(socket_,boost::asio::buffer(buf,head_len),boost::asio::transfer_at_least(head_len));
		std::string head_data(buf.begin(), buf.end());
		boost::regex head_regex("^\\{\\=(\\d+)\\=\\}$",boost::regex::perl);
		boost::smatch what;
		int body_len=0;
		if( regex_match( head_data, what,head_regex ) )
		{
			  std::string s=what[1];
			  body_len= atoi(s.c_str());
		}
		else
		{
			std::cout<<"bad head: "<<head_data<<std::endl;
			return false;
		}
		// read body
		  if (body_len == 0)
		  {
			  std::cout<< " body len zero "<<std::endl;
			  return false;
		   }
		  char buf_body[2048]="\0";
		  socket_.set_option(boost::asio::socket_base::receive_buffer_size(body_len));
		  len = boost::asio::read(socket_,boost::asio::buffer(buf_body,body_len),boost::asio::transfer_at_least(body_len));
		  std::string data_body_str(buf_body,body_len);
		  boost::regex body_regex("^\\{\\=(.*)\\=\\}\\{\\@\\=(.*)\\=\\@\\}$",boost::regex::perl);
		  if( regex_match( data_body_str, what,body_regex ) )
		  {
			  cmd = what[1];
			  data =what[2];
		  }
		  else
		  {
			  std::cout<<"not good body: "<<data_body_str<<std::endl;
			  return false;
		  }
		return true;
  }
void sim_mob::tcp_connection::trafficDataStart(CommunicationDataManager& comDataMgr)
{
	std::cout<<std::endl;
	std::cout<<"visualizer connected"<<std::endl;
	std::cout<<"simmob"<<">"<<std::flush;
	std::fstream file_output("./log_SimmobTrafficData.txt",std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
	  CommunicationDataManager *com = &comDataMgr;
	  std::string cmd_;
	  std::string message_;
	  for(;;)
	  {
		  if(com->getTrafficData(message_))
		  {
			if(socket_.is_open())
			{

				file_output<<"send begin: "<<make_daytime_string();
				std::string send_cmd = "TRAFFICDATA";
				if(sendData(send_cmd,message_))
				{
					file_output<<message_;
					file_output<<"\n";
					file_output<<"send over: "<<make_daytime_string();
					std::string recv_cmd;
					std::string recv_data;
					if(!receiveData(recv_cmd,recv_data))
					{
						std::cout<<"receive client response err "<<std::endl;
						commDone();
						break;
					}
					else
					{
						file_output<<recv_cmd<<" "<<recv_data<<"\n";
						if(recv_data != "RECEIVED")
						{
							commDone();
							break;
						}
					}
				}
				else
				{
					std::cout<<"send err "<<std::endl;
					commDone();
					break;
				}
			}
			else
			{
				std::cout<<"start: socket broken"<<std::endl;
				commDone();
				return;
			}
		  }//end of if
	  }
	  file_output.flush();
	  file_output.close();
  }
void sim_mob::tcp_connection::cmdDataStart(CommunicationDataManager& comDataMgr, ControlManager& ctrlMgr)
{
	std::fstream file_output2("./logSimmobCmdData2.txt",std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
	file_output2<<"asdfasdfasfdasdf"<<"\n";
	file_output2.flush();
	  CommunicationDataManager *com = &comDataMgr;

	  //
	  fd_set fileDescriptorSet;
	  struct timeval timeStruct;
	  // set the timeout to 30 seconds
	  timeStruct.tv_sec = 0;
	  timeStruct.tv_usec = 0;
	  FD_ZERO(&fileDescriptorSet);
	  int nativeSocket = socket_.native();
	  std::string send_cmd = "COMMAND";
	  std::string message_;
	  for(;;)
	  {
		  // receive cmd from client
		  FD_SET(nativeSocket,&fileDescriptorSet);
		  select(nativeSocket+1,&fileDescriptorSet,NULL,NULL,&timeStruct);
		  if(FD_ISSET(nativeSocket,&fileDescriptorSet))
		  {
			  std::string recv_cmd;
			  std::string recv_data;
			  if(!receiveData(recv_cmd,recv_data))
			  {
				  std::cout<<"receive client response err "<<std::endl;
				commDone();
					break;
			  }
			  else
			{
				  file_output2<<recv_cmd<<" "<<recv_data<<"\n";
				  file_output2.flush();
				ctrlMgr.handleInput(recv_data);
				std::string fk_cmd = "FEEDBACK";
				std::string fk_msg="RECEIVED";
				if(!sendData(fk_cmd,fk_msg))
				{
					commDone();
					break;
				}
			}
		  }
		  // send cmd to client
		  if(com->getCmdData(message_))
		  {
			if(socket_.is_open())
			{

				file_output2<<"send begin: "<<make_daytime_string();
				if(sendData(send_cmd,message_))
				{
					file_output2<<message_;
					file_output2<<"\n";
					file_output2<<"send over: "<<make_daytime_string();
					file_output2.flush();
					std::string recv_cmd;
					std::string recv_data;
					if(!receiveData(recv_cmd,recv_data))
					{
						std::cout<<"receive client response err "<<std::endl;
						commDone();
						break;
					}
					else
					{
						file_output2<<recv_cmd<<" "<<recv_data<<"\n";
						file_output2.flush();
						if(recv_data != "RECEIVED")
						{
							commDone();
							break;
						}
					}
				}
				else
				{
					std::cout<<"send err "<<std::endl;
					commDone();
					break;
				}
			}
			else
			{
				std::cout<<"start: socket broken"<<std::endl;
				commDone();
				return;
			}
		  }//end of if
	  }
	  file_output2.flush();
	  file_output2.close();
  }
void sim_mob::tcp_connection::commDone()
{
	  socket_.close();
}
void sim_mob::tcp_connection::roadNetworkDataStart(CommunicationDataManager& comDataMgr)
{
	  std::ofstream file_output;
	  file_output.open("./logSimmobRoadNetworkData.txt");
	  CommunicationDataManager *com = &comDataMgr;
	  std::string send_cmd = "ROADNETWORK";
	  std::string message_;
	  for(;;)
	  {
		  if(com->getRoadNetworkData(message_))
		  {
			if(socket_.is_open())
			{

				file_output<<"send begin: "<<make_daytime_string();
				if(sendData(send_cmd,message_))
				{
					file_output<<message_;
					file_output<<"\n";
					file_output<<"send over: "<<make_daytime_string();
					file_output.flush();
					std::string recv_cmd;
					std::string recv_data;
					if(!receiveData(recv_cmd,recv_data))
					{
						std::cout<<"roadNetworkDataStart:receive client response err "<<std::endl;
						commDone();
						break;
					}
					else
					{
						file_output<<recv_data<<"\n";
						if(recv_data != "RECEIVED")
						{
							commDone();
							break;
						}
					}
				}
				else
				{
					std::cout<<"roadNetworkDataStart: send err "<<std::endl;
					commDone();
					break;
				}
			}
			else
			{
				std::cout<<"roadNetworkDataStart: socket broken"<<std::endl;
				commDone();
				return;
			}
		  }//end of if
	  }
	  file_output.flush();
	  file_output.close();
}


boost::shared_ptr<tcp_connection> sim_mob::tcp_connection::create(boost::asio::io_service& io_service)
 {
   return boost::shared_ptr<tcp_connection>(new tcp_connection(io_service));
 }

 boost::asio::ip::tcp::socket& sim_mob::tcp_connection::socket()
 {
   return socket_;
 }

 void sim_mob::tcp_connection::handle_write(const boost::system::error_code& error, size_t bytesTransferred)
 {
 }

 std::string sim_mob::tcp_connection::make_daytime_string()
 {
   std::time_t now = std::time(0);
   return std::ctime(&now);
 }

 bool sim_mob::tcp_connection::sendData(std::string &cmd,std::string data)
 {
	  std::string message_;
		std::string body = "{=" + cmd+"=}{@="+data+"=@}";


		//head size 12
		char msg[20]="\0";

		//TODO: Need to test that this works:
		//sprintf(msg,"{=%08d=}",body.size());
		sprintf(msg,"{=%08zu=}",body.size());

		std::string head=msg;

		message_=head+body;
		boost::system::error_code err;
		 try
		 {
			 boost::asio::async_write(socket_, boost::asio::buffer(message_),
									  boost::bind(&tcp_connection::handle_write,shared_from_this(),
									  boost::asio::placeholders::error,
									  boost::asio::placeholders::bytes_transferred));
			 if(err) {
				 std::cerr<<"start: send error "<<err.message()<<std::endl;
				 return false;
			 }
		 }
		 catch (std::exception& e)
		 {
			 std::cerr <<"start: "<< e.what() << std::endl;
			 return false;
		 }

		 return true;
 }


