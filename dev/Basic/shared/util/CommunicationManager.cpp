/*
 * CommunicationManager.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: redheli
 */

#include "CommunicationManager.h"
#include <boost/thread.hpp>
#include "ControlManager.h"

//sim_mob::CommunicationManager* sim_mob::CommunicationManager::instance = NULL;
sim_mob::CommunicationDataManager* sim_mob::CommunicationDataManager::instance = NULL;
//std::queue<std::string> sim_mob::CommunicationManager::dataQueue = NULL;

sim_mob::CommunicationDataManager* sim_mob::CommunicationDataManager::GetInstance() {
     if (!instance) {
          instance = new CommunicationDataManager();
     }
     return instance;
}

sim_mob::CommunicationDataManager::CommunicationDataManager() {
}
void sim_mob::CommunicationDataManager::sendTrafficData(std::string &s)
{
		boost::mutex::scoped_lock lock(trafficDataGuard);
		trafficDataQueue.push(s);
}
bool sim_mob::CommunicationDataManager::getTrafficData(std::string &s) {
//		std::cout<<"queue size: "<<dataQueue.size()<<std::endl;
		if(!trafficDataQueue.empty())
		{
			boost::mutex::scoped_lock lock(trafficDataGuard);
			s = trafficDataQueue.front();
			trafficDataQueue.pop();
			return true;
		}
		return false;
}
bool sim_mob::CommunicationDataManager::getCmdData(std::string &s) {
//		std::cout<<"queue size: "<<dataQueue.size()<<std::endl;
		if(!cmdDataQueue.empty())
		{
			boost::mutex::scoped_lock lock(cmdDataGuard);
			s = cmdDataQueue.front();
			cmdDataQueue.pop();
			return true;
		}
		return false;
}
bool sim_mob::CommunicationDataManager::getRoadNetworkData(std::string &s) {
//		std::cout<<"queue size: "<<dataQueue.size()<<std::endl;
		if(!roadNetworkDataQueue.empty())
		{
			boost::mutex::scoped_lock lock(roadNetworkDataGuard);
			s = roadNetworkDataQueue.front();
			roadNetworkDataQueue.pop();
			return true;
		}
		return false;
}
sim_mob::CommunicationManager::CommunicationManager(int port) {
	listenPort = port;
	simulationDone = false;
	CommDone = true;
}

void sim_mob::CommunicationManager::start()
{
	try
  {
	tcp_server server(io_service,listenPort);
	io_service.run();
  }
  catch (std::exception& e)
  {
	std::cerr <<"CommunicationManager::start: "<< e.what() << std::endl;
  }
}

sim_mob::CommunicationManager::~CommunicationManager() {
	// TODO Auto-generated destructor stub
}
bool sim_mob::tcp_connection::receiveData(std::string &cmd,std::string &data)
  {
	  // after send data , lets expect the response from visualizer
		int head_len=12;
		boost::array<char, 12> buf;
		socket_.set_option(boost::asio::socket_base::receive_buffer_size(head_len));
		size_t len = boost::asio::read(socket_,boost::asio::buffer(buf,head_len),boost::asio::transfer_at_least(head_len));
		std::string head_data(buf.begin(), buf.end());
//		file_output<<data<<"\n";
		boost::regex head_regex("^\\{\\=(\\d+)\\=\\}$",boost::regex::perl);
		boost::smatch what;
		int body_len=0;
		if( regex_match( head_data, what,head_regex ) )
		{
			  std::string s=what[1];
			  body_len= atoi(s.c_str());
//			if (RECEIVED != atoi(s.c_str()))
//			{
////						std::cout<<"unknown res "<<std::endl;
////				file_output<<"unknown res "<<"\n";
////						socket_.close();
//				commDone();
//				return false;
//			}
		}
		else
		{
			std::cout<<"bad head: "<<head_data<<std::endl;
//			file_output<<"bad res "<<"\n";
//					socket_.close();
//			commDone();
			return false;
		}
		// read body
		  if (body_len == 0)
		  {
			  std::cout<< " body len zero "<<std::endl;
	//		  break;
//			  commDone();
			  return false;
		   }
		  char buf_body[2048]="\0";
		  socket_.set_option(boost::asio::socket_base::receive_buffer_size(body_len));
		//    	  len = socket.read_some(boost::asio::buffer(buf_body), error);
		  len = boost::asio::read(socket_,boost::asio::buffer(buf_body,body_len),boost::asio::transfer_at_least(body_len));
		//    	  std::cout<<" read body len: "<<len<<std::endl;
		  std::string data_body_str(buf_body,body_len);
//		  boost::regex body_regex("^\\{\\=(.*)\\=\\}$",boost::regex::perl);
		  boost::regex body_regex("^\\{\\=(.*)\\=\\}\\{\\@\\=(.*)\\=\\@\\}$",boost::regex::perl);
	//	  file_output<<data_body_str<<"\n";
		  if( regex_match( data_body_str, what,body_regex ) )
		  {
			  cmd = what[1];
			  data =what[2];
		//		  file_output<<s<<"\n";
		  }
		  else
		  {
			  std::cout<<"not good body: "<<data_body_str<<std::endl;
	//		  break;
			  return false;
		  }
		return true;
  }
void sim_mob::tcp_connection::trafficDataStart()
{
	  std::ofstream file_output;
	  file_output.open("./logSimmobTrafficData.txt");
	  CommunicationDataManager *com = CommunicationDataManager::GetInstance() ;
//	  com->GetInstance()->setCommDone(false);
	  std::string cmd_;
	  std::string message_;
	  for(;;)
	  {
//		  std::cout<<"queue size: "<<dataQueue->size()<<std::endl;
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
//				socket_.close();
				commDone();
				return;
			}
		  }//end of if
//		  else
//		  {
//			  if(com->isSimulationDone())
//			  {
//				  std::cout<<"start: simulation done and all data out"<<std::endl;
//				  std::string str="SHUTDOWN";
//				  file_output<<str<<"\n";
//				  sendData(str);
//				  sleep(1);
//				  commDone();
//				  return;
//			  }
//		  }
//		  sleep(0.001);
	  }
	  file_output.close();
  }
void sim_mob::tcp_connection::cmdDataStart()
{
	  std::ofstream file_output;
	  file_output.open("./logSimmobCmdData.txt");
	  CommunicationDataManager *com = CommunicationDataManager::GetInstance() ;
//	  com->GetInstance()->setCommDone(false);
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
//		  std::cout<<"queue size: "<<dataQueue->size()<<std::endl;
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
				file_output<<recv_cmd<<" "<<recv_data<<"\n";
				ControlManager::GetInstance()->handleInput(recv_data);
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

				file_output<<"send begin: "<<make_daytime_string();
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
//				socket_.close();
				commDone();
				return;
			}
		  }//end of if
//		  else
//		  {
//			  if(com->isSimulationDone())
//			  {
//				  std::cout<<"start: simulation done and all data out"<<std::endl;
//				  std::string str="SHUTDOWN";
//				  file_output<<str<<"\n";
//				  sendData(str);
//				  sleep(1);
//				  commDone();
//				  return;
//			  }
//		  }
//		  sleep(0.001);
	  }
	  file_output.close();
  }
void sim_mob::tcp_connection::commDone()
{
	  socket_.close();
//	  CommunicationManager::GetInstance()->setCommDone(true);
}
void sim_mob::tcp_connection::roadNetworkDataStart()
{
	  std::ofstream file_output;
	  file_output.open("./logSimmobRoadNetworkData.txt");
	  CommunicationDataManager *com = CommunicationDataManager::GetInstance() ;
//	  com->GetInstance()->setCommDone(false);
	  std::string send_cmd = "ROADNETWORK";
	  std::string message_;
	  for(;;)
	  {
//		  std::cout<<"queue size: "<<dataQueue->size()<<std::endl;
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
					std::cout<<"send err "<<std::endl;
					commDone();
					break;
				}
			}
			else
			{
				std::cout<<"start: socket broken"<<std::endl;
//				socket_.close();
				commDone();
				return;
			}
		  }//end of if
//		  else
//		  {
//			  if(com->isSimulationDone())
//			  {
//				  std::cout<<"start: simulation done and all data out"<<std::endl;
//				  std::string str="SHUTDOWN";
//				  file_output<<str<<"\n";
//				  sendData(str);
//				  sleep(1);
//				  commDone();
//				  return;
//			  }
//		  }
//		  sleep(0.001);
	  }
	  file_output.close();
  }
