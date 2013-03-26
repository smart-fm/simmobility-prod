#pragma once
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/set.hpp>
#include <boost/thread/locks.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include "buffering/Shared.hpp"


namespace sim_mob
{
class Entity;
//THIS ENUM IDENTIFIES THE CLASSES TYPES

enum DataClassType
{
	DT_STRING = 1
};
//the following class will be the base class for any sending/receiving data class
//remember the following lines must always be present in your classes
//1.friend class boost::serialization::access;
//2. template<class Archive>
//3. void serialize(Archive &ar, const unsigned int version)
//4. {
//5.	 ar & boost::serialization::base_object<dataMessage>(*this);
//6.     ar & YOUR DATA_1;
//7.     ar & YOUR DATA_2;
//8.     etc
//9. }

class dataMessage
{

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
//        ar & type;
        ar & str;
        ar & sender;
        ar & receiver;
    }
public:


//	DataClassType type;
	std::string str;
	//a proper size of integer is used coz in case of the normal pointer,
	//the serilizer serializes the object that the pointer is pointing to
	//whereas we just need to save the 'pointer' to object so that we
	//can use it as a refernce at the other end. so instead of storing 'sim_mob::Entity *'
	//we store:
#if __x86_64__
	unsigned long sender;
	unsigned long receiver;
#else
	unsigned int sender;
	unsigned int receiver;

#endif


	dataMessage(){
		sender = receiver = 0;
		str = "";
	}
	//uncomment after debugging
//	void setDataClassType();
	virtual void registerType(boost::archive::text_oarchive &oa)
    {
    	oa.register_type(static_cast<dataMessage *>(NULL));
    }

	virtual void registerType(boost::archive::text_iarchive &ia)
    {
		ia.register_type(static_cast<dataMessage *>(NULL));
    }
};

/**************************************************
 * MSG_DATA and its derivatives will be the data type used for
 * incoming and outgoing data messages
 ***************************************************/
typedef dataMessage DATA_MSG;
typedef DATA_MSG* DATA_MSG_PTR;
typedef DATA_MSG& DATA_MSG_REF;


/**************************************************
 SAMPLE MESSAGE
 Add your structure of data-for agent communications- here.
 the methods in sample_DataMessage_Class are mandatory
 ***************************************************/

class sample_DataMessage_Class : public DATA_MSG
{

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<DATA_MSG>(*this);//in this case the direct parent of sample_DataMessage_Class is DATA_MSG.
//        ar & your_data;
    }

//    void *your_data;
public:

    virtual void registerType(boost::archive::text_oarchive &oa)
    {
    	oa.register_type(static_cast<sample_DataMessage_Class *>(NULL));
    }

	virtual void registerType(boost::archive::text_iarchive &ia)
    {
		ia.register_type(static_cast<sample_DataMessage_Class *>(NULL));
    }
};

/*********************************************************************************
 *
 * ************************ add your data classes here****************************
 *
 *********************************************************************************/











/*********************************************************************************
 *
 * The following is just a small helper structure for sending subscription information from
 * common agents to communicator.
 * It contains references to members of CommunicationSupport (which is a sub-class
 * of common agents who are willing to have communication ability)
 *
 *********************************************************************************/
typedef boost::shared_mutex Lock;
typedef boost::unique_lock< Lock > WriteLock;
typedef boost::shared_lock< Lock > ReadLock;

class subscriptionInfo
{

	 sim_mob::Entity * agent;

	//good for avoiding unnecessary writes
	bool& agentUpdateDone;      //All the operations inside the update method of the 'communicating' agent were done
	//the following flags allow access to the incoming and outgoing buffers by bothe owner(communicating agent) and communicator agent without imposing any lock on the buffers
	bool& incomingIsDirty;     //there is something in the incoming buffer (buffer is written by 'communicator' agent; to be read by the 'communicating' agent)
	bool& outgoingIsDirty;		//there is something in the outgoing buffer (buffer is written by 'communicating' agent; to be read by the 'communicator' agent)
	//------------------------------------
	bool& writeIncomingDone;     //the 'communicator' agent updated/write_to the incoming buffer of the 'communicating' agent
	bool& readOutgoingDone;      //the 'communicator' agent  read the outgoing buffer of the 'communicating' agent


	std::vector<DATA_MSG_PTR> &incoming;
	std::vector<DATA_MSG_PTR> &outgoing;
	bool goodForProcessing;

public:
	boost::shared_ptr<Lock> myLock;//this class is not copy constructible. so we need to use sort of a pointer(i use boost shared pointer)
	//general purpose counter
	int cnt_1;//i use this one to control/limit the number of times communicator faces the 'update not done'
	int cnt_2;

	subscriptionInfo(
			sim_mob::Entity *  agent_ ,
			bool& incomingIsDirty_ ,
			bool& outgoingIsDirty_ ,
			bool& writeIncomingDone_,
			bool& readOutgoingDone_ ,
			bool& agentUpdateDone_,
			std::vector<DATA_MSG_PTR>& incoming_,
			std::vector<DATA_MSG_PTR>& outgoing_

			)
	:
		agent(agent_),
		incomingIsDirty(incomingIsDirty_),
		outgoingIsDirty(outgoingIsDirty_),
		writeIncomingDone(writeIncomingDone_),
		readOutgoingDone(readOutgoingDone_),
		agentUpdateDone(agentUpdateDone_),
		incoming(incoming_),
		outgoing(outgoing_)
	{
		cnt_1 = cnt_2 = 0;
		myLock = boost::shared_ptr<Lock>(new Lock);//will be deleted itself :)
	}



	void setEntity(sim_mob::Entity*);
	sim_mob::Entity* getEntity();
	std::vector<DATA_MSG_PTR>& getIncoming();
	std::vector<DATA_MSG_PTR>& getOutgoing();
	void setIncoming(std::vector<DATA_MSG_PTR> value);
	void setOutgoing(std::vector<DATA_MSG_PTR> value);
	void addIncoming(DATA_MSG_PTR value);
	void addOutgoing(DATA_MSG_PTR value);

	void setwriteIncomingDone(bool value);
	void setWriteOutgoingDone(bool value);
	void setAgentUpdateDone(bool value);
	bool iswriteIncomingDone();
	bool isreadOutgoingDone();
	bool isAgentUpdateDone();
	bool isOutgoingDirty();
	bool isIncomingDirty();
	void reset();

	subscriptionInfo & operator=(const subscriptionInfo&) { return *this;}
};


/*********************************************************************************
 *
 * ************************ Common Arguments and return Values *******************
 *
 *********************************************************************************/
class commArguments
{
private:
	std::string fileName;
	std::set<DATA_MSG_PTR> data;
public:
	commArguments(std::string &fileName_,std::set<DATA_MSG_PTR> &data_):fileName(fileName_), data(data_){}
	commArguments(){}

	std::set<DATA_MSG_PTR> &getData() {
		return data;
	}

	void setData(std::set<DATA_MSG_PTR> & value) {
		this->data = value;
	}

	std::string getFileName() const {
		return fileName;
	}

	void setFileName(std::string fileName) {
		this->fileName = fileName;
	}
};

class commResult
{
public:
	enum result
	{
		success,
		failure,
		unknown
	};

	commResult(result result_ = unknown):res(result_){};
	result getResult() const{ return res;}

private:
	result res;

};


}//namespace sim_mob
