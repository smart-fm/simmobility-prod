#pragma once
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/set.hpp>
#include <boost/thread/locks.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
//#include "buffering/Shared.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>


namespace sim_mob
{
class Entity;
//THIS ENUM IDENTIFIES THE CLASSES TYPES
//typedef boost::shared_mutex boost::shared_mutex;
//typedef boost::unique_lock< boost::shared_mutex > writeboost::shared_mutex;
//typedef boost::shared_lock< boost::shared_mutex > Readboost::shared_mutex;

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
        ar & serial;
    }
public:


//	DataClassType type;
	std::string str;
	int serial;
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
		sender = receiver = serial = 0;
		str = "";
	}
	//uncomment after debugging
	void setDataClassType();
	virtual void registerType(boost::archive::text_oarchive &oa)
    {
		std::cout << "dataMessage::serialize=>outgoing registered" << std::endl;
    	oa.register_type(static_cast<dataMessage *>(NULL));
    }

	virtual void registerType(boost::archive::text_iarchive &ia)
    {
		std::cout << "dataMessage::serialize=>incoming registered" << std::endl;
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

//	YOUR_DATA your_data;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<DATA_MSG>(*this);//in this case the direct parent of sample_DataMessage_Class is DATA_MSG.
    	//serialize your data here
//        ar & your_data;
    }

public:
    //copy paste the following 2 methods in YOUR data class and replace 'sample_DataMessage_Class' with the name of your class
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

  container classes acting as incoming , outgoing and(possibly) temporary buffers
  these so called buffers will have diverse data whose types are already defined by
  you (above)

 *********************************************************************************/

class DataContainer
{

public:
	boost::shared_mutex DataContainer_Mutex;
	boost::shared_mutex * Owner_Mutex;//this buffer may belong to someone who might need its own lock when others are operating on this data
	//todo make it private
	std::vector<DATA_MSG_PTR> buffer;
	bool work_in_progress;
	DataContainer();
	DataContainer( const DataContainer& other );
	DataContainer& operator=(DataContainer& other);
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
    	ar.register_type(static_cast<DATA_MSG_PTR>(NULL));
    	ar & buffer;
    }
    void add(DATA_MSG_PTR value);
    void add(std::vector<DATA_MSG_PTR> values);

    void add(DataContainer & value);
    void reset();
    void clear();//clear the buffer but do not delete the referenced elements buffer was pointing to
    void set_work_in_progress(bool);
    bool get_work_in_progress();
    std::vector<DATA_MSG_PTR>& get();
    bool pop(DATA_MSG_PTR & var);
    bool empty();

    ////////////////////////////////
    void setOwnerMutex(boost::shared_mutex*);

};


/*********************************************************************************
 *
 * The following is just a small helper structure for sending subscription information from
 * common agents to communicator.
 * It contains references to members of CommunicationSupport (which is a sub-class
 * of common agents who are willing to have communication ability)
 *
 *********************************************************************************/


//class subscriptionInfo
//{
//
//	 sim_mob::Entity * agent;
//
//	//good for avoiding unnecessary writes
//	bool& agentUpdateDone;      //All the operations inside the update method of the 'communicating' agent were done
//	//the following flags allow access to the incoming and outgoing buffers by bothe owner(communicating agent) and communicator agent without imposing any lock on the buffers
//	bool& incomingIsDirty;     //there is something in the incoming buffer (buffer is written by 'communicator' agent; to be read by the 'communicating' agent)
//	bool& outgoingIsDirty;		//there is something in the outgoing buffer (buffer is written by 'communicating' agent; to be read by the 'communicator' agent)
//	//------------------------------------
//	bool& writeIncomingDone;     //the 'communicator' agent updated/write_to the incoming buffer of the 'communicating' agent
//	bool& readOutgoingDone;      //the 'communicator' agent  read the outgoing buffer of the 'communicating' agent
//
//
//	DataContainer &incoming;
//	DataContainer &outgoing;
//	bool goodForProcessing;
//
//public:
//	boost::shared_ptr<boost::shared_mutex> myboost::shared_mutex;//this class is not copy constructible. so we need to use sort of a pointer(i use boost shared pointer)
//	//general purpose counter
//	int cnt_1;//i use this one to control/limit the number of times communicator faces the 'update not done'
//	int cnt_2;
//
//	subscriptionInfo(
//			sim_mob::Entity *  agent_ ,
//			bool& incomingIsDirty_ ,
//			bool& outgoingIsDirty_ ,
//			bool& writeIncomingDone_,
//			bool& readOutgoingDone_ ,
//			bool& agentUpdateDone_,
//			DataContainer& incoming_,
//			DataContainer& outgoing_
//
//			);
//
//
//	void setEntity(sim_mob::Entity*);
//	sim_mob::Entity* getEntity();
//	DataContainer& getIncoming();
//	DataContainer& getOutgoing();
//	void setIncoming(DataContainer value);
//	void setOutgoing(DataContainer value);
//	void addIncoming(DATA_MSG_PTR value);
//	void addOutgoing(DATA_MSG_PTR value);
//
//	void setwriteIncomingDone(bool value);
//	void setWriteOutgoingDone(bool value);
//	void setAgentUpdateDone(bool value);
//	bool iswriteIncomingDone();
//	bool isreadOutgoingDone();
//	bool isAgentUpdateDone();
//	bool isOutgoingDirty();
//	bool isIncomingDirty();
//	void reset();
//
//	subscriptionInfo & operator=(const subscriptionInfo&) { return *this;}
//};


/*********************************************************************************
 *
 * ************************ Common Arguments and return Values *******************
 *
 *********************************************************************************/
class commArguments
{
private:
	std::string fileName;
	DataContainer data;
public:
	commArguments(std::string &fileName_,DataContainer &data_):fileName(fileName_), data(data_){}
	commArguments(){}

	DataContainer &getData() {
		return data;
	}

	void setData(DataContainer & value) {
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
