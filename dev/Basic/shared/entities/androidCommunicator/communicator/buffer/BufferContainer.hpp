#pragma once

#include <boost/thread/locks.hpp>
#include "boost/thread/shared_mutex.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <jsoncpp/json.h>
#include <boost/thread.hpp>

namespace sim_mob
{
typedef boost::tuple<unsigned int , unsigned int, std::string> DataElement; //<sending agent, receiving agent, data>
//	typedef boost::tuple<int,std::string,Json::Value> DataElement;//<msg type, data, original json>  : I dont know which one will be more useful so I include them all along
class BufferContainer
{

public:
	boost::shared_ptr<boost::shared_mutex>  Owner_Mutex;//this buffer may belong to someone(like Broker) who might need its own lock when others are operating on this data
	//todo make it private
	std::vector<DataElement> buffer;
	bool work_in_progress;

	BufferContainer(boost::shared_ptr<boost::shared_mutex>  Owner_Mutex_);
	BufferContainer();
	BufferContainer( const BufferContainer& other );
	BufferContainer& operator=(BufferContainer& other);

    void add(BufferContainer & value);
    void add(std::vector<DataElement> values);
    void add(DataElement  value);

    void reset();
    void clear();//clear the buffer but do not delete the referenced elements buffer was pointing to

    void set_work_in_progress(bool);
    bool get_work_in_progress();
    bool empty();

    std::vector<DataElement>& get();
    bool pop(DataElement & var);

    ////////////////////////////////
    void setOwnerMutex(boost::shared_ptr<boost::shared_mutex>);

    static DataElement makeDataElement(std::string &str , unsigned int sender = 0, unsigned int receiver = 0)
    {
    	return boost::make_tuple(sender, receiver, str);
    }
};


}//namespace sim_mob
