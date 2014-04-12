#include <cstdlib>
#include <deque>
#include <set>
#include <string>
#include <iostream>
#include <queue>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>


//////////////////////////////////////////////////////////////////////////////////////////////////////
//  This is meant as a simple drop-in replacement for realy.go, since I'm not sure if Go is dropping messages (or if it's exhibiting bad performance).
//  It requires two threads per client, and an additional two threads for the server.
////////////////////////////////////////////////////////////////////////////////////////////////////// 


using boost::asio::ip::tcp;
using boost::system::error_code;

class ServerListener;
class ClientListener;

///A minimalist version of Sim Mobility's ThreadSafeQueue. Push/pop() operat atomically on the queue.
template<class T>
class ThreadSafeQueue {
public:
	///Push an item into the queue.
	void push(const T& item) {
		{
		boost::unique_lock<boost::mutex> lock(mutex);
		messageList.push(item);
		}
		cond.notify_all();
	}

	///Pop an item off the queue and store it in res.
	///Returns true if an item was retrieved, false otherwise.
	void pop(T& res) {
		boost::unique_lock<boost::mutex> lock(mutex);
		while (messageList.empty()) {
			cond.wait(lock);
		}
		res = messageList.front();
		messageList.pop();
	}

private:
	std::queue<T> messageList;
	boost::mutex mutex;
	boost::condition_variable cond;
};

//Log all messages?
bool DebugLog = false;

const std::string SM_HOST = "192.168.0.103";
const std::string SM_PORT = "6745";
const std::string LOC_ADDR = "192.168.0.103";
const unsigned int LOC_PORT = 6799;
const unsigned int MAX_MSG_LENGTH = 30000;

//Our endpoints, as a result.
boost::asio::ip::tcp::endpoint client_ep(boost::asio::ip::address::from_string(LOC_ADDR), LOC_PORT);

//Our new client message.
const char* NC_MSG =	"\x01\x01\x01\x01\x00\x00\x00\x1E"  //Static header
							"00" //Sender/Destination (0 only for this message and id_request)
							"\x00\x00\x19" //Length of first (and only) message.
							"{\"msg_type\":\"new_client\"}";  //The actual message
const unsigned int NC_LEN = 38; //Easier this way.



//Pool of byte* buffers, all of size MAX_MSG_LENGT. If empty, a new one is created.
std::vector<char*> freeBuffers;
boost::mutex    freeBuffersLOCK;

//A message
struct FullMsg {
	char* data;
	unsigned int len;
	FullMsg(char* data=0, unsigned int len=0) : data(data), len(len) {}
};

//Actual server connection.
ServerListener* server = 0;
boost::mutex serverLOCK;

//Map of known clients.
std::map<std::string, ClientListener*> clients;
boost::mutex    clientsLOCK;

//List of unknown clients.
std::vector<ClientListener*> unknown;
boost::mutex    unknownLOCK;

//Connect to the server if this is the first time. Returns true if this call forced the server to initialize.
bool init_server();


//Polls listening to a client. Spawns a new ClientListener on each successfull accept()
class ClientListener {
public:
	ClientListener() : socket(io_service) {
		tcp::acceptor acceptor(io_service, client_ep);
		acceptor.accept(socket);
		socket.set_option(tcp::no_delay(true));
		readClientThread = boost::thread(boost::bind(&ClientListener::readIncomingClient, this));
	}

	void push(const FullMsg& msg) {
		readServerBuff.push(msg);
	}

private:
	void sendToServer(const FullMsg& msg); //We need ServerListener defined for this to work.

	void readIncomingClient() {
		//Some initialization.
		std::cout <<"Client contacted relay.\n";

		{
		boost::unique_lock<boost::mutex> lock(unknownLOCK);
		unknown.push_back(this);
		}

		if (!init_server()) {
			//Send a new_client messge.
			FullMsg newClientMsg;
			newClientMsg.data = new char[MAX_MSG_LENGTH]; //This will end up in the pool anyway.
			newClientMsg.len = NC_LEN;
			memcpy(newClientMsg.data, NC_MSG, NC_LEN); 
			sendToServer(newClientMsg);
		}

		//Start another thread for receiving.		
		readServerThread = boost::thread(boost::bind(&ClientListener::readIncomingServer, this));

		//Now read forever.
		for(;;) {
			//Get a buffer
			{
			boost::unique_lock<boost::mutex> lock(freeBuffersLOCK);
			if (!freeBuffers.empty()) {
				readClientBuff = freeBuffers.back();
				freeBuffers.pop_back();
			} else {
				readClientBuff = new char[MAX_MSG_LENGTH];
			}
			}

			//Read the header.
			error_code err;
		   boost::asio::read(socket, boost::asio::buffer(readClientBuff, 8), err);
			if (err) { throw std::runtime_error("Client read error [1]."); }

			//Deocde the remaining length.
			unsigned int rem_len = ((int(readClientBuff[4])&0xFF)<<24) | ((int(readClientBuff[5])&0xFF)<<16) | ((int(readClientBuff[6])&0xFF)<<8) | (int(readClientBuff[7])&0xFF);
			if (rem_len+8 >= MAX_MSG_LENGTH) { throw std::runtime_error("Client message is too long!"); }

			//Read the remaining data (into the same buffer).
			boost::asio::read(socket, boost::asio::buffer((readClientBuff+8), rem_len), err);
			if (err) { throw std::runtime_error("Client read error [2]."); }

			//Save the client ID globally if it's unknown.
			if (clientID.empty()) {
				//Retrieve the sender ID (sendIDLength in byte 2, destIDLength in byte 3, sendID starts after byte 8, and destID after sendID)
				unsigned int sendIDStart = 8;
				unsigned int sendIDLen = int(readClientBuff[1])&0xFF;
				std::string sendId(&readClientBuff[sendIDStart], sendIDLen);
				if (sendId.empty() || sendId=="0") { throw std::runtime_error("send_id from client is 0; this only happens for new_client, which clients themselves don't send."); }
				clientID = sendId;

				boost::unique_lock<boost::mutex> lock(clientsLOCK);
				std::map<std::string, ClientListener*>::const_iterator it=clients.find(sendId);
				if (it!=clients.end()) { throw std::runtime_error("Client with send_id already exists."); }
				clients[sendId] = this;
				std::cout <<"Connected clients: " <<clients.size() <<"\n";
			}

			//Post it into the server.
			sendToServer(FullMsg(readClientBuff, rem_len+8));

			//Avoid errors on the next read.
			readClientBuff = 0;
		}
	}

	void readIncomingServer() {
		FullMsg msg;
		for(;;) {
			//Read from the server buffer (blocking).
			readServerBuff.pop(msg);

			if (DebugLog) {
				std::cout <<"Sent message to client \"" <<clientID <<"\", data: " <<std::string(msg.data, msg.len) <<"\n";
			}
			
			//Push this message to the client.
			error_code err;
			boost::asio::write(socket, boost::asio::buffer(msg.data, msg.len), err);
			if (err) { throw std::runtime_error("Error sending message to client."); }

			//Return the buffer.
			{
			boost::unique_lock<boost::mutex> lock(freeBuffersLOCK);
			freeBuffers.push_back(msg.data);
			}
			msg.data = 0;
			msg.len = 0;
		}
	}


private:
	boost::asio::io_service io_service;
	tcp::socket socket;
	boost::thread readClientThread;
	char* readClientBuff;
	boost::thread readServerThread;
	ThreadSafeQueue<FullMsg> readServerBuff;
	std::string clientID;
};


//Polls listening to the server.
class ServerListener {
public:
	ServerListener() : socket(io_service) {
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(SM_HOST, SM_PORT);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		boost::asio::connect(socket, endpoint_iterator);
		socket.set_option(tcp::no_delay(true));

		std::cout <<"Connected to Sim Mobility server.\n";

		//Spawn our threads
		readServerThread = boost::thread(boost::bind(&ServerListener::readIncomingServer, this));
		readClientThread = boost::thread(boost::bind(&ServerListener::readIncomingClient, this));
	}

	void push(const FullMsg& msg) {
		readClientBuff.push(msg);
	}

private:
	void readIncomingServer() {
		for (;;) {
			//Get a buffer
			{
			boost::unique_lock<boost::mutex> lock(freeBuffersLOCK);
			if (!freeBuffers.empty()) {
				readServerBuff = freeBuffers.back();
				freeBuffers.pop_back();
			} else {
				readServerBuff = new char[MAX_MSG_LENGTH];
			}
			}

			//Read the header.
			error_code err;
		   boost::asio::read(socket, boost::asio::buffer(readServerBuff, 8), err);
			if (err) { throw std::runtime_error("Server read error [1]."); }

			//Deocde the remaining length.
			unsigned int rem_len = ((int(readServerBuff[4])&0xFF)<<24) | ((int(readServerBuff[5])&0xFF)<<16) | ((int(readServerBuff[6])&0xFF)<<8) | (int(readServerBuff[7])&0xFF);
			if (rem_len+8 >= MAX_MSG_LENGTH) { throw std::runtime_error("Server message is too long!"); }

			//Read the remaining data (into the same buffer).
			boost::asio::read(socket, boost::asio::buffer((readServerBuff+8), rem_len), err);
			if (err) { throw std::runtime_error("Server read error [2]."); }

			//Retrieve the destination ID (sendIDLength in byte 2, destIDLength in byte 3, sendID starts after byte 8, and destID after sendID)
			unsigned int destIDStart = 8 + (int(readServerBuff[1])&0xFF);
			unsigned int destIDLen = int(readServerBuff[2])&0xFF;
			std::string destId(&readServerBuff[destIDStart], destIDLen);

			//A destination of 0 is only allowed with a single id_request message (we'll just trust the server/client).
			ClientListener* destClient = 0;
			if (destId=="0") {
				boost::unique_lock<boost::mutex> lock(unknownLOCK);
				if (unknown.empty()) { throw std::runtime_error("No clients are pending in the \"unknown\" array."); }
				destClient = unknown.back();
				unknown.pop_back();
			} else {
				boost::unique_lock<boost::mutex> lock(clientsLOCK);
				std::map<std::string, ClientListener*>::const_iterator it=clients.find(destId);
				if (it==clients.end()) { throw std::runtime_error("No client with this destination ID exists."); }
				destClient = it->second;
			}

			//Post it into the server.
			FullMsg msg(readServerBuff, rem_len+8);
			if (DebugLog) {
				std::cout <<"Received message from server, to client \"" <<destId <<"\", data: " <<std::string(msg.data, msg.len) <<"\n";
			}
			destClient->push(msg);

			//Avoid errors on the next read.
			readServerBuff = 0;
		}
	}


	void readIncomingClient() {
		FullMsg msg;
		for(;;) {
			//Read from the client buffer (blocking).
			readClientBuff.pop(msg);

			if (DebugLog) {
				std::cout <<"Sent message to server, data: " <<std::string(msg.data, msg.len) <<"\n";
			}
			
			//Push this message to the client.
			error_code err;
			boost::asio::write(socket, boost::asio::buffer(msg.data, msg.len), err);
			if (err) { throw std::runtime_error("Error sending message to server."); }

			//Return the buffer.
			{
			boost::unique_lock<boost::mutex> lock(freeBuffersLOCK);
			freeBuffers.push_back(msg.data);
			}
			msg.data = 0;
			msg.len = 0;
		}
	}


private:
	boost::asio::io_service io_service;
	tcp::socket socket;
	boost::thread readServerThread;
	char* readServerBuff;
	boost::thread readClientThread;
	ThreadSafeQueue<FullMsg> readClientBuff;
};


void ClientListener::sendToServer(const FullMsg& msg) 
{
	if (DebugLog) {
		std::cout <<"Received message from client \"" <<clientID <<"\", to server, data: " <<std::string(msg.data, msg.len) <<"\n";
	}

	server->push(msg);
}


bool init_server() 
{
	boost::unique_lock<boost::mutex> lock(serverLOCK);
	if (!server) {
		server = new ServerListener();
		return true;
	}
	return false;
}


int main(int argc, char* argv[])
{
	//Parse args
	if (argc==2 && std::string(argv[1])=="--debug") {
		DebugLog = true;
	} else if (argc != 1) {
		std::cout <<"Usage: ./relay --debug, or ./relay.\nThe \"--debug\" flag will log all message handoffs.\n";
		return 1;
	}

	//Keep starting new ClientListeners
	std::cout <<"Listening for new client connections...\n";
	for (;;) {
		new ClientListener();
	}
	std::cout <<"Done\n";
	return 0;
}


