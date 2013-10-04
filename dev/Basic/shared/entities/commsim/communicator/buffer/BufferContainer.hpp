//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <boost/thread/locks.hpp>
#include "boost/thread/shared_mutex.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <jsoncpp/json/json.h>
#include <boost/thread.hpp>
//todo change this class to a template class. it is possible-vahid
namespace sim_mob
{
template<class T>
class BufferContainer
{
public:

//	boost::shared_ptr<boost::shared_mutex>  Owner_Mutex;//this buffer may belong to someone(like Broker) who might need its own lock when others are operating on this data
	//todo make it private
	boost::shared_mutex Owner_Mutex;
	std::vector<T> buffer;
	bool work_in_progress;

BufferContainer(){
work_in_progress = false;
}


//BufferContainer(boost::shared_ptr<boost::shared_mutex> Owner_Mutex_){
//	Owner_Mutex = Owner_Mutex_;
//work_in_progress = false;
//}



BufferContainer( const BufferContainer& other ) :
    buffer( other.get() )
 {
 }


BufferContainer& operator=(BufferContainer& other)
{
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	buffer = other.get();
	return *this;
}


void add(T value) {

	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	buffer.push_back(value);
}


void add(std::vector<T> values) {
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	buffer.insert(buffer.end(), values.begin(), values.end());
}


void add(BufferContainer & value) {
//	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	add(value.get());
}


void reset() {
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	T value;
	buffer.clear();
	work_in_progress = false;
}


void clear()
{
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	buffer.clear();
}


void set_work_in_progress(bool value)
{
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	work_in_progress = value;
}


bool get_work_in_progress()
{
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	return work_in_progress;
}

const std::vector<T>& get() const{
//	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	return buffer;
}

 std::vector<T>& get() {
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	return buffer;
}

bool pop(T & var) {
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	if (buffer.size() < 1)
		return false;
	var = buffer.front();
	buffer.erase(buffer.begin());
	return true;
}


bool empty(){
	boost::unique_lock< boost::shared_mutex > lock(Owner_Mutex);
	return buffer.empty();
}

//Implementation////////////////////////////////

 void setOwnerMutex(boost::shared_ptr<boost::shared_mutex> value)
{
	Owner_Mutex = value;
}

};//end of class
}//namespace sim_mob
