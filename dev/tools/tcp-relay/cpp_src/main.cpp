#include <cstdlib>
#include <deque>
#include <set>
#include <list>
#include <string>
#include <sstream>
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


//Log all messages?
bool DebugLog = false;

const std::string SM_HOST = "192.168.0.103";
const std::string SM_PORT = "6745";
const std::string LOC_ADDR = "192.168.0.103";
const unsigned int LOC_PORT = 6799;
const unsigned int MAX_MSG_LENGTH = 30000;
const unsigned int CONGLOM_UPPER_LIMIT = MAX_MSG_LENGTH * 36; //How much space the server allocates in its send buffer.


//Our endpoints, as a result.
boost::asio::ip::tcp::endpoint client_ep(boost::asio::ip::address::from_string(LOC_ADDR), LOC_PORT);

//Only 1 of each of these.
boost::asio::io_service io_service;
tcp::acceptor acceptor(io_service, client_ep);
tcp::resolver resolver(io_service);
tcp::resolver::query query(SM_HOST, SM_PORT);
tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

//Our new client message.
const char* NC_MSG =	"\x01\x01\x01\x01\x00\x00\x00\x1E"  //Static header
							"00" //Sender/Destination (0 only for this message and id_request)
							"\x00\x00\x19" //Length of first (and only) message.
							"{\"msg_type\":\"new_client\"}";  //The actual message
const unsigned int NC_LEN = 38; //Easier this way.

//Actual server connection.
ServerListener* server = 0;

//Map of known clients.
std::map<std::string, ClientListener*> clients;

//List of unknown clients.
std::vector<ClientListener*> unknown;

//Connect to the server if this is the first time. Returns true if this call forced the server to initialize.
bool init_server();


//Polls listening to a client. Spawns a new ClientListener on each successfull accept()
class ClientListener {
public:
	ClientListener() : socket(io_service) {
		acceptor.async_accept(socket, boost::bind(&ClientListener::handle_accept, this, boost::asio::placeholders::error));
		//acceptor.accept(socket);
	}

	void push(const char* msg, unsigned int len) {
		//Add the message.
		bool alreadyWriting = !writeQueue.empty();
		writeQueue.push(std::string(msg, len));

		//"Wake" if this is the first new message in the queue.
		if (!alreadyWriting) {
			writeFrontMessage();
		}
	}

private:
	void handle_accept(const error_code& err) {
		if (err) { throw std::runtime_error("Error handling accept, client."); }
		socket.set_option(tcp::no_delay(true));

		//Start listening for the next client.
		new ClientListener();

		readIncomingClient();
	}

	void readIncomingClient() {
		//Some initialization.
		unknown.push_back(this);
		std::cout <<"Connected clients: " <<clients.size() <<" of " <<clients.size()+unknown.size() <<"\n";

		if (!init_server()) {
			//Send a new_client messge.
			sendToServer(NC_MSG, NC_LEN);
		}

		//Read a message.
		readHeader();
	}

	void readHeader() {
		boost::asio::async_read(socket, boost::asio::buffer(readClientBuff, 8), boost::bind(&ClientListener::handle_read_header, this, boost::asio::placeholders::error));
	}

	void handle_read_header(const error_code& err) {
		if (err) { throw std::runtime_error("Error reading header, client."); }

		//Deocde the remaining length.
		rem_len = ((int(readClientBuff[4])&0xFF)<<24) | ((int(readClientBuff[5])&0xFF)<<16) | ((int(readClientBuff[6])&0xFF)<<8) | (int(readClientBuff[7])&0xFF);
		if (rem_len+8 >= MAX_MSG_LENGTH) { throw std::runtime_error("Client message is too long!"); }

		//Now read the data section (into the same buffer).
		boost::asio::async_read(socket, boost::asio::buffer((readClientBuff+8), rem_len), boost::bind(&ClientListener::handle_read_data, this, boost::asio::placeholders::error));
	}

	void handle_read_data(const error_code& err) {
		if (err) { throw std::runtime_error("Error reading data, client."); }

		//Save the client ID globally if it's unknown.
		if (clientID.empty()) {
			//Retrieve the sender ID (sendIDLength in byte 2, destIDLength in byte 3, sendID starts after byte 8, and destID after sendID)
			unsigned int sendIDStart = 8;
			unsigned int sendIDLen = int(readClientBuff[1])&0xFF;
			std::string sendId(&readClientBuff[sendIDStart], sendIDLen);
			if (sendId.empty() || sendId=="0") { throw std::runtime_error("send_id from client is 0; this only happens for new_client, which clients themselves don't send."); }
			clientID = sendId;

			std::map<std::string, ClientListener*>::const_iterator it=clients.find(sendId);
			if (it!=clients.end()) { throw std::runtime_error("Client with send_id already exists."); }
			clients[sendId] = this;
			std::cout <<"Connected clients: " <<clients.size() <<" of " <<clients.size()+unknown.size() <<"\n";
		}

		//Post it to the server (as a string copy).
		sendToServer(readClientBuff, rem_len+8);

		//Now read a new header.
		readHeader();
	}

	void writeFrontMessage() {
		boost::asio::async_write(socket, boost::asio::buffer(writeQueue.front()), boost::bind(&ClientListener::handle_write, this, boost::asio::placeholders::error));
	}

	void handle_write(const error_code& err) {
		if (err) { throw std::runtime_error("Error writing to client."); }

		//Remove this message; it's been written correctly.
		writeQueue.pop();

		//Is there anything else in the queue to write?
      if (!writeQueue.empty()) {
			writeFrontMessage();
      }
	}

	void sendToServer(const char* msg, unsigned int len); //We need ServerListener defined for this to work.

private:
	tcp::socket socket;

	char readClientBuff[MAX_MSG_LENGTH];
	unsigned int rem_len;

	std::queue<std::string> writeQueue;
	std::string clientID;
};


//Polls listening to the server.
class ServerListener {
public:
	ServerListener() : socket(io_service) {
		toServerBuff = write_queue_1;
		toServerLen = 0;
		alreadyWriting = false;
		boost::asio::async_connect(socket, endpoint_iterator, boost::bind(&ServerListener::handle_connect, this, boost::asio::placeholders::error));
	}

	void push(const char* msg, unsigned int len) {
		//Do we have enough room to add this?
		if (toServerLen+len >= CONGLOM_UPPER_LIMIT) { throw std::runtime_error("Server buffer ran out of space."); }
//		bool alreadyWriting = !outgoing.empty();

		//Add the message.
//		outgoing.push(std::string(msg, len));
		memcpy(&toServerBuff[toServerLen], msg, len); //sizeof(char) is 1, so just len is needed.
		toServerLen += len;

		//"Wake" if this is the first new message in the queue.
		if (!alreadyWriting) {
			writeFrontMessage();
		}
	}

private:
	void handle_connect(const error_code& err) {
		if (err) { throw std::runtime_error("Error connecting to server."); }
		socket.set_option(tcp::no_delay(true));

		std::cout <<"Connected to Sim Mobility server.\n";
		readIncomingServer();
	}

	void writeFrontMessage() {
		//TODO: Test writing "all currently available" messages in the queue (maybe use a stringstream, and pass the char* in directly?)
		//currWriting = toWrite.str();
		//toWrite.str("");
		alreadyWriting = true;
		boost::asio::async_write(socket, boost::asio::buffer(toServerBuff, toServerLen), boost::bind(&ServerListener::handle_write, this, boost::asio::placeholders::error));
		toServerBuff = (toServerBuff==write_queue_1) ? write_queue_2 : write_queue_1;
		toServerLen = 0;
	}

	void handle_write(const error_code& err) {
		if (err) { throw std::runtime_error("Error writing to server."); }

		//Remove this message; it's been written correctly.
		//outgoing.pop();
		//writeQueue.pop_front();
		//writeQueue.erase(writeQueue.begin());
//		writeQueue->clear();

		//Is there anything else in the queue to write?
		alreadyWriting = false;
      if (toServerLen>0) {
			writeFrontMessage();
      }
	}

	void readIncomingServer() {
		//Read a message.
		readHeader();
	}

	void readHeader() {
		boost::asio::async_read(socket, boost::asio::buffer(readServerBuff, 8), boost::bind(&ServerListener::handle_read_header, this, boost::asio::placeholders::error));
	}

	void handle_read_header(const error_code& err) {
		if (err) { throw std::runtime_error("Error reading header, server."); }

		//Deocde the remaining length.
		rem_len = ((int(readServerBuff[4])&0xFF)<<24) | ((int(readServerBuff[5])&0xFF)<<16) | ((int(readServerBuff[6])&0xFF)<<8) | (int(readServerBuff[7])&0xFF);
		if (rem_len+8 >= MAX_MSG_LENGTH) { throw std::runtime_error("Server message is too long!"); }

		//Now read the data section (into the same buffer).
		boost::asio::async_read(socket, boost::asio::buffer((readServerBuff+8), rem_len), boost::bind(&ServerListener::handle_read_data, this, boost::asio::placeholders::error));
	}

	void handle_read_data(const error_code& err) {
		if (err) { throw std::runtime_error("Error reading data, server."); }

		//Retrieve the destination ID (sendIDLength in byte 2, destIDLength in byte 3, sendID starts after byte 8, and destID after sendID)
		unsigned int destIDStart = 8 + (int(readServerBuff[1])&0xFF);
		unsigned int destIDLen = int(readServerBuff[2])&0xFF;
		std::string destId(&readServerBuff[destIDStart], destIDLen);

		//A destination of 0 is only allowed with a single id_request message (we'll just trust the server/client).
		ClientListener* destClient = 0;
		if (destId=="0") {
//std::cout <<"Unknown ID of 0 (size: " <<unknown.size() <<" ) with data: " <<std::string(readServerBuff, rem_len+8) <<"\n";
			if (unknown.empty()) { throw std::runtime_error("No clients are pending in the \"unknown\" array."); }
			destClient = unknown.back();
			unknown.pop_back();
		} else {
			std::map<std::string, ClientListener*>::const_iterator it=clients.find(destId);
			if (it==clients.end()) { throw std::runtime_error("No client with this destination ID exists."); }
			destClient = it->second;
		}

		//Post it to the client (as a string copy).
		if (DebugLog) {
			std::cout <<"Received message from server, to client \"" <<destId <<"\", data: " <<std::string(readServerBuff, rem_len+8) <<"\n";
		}
		destClient->push(readServerBuff, rem_len+8);

		//Now read a new header.
		readHeader();
	}

private:
	tcp::socket socket;
	unsigned int rem_len;
	char readServerBuff[MAX_MSG_LENGTH];

//	std::string currWriting;
//	std::stringstream toWrite;
//	std::queue<std::string> outgoing;
	char* toServerBuff; //Current write queue
	unsigned int toServerLen; //Current "start offset" for the next item.
	char write_queue_1[CONGLOM_UPPER_LIMIT];
	char write_queue_2[CONGLOM_UPPER_LIMIT];
	bool alreadyWriting;
};


void ClientListener::sendToServer(const char* msg, unsigned int len) 
{
	if (DebugLog) {
		std::cout <<"Received message from client \"" <<clientID <<"\", to server, data: " <<std::string(msg, len) <<"\n";
	}

	server->push(msg, len);
}


bool init_server() 
{
	if (!server) {
		server = new ServerListener();
		return true;
	}
	return false;
}


int main(int argc, char* argv[])
{
	//Pretty sure this is guaranteed.
	if (sizeof(char) != 1) { throw std::runtime_error("sizeof(char) must be 1!"); }

	//Parse args
	if (argc==2 && std::string(argv[1])=="--debug") {
		DebugLog = true;
	} else if (argc != 1) {
		std::cout <<"Usage: ./relay --debug, or ./relay.\nThe \"--debug\" flag will log all message handoffs.\n";
		return 1;
	}

	//Start a new ClientListener.
	new ClientListener();

	//Perform all I/O
	io_service.run();

	std::cout <<"Done\n";
	return 0;
}


